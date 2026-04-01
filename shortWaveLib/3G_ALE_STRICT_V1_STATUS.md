# 3G-ALE strict_v1_safe_fallback_complete 状态说明

## 结论

当前仓库中的 3G-ALE 严格版 v1 已冻结为：`strict_v1_safe_fallback_complete`。

这表示：

- `3g-ale` 的严格判决框架已经接通
- 对近邻负样本的误升格已经被抑制
- 当前仓库自带正样本 `3G-ALE.wav` 经过最终离线切片确认后，仍未提供可用于 v1 验收的 `BW0` 主证据
- 因此当前版本停止继续深挖这份样本的前端锁定问题
- 在补充新的、包含可观测 `BW0` 主路径的 3G-ALE 正样本之前，不继续推进 `LE_PDU / transaction` 的最终验收

## 最终离线切片确认

本轮用于确认的证据来源：

1. `E:/HFProtocol/shortWaveLib/Release/LINK11CLEW.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/3G-ALE.wav -p 3g-ale`
2. `E:/HFProtocol/shortWaveLib/Release/LINK11CLEW.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/3G-ALE.wav -p mil141b`
3. `E:/HFProtocol/shortWaveLib/Release/protocolDetec.exe E:/HFProtocol/shortWaveLib/shortWave-Package/data/3G-ALE.wav -p 3g-ale`
4. `E:/HFProtocol/HFProtocol/HFProtocol_1107/Test/demod/3G-ALE.dat.txt`

最终一致结论：

- 当前样本稳定观测到的 burst 家族是：`TLC + BW1 + BW3`
- 当前没有观测到可用 `BW0`
- 当前没有形成 26-bit `LE_PDU` 候选
- 当前没有进入可验证的 CRC 路径
- 当前没有进入可验证的 sync/async 事务链

因此三选一诊断结论固定为：

- `sample_not_suitable_as_v1_primary_positive`

## 当前严格版 v1 已完成内容

- 头文件与导出路径统一
- `BW0BurstCandidate[]` 上抛
- 统一结构化 JSON 输出
- 分项评分：
  - `bw0_score`
  - `pdu_score`
  - `crc_score`
  - `timing_score`
  - `transaction_score`
- 严格回落规则：只有 `BW0 + LE_PDU + CRC/字段一致性 + timing` 同时成立才输出 `3g-ale`
- 当前样本若证据不足，则稳定回落为 `mil141b`
- `2G-ALE` 不会被普遍误报成 `3g-ale`

## 位序冻结（当前版本）

当前位序冻结位置：

- `E:/HFProtocol/shortWaveLib/common/G3AlePdu.h`

当前冻结内容：

- `WalshToDeinterleaveBitOrdering()`
- `DeinterleaveBitOrdering()`
- `ViterbiBitOrdering()`
- `FigureC17BitOrdering()`

当前实现约定：

- Walsh 判决输出按 `b3 b2 b1 b0` 进入去交织输入
- 去交织输出按恢复后的行列顺序左到右输出
- Viterbi 输出按 decoded trellis order，按 `MSB-first` 打包成 26-bit 候选
- Figure C-17 字段解释使用 `MSB-first`、左到右位序
- CRC 输入位顺序和字段拼接顺序暂按同一位序固化

## CRC / 字段核对状态

当前 CRC 实现位置：

- `E:/HFProtocol/shortWaveLib/common/G3AleCrc.h`
- `E:/HFProtocol/shortWaveLib/common/G3AlePdu.h`

当前状态：

- 已有 `crc4` 与 `crc8` 代码骨架
- 已有字段切片与 `LE_Call / LE_Handshake / LE_Scanning_Call / LE_Broadcast` 的最小解释路径
- 但由于当前样本没有进入可用 `BW0` 主路径，CRC 仍未完成样本级闭环验证
- 因此当前 CRC/字段核对状态定义为：`code_frozen_but_not_sample_validated`

这意味着：

- 代码中的位序、字段拼接和 CRC 接口已经冻结
- 但还不能宣称 Appendix C.5.2.2.6 已经通过正样本页级验证

## reject reason 汇总

当前 `BW0` 候选拒绝原因固定为：

- `duration_out_of_range`
- `weak_preamble`
- `no_stable_walsh_decode`
- `bad_deinterleave`
- `not_26bit_candidate`
- `bad_viterbi_output`
- `bit_order_unknown`

说明：

- 前 6 项来自 `E:/HFProtocol/shortWaveLib/common/G3AleDetector.h`
- `bit_order_unknown` 来自 `E:/HFProtocol/shortWaveLib/common/G3AlePdu.h`
- 当前样本因为没有形成 `BW0` 候选，所以这些 reject reason 尚未在 `3G-ALE.wav` 上批量触发；它们当前是“冻结的诊断分类”，不是“已由当前正样本全部覆盖”的结论

## 当前边界

当前版本明确不做：

- 继续在当前 `3G-ALE.wav` 上无限迭代前端
- 在缺少 `BW0` 主证据时冒进输出 `3g-ale`
- 在当前样本上继续推进 `LE_PDU / transaction` 最终验收

## 重新启动 3G-ALE 最终验收的前提

只有在补充新的、包含可观测 `BW0` 主路径的 3G-ALE 正样本后，才继续推进：

- `LE_PDU` 主路径识别
- CRC 样本级闭环验证
- async/sync transaction 最终验收
- `3g-ale` 正式升格条件的最终确认
