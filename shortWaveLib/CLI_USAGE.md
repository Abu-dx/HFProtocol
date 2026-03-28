# 短波命令行输入说明

本文档说明 `LINK11CLEW` 和 `protocolDetec` 两个入口程序在命令行与交互输入层面的改动。
解调与检测的核心接口没有被修改。

## 已修改文件

- `shortWaveLib/common/InputParser.h`
- `shortWaveLib/LINK11CLEW/main.cpp`
- `shortWaveLib/LINK11CLEW/LINK11CLEW.vcxproj`
- `shortWaveLib/protocolDetec/protocolDetec.cpp`

## 改动内容

### 1. 输入路径更友好

两个可执行程序现在都支持以下几种路径形式：

- Windows 风格路径：`E:\data\file.wav`
- POSIX 风格路径：`E:/data/file.wav`
- 相对路径：`.\file.wav`
- 仅输入文件名：`file.wav`

当只输入文件名时，程序会按顺序在以下目录中查找：

- 当前工作目录
- 当前目录下的 `test_data`
- 当前目录或上级目录下的 `shortWave-Package/data`
- `D:/0_kunshan/短波数据db`
- 通过 `--search-dir` 额外传入的目录

成功匹配到文件后，程序会使用 `std::filesystem` 进行路径归一化，并转换为绝对路径后再继续处理。

代码位置：

- `shortWaveLib/common/InputParser.h`
- `shortWaveLib/LINK11CLEW/main.cpp`
- `shortWaveLib/protocolDetec/protocolDetec.cpp`

使用方式：

- 可以直接传完整的 Windows 路径。
- 可以直接传完整的斜杠路径。
- 可以在数据目录下运行程序并传相对路径。
- 可以只传文件名，并按需补充 `--search-dir`。

### 2. 协议选择更简单

`LINK11CLEW.exe` 现在支持：

- `-p link11clew`
- `-p mil110a`
- `-p 4285`
- `-p auto`

如果运行时没有传任何参数，程序会进入交互模式：

1. 提示输入文件路径
2. 显示协议数字菜单
3. 按选择调用对应解调器

代码位置：

- `shortWaveLib/LINK11CLEW/main.cpp`

使用方式：

- `-p auto` 表示由程序自动判断协议。
- `-p link11clew` 这类显式协议会跳过检测，直接进入对应解调器。
- 不带参数直接运行时，会进入交互菜单。

### 3. 主入口支持自动检测

`LINK11CLEW.exe -p auto` 现在会在入口层直接调用 `CProtolDetect`。
如果检测成功，程序会自动分发到匹配的解调器。

当前自动检测覆盖范围与检测 demo 保持一致，包含：

- `mil110a`
- `mil110b`
- `mil141a`
- `mil141b`
- `link11slew`
- `link11clew`
- `4285`
- `4529`

代码位置：

- `shortWaveLib/LINK11CLEW/main.cpp`
- `shortWaveLib/LINK11CLEW/LINK11CLEW.vcxproj`

使用方式：

- `LINK11CLEW.exe file.wav -p auto`
- `LINK11CLEW.exe file.wav`

实现说明：

- 检测器只负责决定“调用哪个解调器”。
- 检测完成后，仍按原来的方式调用现有解调接口。

### 4. `protocolDetec.exe` 可以直接使用了

原来写死在源码里的输入文件路径已经移除。
现在 `protocolDetec.exe` 支持：

- 命令行输入文件路径
- `-p` 协议过滤
- 交互模式
- `--help` 帮助输出

程序还会输出一行便于机器读取的结果：

```text
DETECTED_PROTOCOL=mil110b
```

这主要用于调试和验证自动检测流程。

代码位置：

- `shortWaveLib/protocolDetec/protocolDetec.cpp`

使用方式：

- `protocolDetec.exe file.wav`
- `protocolDetec.exe file.wav -p mil110b`
- 直接运行 `protocolDetec.exe`，然后按提示输入

### 5. 已包含的 bug 修复

- 将 `MIL141B` 的数组释放从 `delete` 修正为 `delete[]`
- 在输出前清空 `MIL141B` 的 `EndTime`，避免打印未初始化的脏数据

代码位置：

- `shortWaveLib/LINK11CLEW/main.cpp`

对使用者的影响：

