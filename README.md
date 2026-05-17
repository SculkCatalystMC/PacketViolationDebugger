# PacketViolationDebugger

[简体中文](README.zh.md) | **English**

## Overview

**PacketViolationDebugger** is a native plugin for Minecraft Bedrock Edition that intercepts and debugs network packet violations. It supports both Server (BDS) and Client modes, printing packet error messages and detailed violation information to the console for debugging and development purposes.

## Features

- 🎮 **Native Bedrock Plugin**: Full support for Minecraft Bedrock Edition
- 🖥️ **Dual Mode Support**: Works with both Bedrock Dedicated Server (BDS) and client instances
- 🔍 **Packet Interception**: Captures and analyzes network packet violations
- 📋 **Detailed Logging**: Prints comprehensive error messages and violation details to the console
- ⚡ **High Performance**: Optimized with MSVC compiler flags for minimal overhead
- 🛡️ **Memory Safe**: Built with modern C++23 standards and memory safety checks

## Supported Packet Violations

The debugger can detect and report various packet violation types, including:
- Invalid packet structure
- Out-of-range values
- Protocol version mismatches
- Malformed data streams

## Requirements

- Windows Operating System
- MSVC Compiler (Visual Studio 2019 or later)
- CMake 3.24 or higher
- Minecraft Bedrock Edition

## Build Instructions

### Prerequisites

1. Install [CMake](https://cmake.org/) 3.24 or later
2. Ensure you have MSVC compiler installed (Visual Studio Build Tools or Visual Studio Community)
3. Clone the repository

### Building

```bash
# Navigate to the project directory
cd PacketViolationDebugger

# Create a build directory
mkdir build
cd build

# Configure the project
cmake ..

# Build the project
cmake --build . --config Release
```

The compiled DLL will be output to `bin/PacketViolationDebugger.dll`

## Usage

### Server Mode (BDS)

1. Place `PacketViolationDebugger.dll` in your BDS plugins directory
2. Launch the server
3. Packet violations will be logged to the console

### Client Mode

1. Load the DLL using a Minecraft client mod loader
2. Packet violations will be displayed in the game console

## Project Structure

```
PacketViolationDebugger/
├── src/
│   ├── PacketViolationDebugger.cpp    # Main entry point
│   ├── PacketViolationType.hpp        # Packet violation type definitions
│   ├── PacketViolationSeverity.hpp    # Severity level definitions
│   ├── MinecraftPacketIds.hpp         # Minecraft packet ID mappings
│   ├── Version.hpp                    # Version information
│   └── deps/
│       └── memory/                    # Memory manipulation utilities
│           ├── Hook.hpp               # Hook registration templates
│           ├── Memory.hpp             # Memory operation utilities
│           └── GlobalThreadPauser.hpp # Thread pausing mechanism
├── CMakeLists.txt                     # CMake build configuration
└── README.md                          # This file
```

## Dependencies

- **Detours**: Microsoft's library for function hooking (fetched via CMake FetchContent)
- **magic_enum**: Header-only library for enum reflection (fetched via CMake FetchContent)

## Configuration

The project uses modern C++23 standards with the following compiler settings:

- **Standard**: C++23 with no extensions
- **Optimization**: `/O2 /Ob3` (Full optimization with inline expansion)
- **Error Handling**: `/EHsc` (C++ exception handling)
- **Runtime Checks**: Enabled with `/GR-` (RTTI disabled for performance)

## Building with Custom Options

You can customize the build with CMake options:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Troubleshooting

### Build Failures

- Ensure MSVC is properly installed and added to PATH
- Verify CMake version is 3.24 or higher
- Clear the `build/` directory and reconfigure

### Missing Dependencies

- Dependencies are automatically fetched via FetchContent
- Ensure internet connection is available during first build
- Check `build/_deps/` for downloaded sources

## Contributing

Contributions are welcome! Please feel free to submit pull requests or report issues.

## License

Please check the repository for the license information.

## Version

Current Version: See [Version.hpp](src/Version.hpp) for details

## Changelog

For detailed changes, please refer to the git commit history.

### Copyright © 2026 SculkCatalystMC. All rights reserved.