# 短波命令行输入说明

本文档说明 `LINK11CLEW` 和 `protocolDetec` 两个入口程序在命令行与交互输入层面的改动。
解调与检测的核心接口没有被修改。

## 本轮新增内容概览

这一轮主要补充了以下能力：

1. 更友好的输入方式
- 支持 Windows 反斜杠路径、斜杠路径、相对路径、仅文件名输入
- 支持在内置目录和 `--search-dir` 指定目录中自动搜索文件
- 支持命令行模式和交互模式两种入口

2. 更方便的协议选择方式
- 支持 `-p/--protocol`
- 支持协议别名，如 `link11clew`、`mil110a`、`4285`、`auto`
- 无参数运行时会显示数字菜单

3. 主程序支持自动检测并分发
- `LINK11CLEW.exe` 可以直接使用 `-p auto`
- 自动检测成功后会自动调用对应解调器
- 自动检测链路已修复，可直接对官方样例使用

4. WAV 输入更安全
- 自动识别标准 `RIFF/WAVE`
- 自动跳过 WAV 头，只读取采样数据
- 当前支持 `16-bit 单声道 PCM WAV`
- 对不支持的 WAV 格式会直接报错

5. 初始参数支持命令行覆盖
- 支持 `--frequency`
- 支持 `--data-rate`
- 适用于入口层里已有相应初始化参数的协议

6. 已修复的入口层问题
- `protocolDetec` 不再写死输入文件路径
- `MIL141B` 的 `delete[]` 释放问题已修复
- `MIL141B` 的 `EndTime` 未初始化输出问题已修复
- 多个错误的完成提示文案已修正

## 本轮新增功能怎么用

### 1. 直接传文件路径

适用场景：
- 你已经知道文件完整路径
- 想直接指定协议或自动检测

示例命令：

```powershell
.\LINK11CLEW.exe E:\data\file.wav
.\LINK11CLEW.exe "E:\data\file.wav" -p link11clew
.\LINK11CLEW.exe E:/data/file.wav -p mil110a
```

### 2. 使用相对路径或仅文件名

适用场景：
- 当前目录就在数据附近
- 不想每次手动写很长的绝对路径

示例命令：

```powershell
.\LINK11CLEW.exe .\file.wav -p auto
.\LINK11CLEW.exe file.wav -p 4285 --search-dir E:\HFProtocol\shortWaveLib\shortWave-Package\data
.\protocolDetec.exe file.wav -p mil110b --search-dir E:\HFProtocol\shortWaveLib\shortWave-Package\data
```

### 3. 使用自动检测

适用场景：
- 只知道输入文件，不确定具体协议
- 希望先检测，再自动进入对应解调器

示例命令：

```powershell
.\LINK11CLEW.exe file.wav
.\LINK11CLEW.exe file.wav -p auto
.\protocolDetec.exe file.wav -p auto
.\LINK11CLEW.exe E:\HFProtocol\shortWaveLib\shortWave-Package\data\Link11-CLEW.wav -p auto
.\LINK11CLEW.exe E:\HFProtocol\shortWaveLib\shortWave-Package\data\M110B.wav -p auto
```

说明：
- `LINK11CLEW.exe` 中的 `auto` 会先做检测，再自动进入匹配的解调器
- `protocolDetec.exe` 中的 `auto` 会输出检测结果，便于单独验证检测流程
- 推荐构建配置为 `Release | Win32`

### 4. 使用交互模式

适用场景：
- 不想记参数
- 想直接运行程序后按提示输入

示例命令：

```powershell
.\LINK11CLEW.exe
.\protocolDetec.exe
```

运行后：

1. 输入文件路径
2. 选择协议编号
3. 等待检测或解调输出

### 5. 覆盖初始参数

适用场景：
- 某些协议需要手工指定更合适的中心频率或数据率
- 希望在不改源码的情况下测试不同初始参数

示例命令：

