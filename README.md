# HFProtocol / shortWaveLib

短波高频（HF）通信协议识别与解调工程，包含 GUI 分析工具、协议检测库和控制台示例程序。

本仓库主要是 C++/MFC + Intel IPP 的 Windows 工程，核心目标是对多种体制波形进行检测、解调与结果输出。

## 1. 项目概览

- 主体语言与框架：C++、MFC（动态链接）
- 核心计算库：Intel IPP（工程中主要按 6.0.2.074 配置）
- 典型输入：`.wav` / `.dat` 信号文件（示例通常按 `short` 二进制流读取）
- 主要产物：
  - GUI 应用（HFProtocol）
  - 协议检测与解调 DLL
  - 控制台 Demo（按协议编号执行不同解调流程）

## 2. 目录结构

```text
E:\HFProtocol
├─ HFProtocol
│  ├─ HFProtocol_1107                # 主解决方案（GUI + 多个协议子工程）
│  ├─ dataset                         # 样例数据与时频图
│  └─ Intel+IPP                       # IPP 安装包与文档（离线）
└─ shortWaveLib
   ├─ LINK11CLEW                      # 控制台解调 demo（main.cpp）
   ├─ protocolDetec                   # 协议检测 demo
   ├─ shortWaveProtocalWaveLib        # 协议头文件接口
   ├─ shortWave-Package               # 打包数据与 release 二进制
   └─ LINK11CLEW.sln                  # demo 解决方案
```

## 3. 主要工程与功能

### 3.1 `HFProtocol/HFProtocol_1107/HFProtocol.sln`

该解决方案包含以下核心项目（节选）：

- `HFProtocol`：MFC GUI 主程序
- `ProtolDetect`：协议检测 DLL
- `Link11CLEW` / `Link11SLEW`
- `MIL110Apro` / `MIL110Bpro`
- `MIL141Apro` / `MIL141Bpro`
- `NATO4285pro` / `NATO4529pro`
- `Test`：测试/联调工程

用途：适合完整开发、GUI 交互分析、子模块联编。

### 3.2 `shortWaveLib/LINK11CLEW.sln`

包含两个控制台相关项目：

- `LINK11CLEW`：通用协议解调示例（通过输入协议编号选择流程）
- `protocolDetec`：协议检测示例

用途：适合脚本化调用、快速验收、批处理测试。

## 4. 支持的协议（以 `shortWaveLib/LINK11CLEW/main.cpp` 为准）

| 编号 | 协议名 |
|---|---|
| 1 | Link11 CLEW |
| 2 | MIL110A |
| 3 | MIL110B |
| 4 | MIL141A |
| 5 | MIL141B |
| 6 | STANAG 4285 |
| 7 | STANAG 4529 |
| 8 | Link11 SLEW |
| 9 | KG84 FSK2 |
| 10 | FSK2_BCH |
| 11 | PI4QPSK |

## 5. 环境要求

- 操作系统：Windows（建议 Windows 10/11）
- IDE：Visual Studio 2022（需安装 C++ 桌面开发 + MFC）
- 工具集：`v143`（工程文件已配置）
- Intel IPP：工程默认指向 `C:\Program Files (x86)\Intel\IPP\6.0.2.074\...`

仓库中提供了离线包（可选）：

- `HFProtocol/Intel+IPP/Intel IPP/w_ipp_ia32_p_6.0.2.074.exe`
- `HFProtocol/Intel+IPP/Intel IPP/ippsman.pdf`

## 6. 快速开始

### 6.1 路线 A：直接使用打包产物（最快）

可直接使用：

- `shortWaveLib/shortWave-Package/shortWaveLib-release/LINK11CLEW.exe`
- 配套 DLL 与样例数据在同目录与 `data/` 目录中。

示例数据：

- `shortWaveLib/shortWave-Package/data/M110B.wav`
- `shortWaveLib/shortWave-Package/data/STANAG-S4285.wav`
- `shortWaveLib/shortWave-Package/data/PI4QPSK_9600.dat`

