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