```powershell
.\LINK11CLEW.exe file.wav -p 4285 --frequency 1800 --data-rate 2400
.\LINK11CLEW.exe file.wav -p 4529 --frequency 2000 --data-rate 1800
```

说明：
- `--frequency` 用于覆盖入口层中的初始中心频率
- `--data-rate` 用于覆盖入口层中的初始数据率
- 如果某个协议入口没有使用这两个初始量，则对应参数不会强行影响算法内部流程

### 6. 使用 WAV 文件

适用场景：
- 输入文件是标准 WAV
- 希望程序自动跳过 WAV 头，不要把头部误当成信号样本

示例命令：

```powershell
.\LINK11CLEW.exe E:\data\sample.wav -p auto
.\protocolDetec.exe E:\data\sample.wav
```

当前支持：

- `16-bit`
- `mono`
- `PCM WAV`

不支持时的处理：

- 程序会直接提示格式不支持
- 不会把错误格式的数据继续送进检测或解调流程

## 已修改文件

- `shortWaveLib/common/InputAudio.h`
- `shortWaveLib/common/InputParser.h`
- `shortWaveLib/LINK11CLEW.sln`
- `shortWaveLib/LINK11CLEW/main.cpp`
- `shortWaveLib/LINK11CLEW/LINK11CLEW.vcxproj`
- `shortWaveLib/protocolDetec/protocolDetec.cpp`
- `shortWaveLib/protocolDetec/protocolDetec.vcxproj`
- `shortWaveLib/shortWaveProtocalWaveLib/ProtolDetect.h`

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

现在入口层还会自动识别标准 WAV 头：

- 如果输入是原始 `short` 数据流，会按原逻辑直接读取
- 如果输入是标准 PCM WAV，会自动跳过 WAV 头，只把采样数据送入检测或解调流程
- 当前支持的 WAV 形式为：`16-bit 单声道 PCM WAV`
- 如果是其他 WAV 格式，程序会直接提示不支持，避免把 WAV 头或错误数据送进算法

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
- `--frequency` 可用于覆盖部分协议的初始中心频率。
- `--data-rate` 可用于覆盖部分协议的初始数据率。

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

补充：

- `3g-ale` 的实验性协议级检测已经接入 `mil141b` 的二级细分
- 但它现在使用严格证据链判决，不会只凭 `BW1/BW3/TLC` 组合就自动升格
- 如果缺少合法 `BW0 + LE_PDU` 证据，`auto` 会继续保留为 `mil141b`

自动检测修复完成后，官方样例已验证可用：

- `Link11-CLEW.wav -> link11clew`
- `M110B.wav -> mil110b`

代码位置：

- `shortWaveLib/LINK11CLEW/main.cpp`
- `shortWaveLib/LINK11CLEW/LINK11CLEW.vcxproj`
- `HFProtocol/HFProtocol_1107/ProtolDetect/ProtolDetect.cpp`
- `shortWaveLib/shortWaveProtocalWaveLib/ProtolDetect.h`

使用方式：

- `LINK11CLEW.exe file.wav -p auto`
- `LINK11CLEW.exe file.wav`

实现说明：

- 检测器只负责决定“调用哪个解调器”。
- 检测完成后，仍按原来的方式调用现有解调接口。
- 自动检测阶段同样会跳过标准 WAV 文件头。

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
- 新增统一的 PCM/WAV 输入打开逻辑，避免把 WAV 头误当成样本
- 新增 `--frequency` 和 `--data-rate` 的入口层初始参数覆盖
- 修复 `MIL141B` 的 `delete[]` 释放问题
- 修复 `MIL141B` 的 `EndTime` 未初始化输出问题
- 去掉控制台退出时的 `system("pause")`

### `shortWaveLib/LINK11CLEW/LINK11CLEW.vcxproj`

- 为 Win32 Debug 和 Release 配置补充 `ProtolDetect.lib` 链接
- 将 `HFProtocol_1107/ProtolDetect` 设为项目引用，避免主程序继续链接旧版检测库
- 构建后自动复制匹配版本的 `ProtolDetect.dll`
- 保持 x64 配置不变，避免影响当前更稳定的 Win32 使用路径

