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

#pragma once
#include "Memory.hpp"
#include <initializer_list>
#include <type_traits>
#include <utility>

#define VA_EXPAND(...) __VA_ARGS__

namespace memory {

template <typename T>
struct IsConstMemberFun : std::false_type {};

template <typename T, typename Ret, typename... Args>
struct IsConstMemberFun<Ret (T::*)(Args...) const> : std::true_type {};

template <typename T>
inline constexpr bool IsConstMemberFunV = IsConstMemberFun<T>::value;

template <typename T>
struct AddConstAtMemberFun {
    using type = T;
};

template <typename T, typename Ret, typename... Args>
struct AddConstAtMemberFun<Ret (T::*)(Args...)> {
    using type = Ret (T::*)(Args...) const;
};

template <typename T>
using AddConstAtMemberFunT = typename AddConstAtMemberFun<T>::type;

template <typename T, typename U>
using AddConstAtMemberFunIfOriginIs = std::conditional_t<IsConstMemberFunV<U>, AddConstAtMemberFunT<T>, T>;

int hook(FuncPtr target, FuncPtr detour, FuncPtr* originalFunc, bool suspendThreads = true);

bool unhook(FuncPtr target, FuncPtr detour, bool suspendThreads = true);

FuncPtr resolveIdentifier(std::initializer_list<const char*> identifiers);


template <typename T>
concept FuncPtrType = std::is_function_v<std::remove_pointer_t<T>> || std::is_member_function_pointer_v<T>;

template <typename T>
    requires(FuncPtrType<T> || std::is_same_v<T, uintptr_t>)
constexpr FuncPtr resolveIdentifier(T identifier) {
    return toFuncPtr(identifier);
}

// redirect to resolveIdentifier(FuncPtr)
template <typename T>
constexpr FuncPtr resolveIdentifier(FuncPtr address) {
    return address;
}

} // namespace memory

#define MEMORY_CONCAT_IMPL(a, b)      a##b
#define MEMORY_CONCAT(a, b)           MEMORY_CONCAT_IMPL(a, b)
#define MEMORY_HOOK_TYPE_NAME(id)     MEMORY_CONCAT(AutoHook, id)
#define MEMORY_HOOK_REGISTER_NAME(id) MEMORY_CONCAT(MEMORY_HOOK_TYPE_NAME(id), AutoRegister)
#define HOOK_SIGS(...)                ::memory::resolveIdentifier({__VA_ARGS__})

#define HOOK_IMPL(REGISTER, FUNC_PTR, STATIC, CALL, DEF_TYPE, TYPE, IDENTIFIER, RET_TYPE, ...)                                                       \
    struct DEF_TYPE TYPE {                                                                                                                           \
        using FuncPtr        = ::memory::FuncPtr;                                                                                                    \
        using OriginFuncType = ::memory::AddConstAtMemberFunIfOriginIs<RET_TYPE FUNC_PTR(__VA_ARGS__), decltype(IDENTIFIER)>;                        \
                                                                                                                                                     \
        inline static FuncPtr        target{};                                                                                                       \
        inline static OriginFuncType originFunc{};                                                                                                   \
                                                                                                                                                     \
        template <typename... Args>                                                                                                                  \
        STATIC RET_TYPE origin(Args&&... params) {                                                                                                   \
            return CALL(std::forward<Args>(params)...);                                                                                              \
        }                                                                                                                                            \
                                                                                                                                                     \
        STATIC RET_TYPE detour(__VA_ARGS__);                                                                                                         \
                                                                                                                                                     \
        static int hook() {                                                                                                                          \
            target = memory::resolveIdentifier<OriginFuncType>(IDENTIFIER);                                                                          \
            if (target == nullptr) {                                                                                                                 \
                return -1;                                                                                                                           \
            }                                                                                                                                        \
            return memory::hook(target, memory::toFuncPtr(&DEF_TYPE::detour), reinterpret_cast<FuncPtr*>(&originFunc));                              \
        }                                                                                                                                            \
                                                                                                                                                     \
        static bool unhook() { return memory::unhook(target, memory::toFuncPtr(&DEF_TYPE::detour)); }                                                \
    };                                                                                                                                               \
    REGISTER;                                                                                                                                        \
    RET_TYPE DEF_TYPE::detour(__VA_ARGS__)

#define AUTO_INSTANCE_HOOK_IMPL(ID, ...)                                                                                                             \
    VA_EXPAND(HOOK_IMPL(                                                                                                                             \
        inline struct MEMORY_HOOK_REGISTER_NAME(ID) {                                                                                                \
            MEMORY_HOOK_REGISTER_NAME(ID)() { MEMORY_HOOK_TYPE_NAME(ID)::hook(); }                                                                   \
            ~MEMORY_HOOK_REGISTER_NAME(ID)() { MEMORY_HOOK_TYPE_NAME(ID)::unhook(); }                                                                \
        } MEMORY_HOOK_REGISTER_NAME(ID),                                                                                                             \
        (MEMORY_HOOK_TYPE_NAME(ID)::*),                                                                                                              \
        ,                                                                                                                                            \
        (this->*originFunc),                                                                                                                         \
        MEMORY_HOOK_TYPE_NAME(ID),                                                                                                                   \
        ,                                                                                                                                            \
        __VA_ARGS__                                                                                                                                  \
    ))

#define AUTO_INSTANCE_HOOK(IDENTIFIER, RET_TYPE, ...) VA_EXPAND(AUTO_INSTANCE_HOOK_IMPL(__COUNTER__, IDENTIFIER, RET_TYPE, __VA_ARGS__))