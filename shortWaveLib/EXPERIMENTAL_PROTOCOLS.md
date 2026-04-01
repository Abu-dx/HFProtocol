# 实验性协议级检测说明

本文档说明 `3g-ale`、`andvt`、`link22-candidate` 三种实验性协议级检测入口的当前状态。

这一版的重点已经从“像不像某个波形家族”收紧为“证据链是否足够支撑协议级结论”。尤其是 `3g-ale`，现在不会再因为 `BW1/BW3/TLC` 组合像就直接升格。

## 1. 当前状态

### 1.1 `3g-ale`

- 当前状态：已接入严格版 v1 检测链
- 当前冻结状态：`strict_v1_safe_fallback_complete`
- 入口：
  - `LINK11CLEW.exe -p 3g-ale`
  - `LINK11CLEW.exe -p auto`
  - `protocolDetec.exe -p 3g-ale`
  - `protocolDetec.exe -p auto`
- 实现原则：
  - 复用现有 `MIL141B` 前端
  - 先尝试 `BW0` 检测与结构化输出
  - 只有同时满足 `合法 BW0 + 可解释 LE_PDU + CRC/字段一致性 + timing 合理` 才输出 `3g-ale`
  - 否则回落为 `mil141b`

当前已实现的模块：

- `MIL141Bpro / ProtolDetect` 头文件与导出来源统一
- `BW0` 结构化输出骨架：
  - `start/end`
  - `center_hz`
  - `headType`
  - `walsh_scores[]`
  - `deinterleaved_bits`
  - `viterbi_bits`
  - `decode_quality`
- `BW0BurstCandidate[]` 上抛到协议层
- `LE_PDU / CRC / timing / transaction` 的 v1 结构化判决框架
- `confidence` 分项评分：
  - `bw0_score`
  - `pdu_score`
  - `crc_score`
  - `timing_score`
  - `transaction_score`

当前重要结论：

- 仓库自带正样本 `E:/HFProtocol/shortWaveLib/shortWave-Package/data/3G-ALE.wav`
  在现有前端下，仍然只稳定检出 `BW1/BW3/TLC`
- 当前没有获得足够的 `BW0 + LE_PDU` 证据
- 因此新检测器会**有意**回落为 `mil141b`
- 当前对这份样本的三选一诊断结论是：`sample_not_suitable_as_v1_primary_positive`
- 当前观测到的 burst 家族组合是：`TLC + BW1 + BW3`，没有可用 `BW0`
- 这是当前版本的正确行为，不是误报

当前样例命令：

```powershell
E:/HFProtocol/shortWaveLib/Release/LINK11CLEW.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/3G-ALE.wav -p 3g-ale
E:/HFProtocol/shortWaveLib/Release/LINK11CLEW.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/3G-ALE.wav -p auto
```

当前预期：

- `-p 3g-ale` 会输出结构化 JSON
- 当前样例的 `protocol` 仍会是 `mil141b`
- `-p auto` 会把该样例识别为 `mil141b`

这说明：

- 负样本抑制和严格回落逻辑已经生效
- 但 `BW0` 主证据链还需要继续收敛

### 1.2 `andvt`

- 当前状态：已接入手动候选检测
- 使用方式：
  - `LINK11CLEW.exe -p andvt`
  - `protocolDetec.exe -p andvt`
- 当前不接入 `auto`
- 判定基础：
  - 多音频谱峰数量
  - 多音频谱覆盖宽度
  - 峰值密度比例
- 当前目标：
  - 只做协议级候选检测
  - 不做语音恢复
  - 不做内容解译

### 1.3 `link22-candidate`

- 当前状态：已接入手动 bearer candidate 检测
- 使用方式：
  - `LINK11CLEW.exe -p link22-candidate`
  - `protocolDetec.exe -p link22-candidate`
- 当前不接入 `auto`
- 判定基础：
  - 复用 `NATO4529 / STANAG 4539` 承载波形前端
- 当前含义：
  - 只表示“与 Link22 承载波形一致的候选”
  - 不代表已经完成 `Link22` 协议层识别

## 2. `3g-ale` 输出格式

当前 `LINK11CLEW.exe -p 3g-ale` 输出为结构化 JSON，核心字段如下：