运行方式（PowerShell）：

```powershell
$exe = "E:\HFProtocol\shortWaveLib\shortWave-Package\shortWaveLib-release\LINK11CLEW.exe"
@(
  "E:\HFProtocol\shortWaveLib\shortWave-Package\data\M110B.wav"  # 输入文件
  "3"                                                            # 协议编号：MIL110B
) | & $exe
```

### 6.2 路线 B：编译并运行 GUI（HFProtocol）

1. 用 Visual Studio 打开：
   - `HFProtocol/HFProtocol_1107/HFProtocol.sln`
2. 选择配置：
   - 建议 `Release | Win32`（该工程多处按 x86/ia32 链接）
3. 生成解决方案。
4. 启动 `HFProtocol` 项目。
5. 在 GUI 中通过菜单打开 `.wav/.dat` 文件进行分析识别。

### 6.3 路线 C：编译控制台 Demo（shortWaveLib）

1. 打开：
   - `shortWaveLib/LINK11CLEW.sln`
2. 构建 `LINK11CLEW`（建议先试 `Release | Win32`）。
3. 运行后按顺序输入：
   - 第一行：待处理文件路径
   - 第二行：协议编号（见第 4 节）

示例：

```text
E:\HFProtocol\shortWaveLib\shortWave-Package\data\STANAG-S4285.wav
6
```

## 7. 输入输出说明

### 7.1 输入

- 代码中大量示例按二进制 `short` 流读取输入文件。
- 典型采样率在示例中常见 `9.6kHz`（不同解调函数有各自参数）。
- 若用 `.wav`，请优先使用示例目录中的同类型文件做对照测试。

### 7.2 输出

- 控制台程序直接打印检测/解调结果（协议名、起止时间、频率、帧信息等）。
- `HFProtocol_1107/Test/demod/` 下可见历史联调输出样例（`.txt/.bit/.demode`）。

## 8. 常见问题（FAQ）

### Q1: 编译报 `ipps.lib` / `ippsr.lib` 找不到

请确认 IPP 安装路径与工程属性一致，或在 VS 中补充：

- `Include Directories`
- `Library Directories`
- `Executable Path`（运行时 DLL 查找）

并优先使用 Win32 配置进行首轮验证。

### Q2: 运行时报缺少 DLL（如 `ipps-6.0.dll`）

将相关 DLL 放到可执行文件同目录，或加入系统 `PATH`。  
`shortWaveLib/shortWave-Package/shortWaveLib-release/` 已提供常见运行 DLL。

### Q3: x64 能否直接替代 Win32

仓库里存在 x64 配置，但多个工程默认依赖仍偏向 `ia32` 路径。  
建议先在 Win32 跑通，再逐步迁移并修正所有 include/lib/runtime 路径。

## 9. 开发建议

- 先用 `shortWave-Package` 的样例数据验证运行链路，再替换为自有数据。
- 对控制台程序，建议在外层加脚本统一喂入文件路径和协议编号。
- 若要做批处理，优先基于 `shortWaveLib/LINK11CLEW/main.cpp` 封装参数化入口（替换标准输入交互）。

## 10. 关键路径索引

- 主解决方案：`HFProtocol/HFProtocol_1107/HFProtocol.sln`
- GUI 主程序：`HFProtocol/HFProtocol_1107/HFProtocol/`
- 协议检测头：`shortWaveLib/protocolDetec/ProtolDetect.h`
- 控制台入口：`shortWaveLib/LINK11CLEW/main.cpp`
- 控制台解决方案：`shortWaveLib/LINK11CLEW.sln`
- 样例数据：`HFProtocol/dataset/` 与 `shortWaveLib/shortWave-Package/data/`

## 11. 说明

- 当前仓库未提供完整官方 README，本文件根据现有工程配置与源码入口整理。
- 如果你希望，我可以继续补一份英文版 README，或再加一节“协议开发接口速查”（按每个 DLL 的 init/demode/free 三段式 API 列表）。
