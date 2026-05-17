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
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

#define _STRINGIZE(S) #S
#define STRINGIZE(S)  _STRINGIZE(S)

#define FILE_VERSION_STRING STRINGIZE(VERSION_MAJOR.VERSION_MINOR.VERSION_PATCH)

#define FILE_VERSION_FLAG 0x0L
