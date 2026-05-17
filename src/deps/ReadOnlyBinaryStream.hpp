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
#include <cstdint>
#include <string>

class ReadOnlyBinaryStream {
public:
    bool             mHasOverflowed;
    std::string_view mBufferView;
    size_t           mReadPointer;

    [[nodiscard]] constexpr explicit ReadOnlyBinaryStream(std::string_view buffer) noexcept
    : mBufferView(buffer),
      mReadPointer(0),
      mHasOverflowed(false) {}

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

    [[nodiscard]] constexpr int32_t getVarInt() noexcept {
        uint32_t value = getUnsignedVarInt();
        return (value & 1) ? ~(value >> 1) : (value >> 1);
    }

    [[nodiscard]] constexpr std::string getString() noexcept {
        uint32_t length = getUnsignedVarInt();
        if (mHasOverflowed || length == 0) {
            return {};
        }

        if (mReadPointer + length > mBufferView.size()) {
            mHasOverflowed = true;
            return {};
        }

        std::string result = std::string(mBufferView.substr(mReadPointer, length));
        mReadPointer += length;
        return result;
    }
};