- `MIL141B` 退出时不再带有明显的堆释放错误风险。
- `MIL141B` 的输出不会再出现随机的 `EndTime` 内容。

## 按文件汇总修复内容

### `shortWaveLib/common/InputParser.h`

- 新增可复用的命令行/交互输入解析器
- 新增基于 `std::filesystem::lexically_normal()` 的路径归一化
- 新增基于 `std::filesystem::absolute()` 的绝对路径转换
- 新增内置搜索目录与用户自定义搜索目录的文件名查找
- 新增 `-h`、`-i`、`-p`、`--search-dir` 等通用参数解析

### `shortWaveLib/LINK11CLEW/main.cpp`

- 替换原来 `std::cin >> filePath` 和纯数字协议分发流程
- 新增 `PrintUsage()` 帮助输出
- 新增 `PromptProtocolMenu()` 交互式协议菜单
- 新增 `link11clew`、`mil110a`、`4285`、`auto` 等协议别名映射
- 新增主入口直接调用 `CProtolDetect` 的自动检测流程
- 修复 `MIL141B` 的 `delete[]` 释放问题
- 修复 `MIL141B` 的 `EndTime` 未初始化输出问题
- 去掉控制台退出时的 `system("pause")`

### `shortWaveLib/LINK11CLEW/LINK11CLEW.vcxproj`

- 为 Win32 Debug 和 Release 配置补充 `ProtolDetect.lib` 链接
- 保持 x64 配置不变，避免影响当前更稳定的 Win32 使用路径

### `shortWaveLib/protocolDetec/protocolDetec.cpp`

- 移除硬编码的演示输入路径
- 新增帮助输出
- 新增与主程序一致的路径解析与搜索逻辑
- 新增协议过滤别名
- 新增交互模式
- 保持检测器核心调用方式不变
- 新增 `DETECTED_PROTOCOL=...` 结果输出，方便验证

## 如何使用

### LINK11CLEW 示例

```powershell
.\LINK11CLEW.exe E:\data\file.wav
.\LINK11CLEW.exe "E:\data\file.wav" -p link11clew
.\LINK11CLEW.exe .\file.wav -p auto
.\LINK11CLEW.exe file.wav -p 4285 --search-dir E:\HFProtocol\shortWaveLib\shortWave-Package\data
```

交互模式：

```powershell
.\LINK11CLEW.exe
```

然后按下面顺序操作：

1. 输入文件路径
2. 输入协议编号
3. 等待程序输出解调结果

### protocolDetec 示例

```powershell
.\protocolDetec.exe E:\data\file.wav
.\protocolDetec.exe .\file.wav -p auto
.\protocolDetec.exe file.wav -p mil110b --search-dir E:\HFProtocol\shortWaveLib\shortWave-Package\data
```

交互模式：

```powershell
.\protocolDetec.exe
```

## 说明

- `LINK11CLEW.exe` 的自动检测只负责选择要调用的解调器，不会改动解调器内部流程。
- `Link11CLEWdemode`、`MIL141Ademode`、`Demode_PSKrealFSE` 等核心接口没有修改。
- x64 配置目前保留了安全兜底逻辑；现阶段更推荐按现有工程结构使用 Win32。

## 手动验证步骤

请在完成 Win32 编译后，使用 Developer PowerShell 或 Visual Studio 命令行进行验证。

1. 绝对 Windows 路径

```powershell
.\LINK11CLEW.exe E:\data\file.wav -p link11clew
```

2. 绝对斜杠路径

```powershell
.\LINK11CLEW.exe E:/data/file.wav -p mil110a
```

3. 相对路径

```powershell
.\LINK11CLEW.exe .\shortWave-Package\data\sample.wav -p 4285
```

4. 仅文件名搜索

```powershell
.\LINK11CLEW.exe sample.wav -p auto --search-dir E:\HFProtocol\shortWaveLib\shortWave-Package\data
```

5. 交互模式

```powershell
.\LINK11CLEW.exe
```

6. 仅检测流程验证

```powershell
.\protocolDetec.exe sample.wav -p auto --search-dir E:\HFProtocol\shortWaveLib\shortWave-Package\data
```

验证时重点确认：

- 各种路径形式都能被正确接受
- `auto` 会打印检测结果并进入对应解调流程
- 显式 `-p` 会跳过自动检测
- 输入非法协议名时，程序会打印帮助而不是进入异常状态
