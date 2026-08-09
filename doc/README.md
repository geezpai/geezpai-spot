# Geezpai Spot 开发文档

## 硬件参考

| 文档 | 说明 |
|------|------|
| [Geezpai_Spot-v1.0_规格书.md](Geezpai_Spot-v1.0_规格书.md) | 产品规格书 (芯片/接口/功耗/IO) |
| [Geezpai_Spot_开发板说明.md](Geezpai_Spot_开发板说明.md) | 引脚映射、跳线配置、供电说明、外设接口 |
| [nRF52840_IO_Map.md](nRF52840_IO_Map.md) | nRF52840 全部 IO 功能分配速查表 |
| [Geezpai_Spot-v1.0.pdf](Geezpai_Spot-v1.0.pdf) | PCB 原理图 |
| [pcb.png](pcb.png) | PCB 实物照片 |

## 芯片数据手册

| 文档 | 芯片 |
|------|------|
| [ST7735S芯片手册.pdf](ST7735S芯片手册.pdf) | 0.96" TFT LCD 驱动 IC |
| [DS_da217.pdf](DS_da217.pdf) | DA217 G-Sensor |
| [C161819_蜂鸣器_QMB-09B-05_规格书_WJ1760500.PDF](C161819_蜂鸣器_QMB-09B-05_规格书_WJ1760500.PDF) | 3V 有源蜂鸣器 |

## 软件开发

| 文档 | 说明 |
|------|------|
| [../README.md](../README.md) | 项目首页：快速开始、功能特性、编译命令 |
| [../examples/README.md](../examples/README.md) | 示例索引：6 级 15+ 示例路线图 |
| [../examples/05_findmy/locator_tag/README.md](../examples/05_findmy/locator_tag/README.md) | 主力固件详解：FHN 工作流程、UWB 集成状态 |

## 已知问题

DA217 G-Sensor 运动中断在特定条件下可能不触发，调试中。