```json
{
  "protocol": "3g-ale | mil141b | unknown",
  "sample_assessment": "sample_contains_usable_bw0 | sample_contains_bw0_but_frontend_cannot_lock | sample_not_suitable_as_v1_primary_positive | unknown",
  "confidence": 0.0,
  "scores": {
    "bw0_score": 0.0,
    "pdu_score": 0.0,
    "crc_score": 0.0,
    "timing_score": 0.0,
    "transaction_score": 0.0
  },
  "bursts": [],
  "pdus": [],
  "transaction_type": "async_scan_call | async_unicast_call | sync_call | unknown",
  "timing_ok": true,
  "notes": []
}
```

说明：

- `protocol`
  - 对外正式协议名
- `scores`
  - 分项分数，便于判断问题卡在 `BW0 / PDU / CRC / timing / transaction` 的哪一层
- `bursts`
  - 当前已识别的 `MIL141B` 家族 burst
- `pdus`
  - 当前从 `BW0` 中解释出来的 PDU 候选
- `notes`
  - 当前判决的解释性备注
- `sample_assessment`
  - 当前对样本真实性的三选一诊断结论

## 3. 示例命令

### 3.1 `LINK11CLEW.exe`

```powershell
E:/HFProtocol/shortWaveLib/Release/LINK11CLEW.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/3G-ALE.wav -p 3g-ale
E:/HFProtocol/shortWaveLib/Release/LINK11CLEW.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/3G-ALE.wav -p auto
E:/HFProtocol/shortWaveLib/Release/LINK11CLEW.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/ANDVT.wav -p andvt
E:/HFProtocol/shortWaveLib/Release/LINK11CLEW.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/STANAG-4529-uncoded.wav -p link22-candidate
```

### 3.2 `protocolDetec.exe`

```powershell
E:/HFProtocol/shortWaveLib/Release/protocolDetec.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/3G-ALE.wav -p 3g-ale
E:/HFProtocol/shortWaveLib/Release/protocolDetec.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/3G-ALE.wav -p auto
E:/HFProtocol/shortWaveLib/Release/protocolDetec.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/ANDVT.wav -p andvt
E:/HFProtocol/shortWaveLib/Release/protocolDetec.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/STANAG-4529-uncoded.wav -p link22-candidate
```

如果命令行仍提示不认识 `3g-ale`，通常说明运行的还是旧版 `protocolDetec.exe`，需要重新编译 `shortWaveLib/LINK11CLEW.sln` 的 `Release | Win32` 产物。

## 4. 已完成与未完成

### 已完成

- `3g-ale` 检测入口已接入
- 严格回落规则已接入
- `BW0` 结构化输出骨架已接入
- 分项评分与统一 JSON 输出已接入
- 对 `2G-ALE` 负样本不会普遍误报为 `3g-ale`

### 仍是启发式或待收敛

- `LE_PDU` 字段解释
- `CRC` 多项式与位序的最终规范固化
- `timing / transaction` 细节门限
- `BW0` 前端抓取能力

### 当前最大阻塞

当前仓库自带的 `3G-ALE.wav` 在现有前端下没有提供足够的 `BW0` 主证据，因此还不能稳定通过：

- `LE_Scanning_Call`
- `LE_Call`
- `LE_Handshake`

这也是当前还不能把样例正式输出为 `3g-ale` 的直接原因。

## 5. 位序冻结（v1）

当前代码中已经把 3G-ALE v1 的位序假设冻结为显式常量，位置在：

- `E:/HFProtocol/shortWaveLib/common/G3AlePdu.h`
- `WalshToDeinterleaveBitOrdering()`
- `DeinterleaveBitOrdering()`
- `ViterbiBitOrdering()`
- `FigureC17BitOrdering()`

当前实现约定：

- Walsh 判决输出按 `b3 b2 b1 b0` 进入去交织输入
- 去交织输出按恢复后的行列顺序左到右输出
- Viterbi 输出按 trellis decoded order，按 `MSB-first` 打包成 26-bit 候选
- Figure C-17 字段解释使用 `MSB-first`、左到右的候选位序
- CRC 输入位顺序与字段拼接顺序当前也按上述位序固化

注意：CRC 多项式与位序已经写入代码骨架，但仍需要继续对照 `141B Appendix C.5.2.2.6` 做页级核实，当前还不能宣称“最终完全定版”。


## 6. 当前冻结状态

当前 3G-ALE 严格版 v1 已冻结为 strict_v1_safe_fallback_complete。

详细状态说明见：

- [3G_ALE_STRICT_V1_STATUS.md](E:/HFProtocol/shortWaveLib/3G_ALE_STRICT_V1_STATUS.md)

