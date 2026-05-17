# PacketViolationDebugger

[English](README.md) | **简体中文**

## 项目简介

**PacketViolationDebugger** 是一个针对 Minecraft 基岩版的原生插件，用于拦截和调试网络数据包错误。它同时支持服务端（BDS）和客户端模式，可将数据包错误信息和详细的违规信息打印到控制台，方便开发者调试和开发。

## 主要特性

- 🎮 **原生基岩插件**：完全支持 Minecraft 基岩版
- 🖥️ **双模式支持**：支持基岩专用服务器（BDS）和客户端
- 🔍 **数据包拦截**：捕获并分析网络数据包违规
- 📋 **详细日志**：将详细的错误信息和违规信息打印到控制台
- ⚡ **高性能**：使用 MSVC 编译器优化标志，开销最小
- 🛡️ **内存安全**：采用现代 C++23 标准构建，具有内存安全检查

## 支持的数据包违规类型

调试器可以检测和报告各种数据包违规类型，包括：
- 无效的数据包结构
- 超出范围的值
- 协议版本不匹配
- 格式错误的数据流

## 系统要求

- Windows 操作系统
- MSVC 编译器（Visual Studio 2019 或更新版本）
- CMake 3.24 或更高版本
- Minecraft 基岩版

## 编译说明

### 前置要求

1. 安装 [CMake](https://cmake.org/) 3.24 或更新版本
2. 确保安装了 MSVC 编译器（Visual Studio 生成工具或 Visual Studio Community）
3. 克隆项目仓库

### 构建步骤

```bash
# 进入项目目录
cd PacketViolationDebugger

# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译项目
cmake --build . --config Release
```

编译后的 DLL 文件将输出到 `bin/PacketViolationDebugger.dll`

## 使用方法

### 服务端模式（BDS）

1. 将 `PacketViolationDebugger.dll` 放入 BDS 插件目录
2. 启动服务器
3. 数据包违规信息将被打印到控制台

### 客户端模式

1. 使用 Minecraft 客户端 Mod 加载器加载该 DLL
2. 数据包违规信息将显示在游戏控制台中

## 项目结构

```
PacketViolationDebugger/
├── src/
│   ├── PacketViolationDebugger.cpp    # 主入口
│   ├── PacketViolationType.hpp        # 数据包违规类型定义
│   ├── PacketViolationSeverity.hpp    # 严重级别定义
│   ├── MinecraftPacketIds.hpp         # Minecraft 数据包 ID 映射
│   ├── Version.hpp                    # 版本信息
│   └── deps/
│       └── memory/                    # 内存操作工具
│           ├── Hook.hpp               # Hook 注册模板
│           ├── Memory.hpp             # 内存操作工具
│           └── GlobalThreadPauser.hpp # 线程暂停机制
├── CMakeLists.txt                     # CMake 构建配置
└── README.zh.md                       # 本文件
```

## 依赖项

- **Detours**：微软的函数 Hook 库（通过 CMake FetchContent 自动获取）
- **magic_enum**：仅头文件的枚举反射库（通过 CMake FetchContent 自动获取）

## 构建配置

项目使用现代 C++23 标准，配置如下：

- **标准**：C++23（无扩展）
- **优化**：`/O2 /Ob3`（完全优化，包括内联展开）
- **错误处理**：`/EHsc`（C++ 异常处理）
- **运行时检查**：启用，使用 `/GR-`（禁用 RTTI 以提高性能）

## 自定义构建选项

你可以使用 CMake 选项自定义构建：

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## 常见问题排查

### 编译失败

- 确保 MSVC 已正确安装并添加到 PATH
- 验证 CMake 版本是否为 3.24 或更高
- 清除 `build/` 目录并重新配置

### 缺少依赖项

- 依赖项通过 FetchContent 自动获取
- 确保首次编译时有网络连接
- 检查 `build/_deps/` 中的下载源文件

## 贡献指南

欢迎提交 Pull Request 或报告 Issue！

## 许可证

请查看仓库中的许可证信息。

## 版本信息

当前版本：详见 [Version.hpp](src/Version.hpp)

## 更新日志

详细的更改信息请参考 Git 提交历史。

### 版权所有 © 2026 SculkCatalystMC。保留所有权利。

