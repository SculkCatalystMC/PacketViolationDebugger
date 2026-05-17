// Copyright © 2026 SculkCatalystMC. All rights reserved.
//
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License or any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Hook.hpp"

#include <Windows.h>
#include <detours/detours.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <print>
#include <psapi.h>
#include <set>
#include <string>
#include <tlhelp32.h>
#include <unordered_map>
#include <vector>

namespace thread {

class GlobalThreadPauser {
    std::vector<unsigned int> pausedIds;

public:
    GlobalThreadPauser();
    ~GlobalThreadPauser();
};

GlobalThreadPauser::GlobalThreadPauser() {
    HANDLE      h         = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    static auto processId = GetCurrentProcessId();
    auto        threadId  = GetCurrentThreadId();

    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    if (Thread32First(h, &te)) {
        do {
            if (te.dwSize >= offsetof(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID)) {
                if (te.th32OwnerProcessID == processId && te.th32ThreadID != threadId) {
                    HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, false, te.th32ThreadID);
                    if (thread != nullptr) {
                        if ((int)SuspendThread(thread) != -1) {
                            pausedIds.emplace_back(te.th32ThreadID);
                        }
                        CloseHandle(thread);
                    }
                }
            }
            te.dwSize = sizeof(te);
        } while (Thread32Next(h, &te));
    }
    CloseHandle(h);
}

GlobalThreadPauser::~GlobalThreadPauser() {
    for (auto id : pausedIds) {
        HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, false, id);
        if (thread != nullptr) {
            ResumeThread(thread);
            CloseHandle(thread);
        }
    }
}

} // namespace thread

namespace memory {

FuncPtr resolveIdentifier(std::initializer_list<const char*> identifiers) {
    for (const auto& identifier : identifiers) {
        FuncPtr result = resolveSignature(identifier);
        if (result != nullptr) {
            return result;
        }
    }
    std::println("\x1b[91m[Error] Failed to find signature!\x1b[0m");
    throw std::exception("bad signature resolve");
}

struct HookElement {
    FuncPtr  detour{};
    FuncPtr* originalFunc{};
    int      id{};

    bool operator<(const HookElement& other) const { return id < other.id; }
};

struct HookData {
    FuncPtr               target{};
    FuncPtr               origin{};
    FuncPtr               start{};
    FuncPtr               thunk{};
    int                   hookId{};
    std::set<HookElement> hooks{};

    inline ~HookData() {
        if (this->thunk != nullptr) {
            VirtualFree(this->thunk, 0, MEM_RELEASE);
            this->thunk = nullptr;
        }
    }

    inline void updateCallList() {
        FuncPtr* last = nullptr;
        for (auto& item : this->hooks) {
            if (last == nullptr) {
                this->start = item.detour;
                last        = item.originalFunc;
            } else {
                *last = item.detour;
                last  = item.originalFunc;
            }
        }
        if (last == nullptr) this->start = this->origin;
        else *last = this->origin;
    }

