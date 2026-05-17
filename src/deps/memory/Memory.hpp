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

#include <Windows.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace memory {

using FuncPtr = void*;


template <typename T>
    requires(sizeof(T) == sizeof(FuncPtr))
constexpr FuncPtr toFuncPtr(T t) {
    union {
        FuncPtr fp;
        T       t;
    } u{};
    u.t = t;
    return u.fp;
}

template <typename T>
    requires(std::is_member_function_pointer_v<T> && sizeof(T) == sizeof(FuncPtr) + sizeof(ptrdiff_t))
constexpr FuncPtr toFuncPtr(T t) {
    union {
        struct {
            FuncPtr   fp;
            ptrdiff_t offset;
        };
        T t;
    } u{};
    u.t = t;
    return u.fp;
}

template <typename T>
inline void memcpy_t(void* dst, const void* src, size_t count) {
    memcpy(dst, src, count * sizeof(T));
}

template <typename T>
inline void memcpy_t(void* dst, const void* src) {
    memcpy(dst, src, sizeof(T));
}

FuncPtr resolveSignature(char const* signature);
bool    IsReadableMemory(void* ptr, size_t size);

} // namespace memory
