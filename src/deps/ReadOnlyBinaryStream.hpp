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
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

class ReadOnlyBinaryStream {
public:
    bool             mHasOverflowed;
    std::string_view mBufferView;
    size_t           mReadPointer;

private:
    template <typename T>
    constexpr bool read(T* target) noexcept {
        if (mHasOverflowed) {
            return false;
        }
        size_t newReadPointer = mReadPointer + sizeof(T);

        if (newReadPointer < mReadPointer || newReadPointer > mBufferView.length()) {
            mHasOverflowed = true;
            return false;
        }

        std::copy(mBufferView.begin() + mReadPointer, mBufferView.begin() + newReadPointer, reinterpret_cast<char*>(target));
        mReadPointer = newReadPointer;
        return true;
    }

public:
    [[nodiscard]] constexpr explicit ReadOnlyBinaryStream(std::string_view buffer) noexcept
    : mBufferView(buffer),
      mReadPointer(0),
      mHasOverflowed(false) {}

    [[nodiscard]] constexpr size_t size() const noexcept { return mBufferView.size(); }

    [[nodiscard]] constexpr size_t getPosition() const noexcept { return mReadPointer; }

    constexpr void setPosition(size_t value) noexcept { mReadPointer = value; }

    constexpr void resetPosition() noexcept { setPosition(0); }

    constexpr void ignoreBytes(size_t length) noexcept { mReadPointer += length; }

    [[nodiscard]] constexpr std::string getLeftBuffer() const noexcept { return std::string(mBufferView.substr(mReadPointer)); }

    [[nodiscard]] constexpr std::string_view getLeftBufferView() const noexcept { return mBufferView.substr(mReadPointer); }

    [[nodiscard]] constexpr bool isOverflowed() const noexcept { return mHasOverflowed; }

    [[nodiscard]] constexpr bool hasDataLeft() const noexcept { return mReadPointer < mBufferView.size(); }

    [[nodiscard]] constexpr std::string_view view() const noexcept { return mBufferView; }

    [[nodiscard]] constexpr bool operator==(ReadOnlyBinaryStream const& other) const noexcept { return mBufferView == other.mBufferView; }

    constexpr bool getBytes(void* target, size_t num) noexcept {
        if (mHasOverflowed) {
            return false;
        }
        if (num == 0) {
            return true;
        }

        size_t newPointer = mReadPointer + num;

        if (newPointer < mReadPointer || newPointer > mBufferView.size()) {
            mHasOverflowed = true;
            return false;
        }

        std::copy(mBufferView.begin() + mReadPointer, mBufferView.begin() + newPointer, static_cast<char*>(target));
        mReadPointer = newPointer;
        return true;
    }

    [[nodiscard]] constexpr uint8_t getUnsignedChar() noexcept {
        uint8_t value = 0;
        read(&value);
        return value;
    }

    [[nodiscard]] constexpr uint8_t getByte() noexcept { return getUnsignedChar(); }

    [[nodiscard]] constexpr uint16_t getUnsignedShort() noexcept {
        uint16_t value = 0;
        read(&value);
        return value;
    }

    [[nodiscard]] constexpr uint32_t getUnsignedInt() noexcept {
        uint32_t value = 0;
        read(&value);
        return value;
    }

    [[nodiscard]] constexpr uint64_t getUnsignedInt64() noexcept {
        uint64_t value = 0;
        read(&value);
        return value;
    }

    [[nodiscard]] constexpr bool getBool() noexcept { return getUnsignedChar() != 0; }

    [[nodiscard]] constexpr double getDouble() noexcept {
        double value = 0;
        read(&value);
        return value;
    }

    [[nodiscard]] constexpr float getFloat() noexcept {
        float value = 0;
        read(&value);
        return value;
    }

    [[nodiscard]] constexpr int32_t getSignedInt() noexcept {
        int32_t value = 0;
        read(&value);
        return value;
    }

    [[nodiscard]] constexpr int64_t getSignedInt64() noexcept {
        int64_t value = 0;
        read(&value);
        return value;
    }

    [[nodiscard]] constexpr int16_t getSignedShort() noexcept {
        int16_t value = 0;
        read(&value);
        return value;
    }