    inline int incrementHookId() { return ++hookId; }
};

std::unordered_map<FuncPtr, std::shared_ptr<HookData>>& getHooks() {
    static std::unordered_map<FuncPtr, std::shared_ptr<HookData>> hooks;
    return hooks;
}

static std::mutex hooksMutex{};

FuncPtr createThunk(FuncPtr* target) {
    constexpr auto THUNK_SIZE            = 18;
    unsigned char  thunkData[THUNK_SIZE] = {0};
    // generate a thunk:
    // mov rax hooker1
    thunkData[0] = 0x48;
    thunkData[1] = 0xB8;
    memcpy(thunkData + 2, &target, sizeof(FuncPtr*));
    // mov rax [rax]
    thunkData[10] = 0x48;
    thunkData[11] = 0x8B;
    thunkData[12] = 0x00;
    // jmp rax
    thunkData[13] = 0xFF;
    thunkData[14] = 0xE0;

    auto thunk = VirtualAlloc(nullptr, THUNK_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    memcpy(thunk, thunkData, THUNK_SIZE);
    DWORD dummy;
    VirtualProtect(thunk, THUNK_SIZE, PAGE_EXECUTE_READ, &dummy);
    return thunk;
}

int processHook(FuncPtr target, FuncPtr detour, FuncPtr* originalFunc) {
    FuncPtr tmp = target;
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    int rv = DetourAttach(&tmp, detour);
    DetourTransactionCommit();
    *originalFunc = tmp;
    return rv;
}

int hook(FuncPtr target, FuncPtr detour, FuncPtr* originalFunc, bool suspendThreads) {
    std::lock_guard lock(hooksMutex);

    std::unique_ptr<thread::GlobalThreadPauser> pauser;
    if (suspendThreads) {
        pauser = std::make_unique<thread::GlobalThreadPauser>();
    }
    auto it = getHooks().find(target);
    if (it != getHooks().end()) {
        auto hookData = it->second;
        hookData->hooks.insert({detour, originalFunc, hookData->incrementHookId()});
        hookData->updateCallList();
        return ERROR_SUCCESS;
    }
    auto hookData   = new HookData{target, target, detour, nullptr, {}, {}};
    hookData->thunk = createThunk(&hookData->start);
    hookData->hooks.insert({detour, originalFunc, hookData->incrementHookId()});
    auto ret = processHook(target, hookData->thunk, &hookData->origin);
    if (ret) {
        delete hookData;
        return ret;
    }
    hookData->updateCallList();
    getHooks().emplace(target, std::shared_ptr<HookData>(hookData));
    return ERROR_SUCCESS;
}

bool unhook(FuncPtr target, FuncPtr detour, bool suspendThreads) {
    std::lock_guard lock(hooksMutex);

    std::unique_ptr<thread::GlobalThreadPauser> pauser;
    if (suspendThreads) {
        pauser = std::make_unique<thread::GlobalThreadPauser>();
    }

    if (target == nullptr) {
        return false;
    }
    auto hookDataIter = getHooks().find(target);
    if (hookDataIter == getHooks().end()) {
        return false;
    }
    auto& hookData = hookDataIter->second;
    for (auto it = hookData->hooks.begin(); it != hookData->hooks.end(); ++it) {
        if (it->detour == detour) {
            hookData->hooks.erase(it);
            hookData->updateCallList();
            break;
        }
    }

    if (hookData->hooks.empty()) {
        FuncPtr tmp = hookData->start;
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&tmp, hookData->thunk);
        DetourTransactionCommit();

        getHooks().erase(target);
    }

    return true;
}

FuncPtr resolveSignature(char const* signature) {
    static std::unordered_map<std::string, uintptr_t> signatureCache;
    if (signatureCache.find(signature) != signatureCache.end()) {
        return reinterpret_cast<FuncPtr>(signatureCache[signature]);
    }

    auto ParseSignature = [](const char* sig, std::vector<unsigned char>& pattern, std::vector<unsigned char>& mask) {
        while (*sig) {
            if (*sig == ' ' || *sig == '\t') {
                ++sig;
                continue;
            }
            if (*sig == '?') {
                pattern.push_back(0);
                mask.push_back(0);
                if (*(sig + 1) == '?') sig += 2;
                else ++sig;
            } else {
                char         byteStr[3] = {sig[0], sig[1], 0};
                unsigned int byteVal    = strtoul(byteStr, nullptr, 16);
                pattern.push_back(static_cast<unsigned char>(byteVal));
                mask.push_back(0xFF);
                sig += 2;
            }
        }
    };

    struct MemoryRegion {
        uintptr_t base;
        size_t    size;
    };

    auto GetModuleMemoryRegions = [](uintptr_t rangeStart, uintptr_t rangeEnd) {
        std::vector<MemoryRegion> regions;
        uintptr_t                 addr = rangeStart;
        while (addr < rangeEnd) {
            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) == 0) {
                addr += 0x1000;
                continue;
            }
            if (mbi.State == MEM_COMMIT && !(mbi.Protect & PAGE_NOACCESS) && !(mbi.Protect & PAGE_GUARD)) {
                MemoryRegion r{};
                r.base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
                r.size = mbi.RegionSize;
                regions.push_back(r);
            }
            addr += mbi.RegionSize;
        }
        return regions;
    };

    std::vector<unsigned char> pattern;
    std::vector<unsigned char> mask;
    ParseSignature(signature, pattern, mask);
    const size_t patLen = pattern.size();
    if (patLen == 0) return 0;

    bool pure = true;
    for (unsigned char m : mask) {
        if (m != 0xFF) {
            pure = false;
            break;
        }
    }

    static const auto rangeStart = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
    static MODULEINFO miModInfo  = {0};
    static bool       init       = false;
    if (!init) {
        init = true;
        GetModuleInformation(GetCurrentProcess(), reinterpret_cast<HMODULE>(rangeStart), &miModInfo, sizeof(miModInfo));
    }
    const uintptr_t rangeEnd = rangeStart + miModInfo.SizeOfImage;

    std::vector<MemoryRegion> regions = GetModuleMemoryRegions(rangeStart, rangeEnd);
    if (regions.empty()) return 0;

    for (const auto& region : regions) {
        uintptr_t regionEnd = region.base + region.size;
        for (uintptr_t addr = region.base; addr <= regionEnd - patLen; ++addr) {
            if (pure) {
                if (memcmp(reinterpret_cast<const void*>(addr), pattern.data(), patLen) == 0) return reinterpret_cast<FuncPtr>(addr);
            } else {
                bool match = true;
                for (size_t k = 0; k < patLen; k++) {
                    unsigned char byteVal = *(reinterpret_cast<unsigned char*>(addr + k));
                    if (mask[k] != 0 && byteVal != pattern[k]) {
                        match = false;
                        break;
                    }
                }
                if (match) return reinterpret_cast<FuncPtr>(addr);
            }
        }
    }

    return nullptr;
}

bool IsReadableMemory(void* ptr, size_t /*size*/) {
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(ptr, &mbi, sizeof(mbi))) {
        return (mbi.State == MEM_COMMIT) && (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_READ));
    }
    return false;
}

} // namespace memory