### `shortWaveLib/LINK11CLEW.sln`

- 将 `ProtolDetect` 项目加入 `shortWaveLib` 解决方案
- 让 `Clean/Rebuild Solution` 时自动一起构建检测库

### `shortWaveLib/protocolDetec/protocolDetec.cpp`

- 移除硬编码的演示输入路径
- 新增帮助输出
- 新增与主程序一致的路径解析与搜索逻辑
- 新增 WAV 头识别与跳过逻辑
- 新增协议过滤别名
- 新增交互模式
- 保持检测器核心调用方式不变
- 新增 `DETECTED_PROTOCOL=...` 结果输出，方便验证

### `shortWaveLib/protocolDetec/protocolDetec.vcxproj`

- 与主程序一样改为优先链接 `HFProtocol_1107` 目录下的 `ProtolDetect`
- 构建后自动复制对应的 `ProtolDetect.dll`

### `shortWaveLib/common/InputAudio.h`

- 新增统一的原始 PCM/WAV 输入检查与打开逻辑
- 自动识别 `RIFF/WAVE`
- 自动定位 `data` 块起始位置
- 对不支持的 WAV 规格给出明确错误提示

## 如何使用

### LINK11CLEW 示例

```powershell
.\LINK11CLEW.exe E:\data\file.wav
.\LINK11CLEW.exe "E:\data\file.wav" -p link11clew
.\LINK11CLEW.exe .\file.wav -p auto
.\LINK11CLEW.exe file.wav -p 4285 --search-dir E:\HFProtocol\shortWaveLib\shortWave-Package\data
.\LINK11CLEW.exe file.wav -p 4285 --frequency 1800 --data-rate 2400
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
- 对标准 WAV 输入，程序会自动跳过文件头；对原始 `.dat` 或无头 `short` 数据流，程序仍按原逻辑读取。
- `--frequency` 和 `--data-rate` 只会覆盖入口层中已有对应初始化参数的协议，不会强行改动所有算法内部状态。
- 自动检测如果表现异常，优先确认是否使用了 `Release | Win32` 并且已经重新构建了 `ProtolDetect`。

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

## 实验性协议级检测

当前额外提供三类实验性协议级检测入口：

- `3g-ale`
- `andvt`
- `link22-candidate`

使用原则：

- `3g-ale`：手动可用，且 `auto` 在命中 `mil141b` 后会继续细分
- 当前 `3G-ALE.wav` 的结构化输出会给出 `sample_assessment=sample_not_suitable_as_v1_primary_positive`，说明样本里只稳定看到 `TLC/BW1/BW3`，没有可用 `BW0` 主证据
- 当前 3G-ALE 严格版 v1 已冻结为 `strict_v1_safe_fallback_complete`，详见 `E:/HFProtocol/shortWaveLib/3G_ALE_STRICT_V1_STATUS.md`
- `andvt`：仅手动候选检测，不进入 `auto`
- `link22-candidate`：仅手动 bearer candidate 检测，不进入 `auto`

快速示例：

```powershell
.\LINK11CLEW.exe E:\HFProtocol\shortWaveLib\shortWave-Package\data\3G-ALE.wav -p 3g-ale
.\LINK11CLEW.exe E:\HFProtocol\shortWaveLib\shortWave-Package\data\3G-ALE.wav -p auto
.\LINK11CLEW.exe E:\HFProtocol\shortWaveLib\shortWave-Package\data\ANDVT.wav -p andvt
.\LINK11CLEW.exe E:\HFProtocol\shortWaveLib\shortWave-Package\data\STANAG-4529-uncoded.wav -p link22-candidate
```

详细说明见：

- [EXPERIMENTAL_PROTOCOLS.md](E:/HFProtocol/shortWaveLib/EXPERIMENTAL_PROTOCOLS.md)