    [[nodiscard]] constexpr uint32_t getUnsignedVarInt() noexcept {
        uint32_t value = 0;
        unsigned shift = 0;
        uint8_t  byte;

        do {
            if (shift >= 35) {
                mHasOverflowed = true;
                return value;
            }

            if (mReadPointer >= mBufferView.size()) {
                mHasOverflowed = true;
                return value;
            }

            byte   = static_cast<uint8_t>(mBufferView[mReadPointer++]);
            value |= (byte & 0x7F) << shift;
            shift += 7;

        } while (byte & 0x80);

        return value;
    }

    [[nodiscard]] constexpr uint64_t getUnsignedVarInt64() noexcept {
        uint64_t value = 0;
        unsigned shift = 0;
        uint8_t  byte;

        do {
            if (shift >= 70) {
                mHasOverflowed = true;
                return value;
            }

            if (mReadPointer >= mBufferView.size()) {
                mHasOverflowed = true;
                return value;
            }

            byte   = static_cast<uint8_t>(mBufferView[mReadPointer++]);
            value |= static_cast<uint64_t>(byte & 0x7F) << shift;
            shift += 7;

        } while (byte & 0x80);

        return value;
    }

    [[nodiscard]] constexpr int32_t getVarInt() noexcept {
        uint32_t value = getUnsignedVarInt();
        return (value & 1) ? ~(value >> 1) : (value >> 1);
    }

    [[nodiscard]] constexpr int64_t getVarInt64() noexcept {
        uint64_t value = getUnsignedVarInt64();
        return (value & 1) ? ~(value >> 1) : (value >> 1);
    }

    constexpr void getString(std::string& outString) noexcept {
        uint32_t length = getUnsignedVarInt();
        if (mHasOverflowed || length == 0) {
            outString.clear();
            return;
        }

        if (mReadPointer + length > mBufferView.size()) {
            mHasOverflowed = true;
            outString.clear();
            return;
        }

        outString.assign(mBufferView.substr(mReadPointer, length));
        mReadPointer += length;
    }

    constexpr void readString(std::string& outString) noexcept {
        uint32_t length = getUnsignedVarInt();
        if (mHasOverflowed || length == 0) {
            outString.clear();
            return;
        }

        if (mReadPointer + length > mBufferView.size()) {
            mHasOverflowed = true;
            outString.clear();
            return;
        }

        outString.assign(mBufferView.substr(mReadPointer, length));
        mReadPointer += length;
    }

    [[nodiscard]] constexpr std::string_view getStringView() noexcept {
        uint32_t length = getUnsignedVarInt();
        if (mHasOverflowed || length == 0) {
            return {};
        }

        if (mReadPointer + length > mBufferView.size()) {
            mHasOverflowed = true;
            return {};
        }

        auto result   = mBufferView.substr(mReadPointer, length);
        mReadPointer += length;
        return result;
    }

    [[nodiscard]] constexpr std::string getString() noexcept {
        std::string result;
        getString(result);
        return result;
    }

    [[nodiscard]] constexpr uint32_t getUnsignedInt24() noexcept {
        if (mReadPointer + 3 > mBufferView.size()) {
            mHasOverflowed = true;
            return 0;
        }

        uint32_t value  = static_cast<uint8_t>(mBufferView[mReadPointer++]);
        value          |= static_cast<uint32_t>(static_cast<uint8_t>(mBufferView[mReadPointer++])) << 8;
        value          |= static_cast<uint32_t>(static_cast<uint8_t>(mBufferView[mReadPointer++])) << 16;
        return value;
    }

    constexpr void getRawBytes(std::string& rawBuffer, size_t length) noexcept {
        if (length == 0) {
            rawBuffer.clear();
            return;
        }

        if (mReadPointer + length > mBufferView.size()) {
            mHasOverflowed = true;
            rawBuffer.clear();
            return;
        }

        rawBuffer.assign(mBufferView.substr(mReadPointer, length));
        mReadPointer += length;
    }

    [[nodiscard]] constexpr std::string getRawBytes(size_t length) noexcept {
        std::string result;
        getRawBytes(result, length);
        return result;
    }
};