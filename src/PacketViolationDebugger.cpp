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

#include "MinecraftPacketIds.hpp"
#include "PacketViolationSeverity.hpp"
#include "PacketViolationType.hpp"
#include "Version.hpp"
#include "deps/ReadOnlyBinaryStream.hpp"
#include "deps/memory/Hook.hpp"
#include <chrono>
#include <magic_enum/magic_enum.hpp>
#include <print>

#define CURRENT_TIME                                                                                                                                 \
    std::chrono::zoned_time { std::chrono::current_zone(), std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now()) }

AUTO_INSTANCE_HOOK(
    HOOK_SIGS(
        // 1.21.50
        "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC 70 41 8B F1 48 8B FA 48 8B D9 48 83 C1 18 48 83 7A ?? ?? 76 ?? 48 8B 12",
        // 1.21.60 - 1.26.0
        "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 81 EC 80 00 00 00 41 8B F1 48 8B FA 48 8B D9 48 83 C1 18 48 8B 01 4C 8B 90 ?? ?? ?? ?? 48 8B C2 48 83 "
        "7A ?? ?? 76 ?? 48 8B 02",
        // 1.26.10
        "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 81 EC 90 00 00 00 0F 29 B4 24 ?? ?? ?? ?? 41 8B F1",
        // 1.26.20
        "55 56 57 53 48 83 EC 78 48 8D 6C 24 70 48 C7 45 00 FE FF FF FF 44 89 CB"
        // 1.26.30
        "55 41 57 41 56 41 55 41 54 56 57 53 48 83 EC 78 48 8D 6C 24 70 48 C7 45 00 FE FF FF FF 44 89 CB"
    ),
    void,
    std::string& buffer,
    int          reliability,
    int          compressible
) {
    auto stream   = ReadOnlyBinaryStream(buffer);
    auto header   = stream.getUnsignedVarInt();
    auto packetId = static_cast<MinecraftPacketIds>(header & 0x3FF);
    if (packetId == MinecraftPacketIds::PacketViolationWarning) {
        auto type     = static_cast<PacketViolationType>(stream.getVarInt());
        auto severity = static_cast<PacketViolationSeverity>(stream.getVarInt());
        auto errorId  = static_cast<MinecraftPacketIds>(stream.getVarInt());
        auto context  = stream.getString();
        std::println("\x1b[91m{:%T} ERROR [PacketViolationDebugger] Packet violation detected!", CURRENT_TIME);
        std::println("===============================================================================================================");
        std::println("    Violation Packet ID: {} ({}Packet)", static_cast<int>(errorId), magic_enum::enum_name(errorId));
        std::println("    Violation Reason: {}", context);
        std::println("    Violation Type: {}", magic_enum::enum_name(type));
        std::println("    Violation Severity: {}", magic_enum::enum_name(severity));
        std::println("===============================================================================================================\x1b[0m");
    }
    return origin(buffer, reliability, compressible);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        std::println(
            "\033[38;2;173;216;230m{:%T} \033[38;2;32;178;170mINFO \x1b[0m[PacketViolationDebugger] PacketViolationDebugger v{} loaded!",
            CURRENT_TIME,
            FILE_VERSION_STRING
        );
        std::println(
            "\033[38;2;173;216;230m{:%T} \033[38;2;32;178;170mINFO \x1b[0m[PacketViolationDebugger] "
            "https://github.com/SculkCatalystMC/PacketViolationDebugger",
            CURRENT_TIME
        );
        std::println(
            "\033[38;2;173;216;230m{:%T} \033[38;2;32;178;170mINFO \x1b[0m[PacketViolationDebugger] Copyright © 2026 SculkCatalystMC. All rights "
            "reserved.",
            CURRENT_TIME
        );
        DisableThreadLibraryCalls(hModule);
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
