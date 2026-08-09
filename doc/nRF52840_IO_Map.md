# nRF52840 IO Map (参考草案)

> ⚠️ **本文档为早期原理图阅读笔记，部分引脚标注可能不准确。**
> 开发请以 [Geezpai_Spot_开发板说明.md](Geezpai_Spot_开发板说明.md) 第 3 节的 IO 分配表为准，
> 固件请以 `include/app_board_pins.h` 为准。

## 概要
- 芯片：`nRF52840-QFAA`
- 原理图：`Geezpai_Spot-v1.0.pdf` (NRF_UWB_TEST V1.00)
- 用途：快速对照原理图网络名与 nRF GPIO

## Pin Map

| Pin | nRF 管脚名 | 网络名 | 功能分类 | 当前用途 | 方向建议 | 备注 |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | `DEC1` | `DEC1` | 电源 | 去耦 | - | — |
| 2 | `P0.00/XL1` | `P0.00/XL1` | 时钟 | 32.768k 晶振 | - | 接 `Y1` |
| 3 | `P0.01/XL2` | `P0.01/XL2` | 时钟 | 32.768k 晶振 | - | 接 `Y1` |
| 4 | `P0.04/AIN2` | `P0.04/AIN2` | GPIO | 保留 (J6 Pin1) | - | — |
| 5 | `P0.05/AIN3` | `P0.05/AIN3` | GPIO | 保留 (J6 Pin2) | - | — |
| 6 | `P0.07` | `P0.07` | GPIO | 保留 (J6 Pin3) | - | — |
| 7 | `P0.08` | `P0.08` | GPIO | 保留 | - | — |
| 8 | `P1.08` | `P1.08` | GPIO | 保留 (J6 Pin4) | - | — |
| 9 | `P1.09` | `P1.09` | GPIO | 保留 (J6 Pin5) | - | — |
| 10 | `P0.11` | `P0.11` | GPIO | 保留 (J6 Pin6) | - | — |
| 11 | `P0.12` | `P0.12` | GPIO | LCD SPI MISO (loopback) | - | 保留 |
| 12 | `VDD` | `3.3V_nRF` | 电源 | 主电源 | - | 3.3V |
| 13 | `P0.13` | `P0.13` | UART | Console UART TX | Output | 115200bps |
| 14 | `P0.14` | `P0.14` | UART | Console UART RX | Input | 115200bps |
| 15 | `P0.17` | `SDA` | I2C | G-Sensor DA217 SDA | IO | I2C0, 400kHz |
| 16 | `P0.18/RESET` | `NRF_RESET` | 系统 | nRF 复位 (S5 按键) | Input | — |
| 17 | `VDD` | `3.3V_nRF` | 电源 | 主电源 | - | 3.3V |
| 18 | `P0.19` | `SCL` | I2C | G-Sensor DA217 SCL | IO | I2C0, 400kHz |
| 19 | `P0.20` | `SPI_CLK` | SPI | LCD SCK | Output | SPI2, 16MHz |
| 20 | `P0.21` | `MOSI` | SPI | LCD MOSI | Output | SPI2 |
| 21 | `P0.22` | `BEEP` | PWM | 蜂鸣器 | Output | PWM0 |
| 22 | `P0.23` | `UWB_INT` | 中断 | UWB 中断输入 | Input | J7 跳线 |
| 23 | `P0.24` | `NRF_RX` | UART | UWB UART RX | Input | UART1, 921600bps, J4 |
| 24 | `P1.00` | `NRF_TX` | UART | UWB UART TX | Output | UART1, 921600bps, J5 |
| 25 | `VDD` | `3.3V_nRF` | 电源 | 主电源 | - | 3.3V |
| 26 | `SWDIO` | `SWD` | 调试 | BLE SWD 数据 | IO | J6 Pin15 |
| 27 | `SWDCLK` | `SWC` | 调试 | BLE SWD 时钟 | Input | J6 Pin14 |
| 28 | `NC` | `NC` | — | 未连接 | - | — |
| 29 | `P0.09/NFC1` | `P0.09/NFC1` | NFC | NFC 天线 | IO | 焊盘引出 |
| 30 | `P0.10/NFC2` | `P0.10/NFC2` | NFC | NFC 天线 | IO | 焊盘引出 |
| 31 | `ANT` | `ANT1` | RF | 2.4G 射频 | - | 匹配网络 → 天线/IPEX |
| 32 | `VSS_PA` | `GND` | RF | 射频地 | - | — |
| 33 | `DEC6` | `DEC6` | 电源 | 去耦 | - | — |
| 34 | `DEC3` | `DEC3` | 电源 | 去耦 | - | — |
| 35 | `XC1` | `XC1` | 时钟 | 32MHz 晶振 | - | 接 `Y2` |
| 36 | `XC2` | `XC2` | 时钟 | 32MHz 晶振 | - | 接 `Y2` |
| 37 | `VDD` | `3.3V_nRF` | 电源 | 主电源 | - | 3.3V |
| 38 | `P1.15` | `KEY1` | 按键 | 用户按键 1 (S3) | Input | 低有效, 内部上拉 |
| 39 | `P0.03` | `KEY2` | 按键 | 用户按键 2 (S4) | Input | 低有效, 内部上拉 |
| 40 | `P0.02/AIN0` | `LCD_RST` | LCD | LCD 硬件复位 | Output | 低有效 |
| 41 | `P0.28/AIN4` | `RS` | LCD | LCD RS (Data/Command) + 电池 ADC | Output | ⚠️ LF-IO |
| 42 | `P0.29/AIN5` | `CS` | SPI | LCD 片选 | Output | 低有效, ⚠️ LF-IO |
| 43 | `P0.30/AIN6` | `LCD_BL` | LCD | LCD 背光 | Output | ⚠️ LF-IO |
| 44 | `P0.31/AIN7` | `GSENSOR_INT` | 中断 | G-Sensor DA217 INT1 | Input | ⚠️ LF-IO |
| 45 | `VSS` | `GND` | 电源 | 数字地 | - | GND |
| 46 | `DEC4` | `DEC4_6` | 电源 | DC/DC 去耦 | - | 接 LC 网络 |
| 47 | `DCC` | `3.3V_nRF` | 电源 | DC/DC 相关 | - | — |
| 48 | `VDD` | `3.3V_nRF` | 电源 | 主电源 | - | 3.3V |
| 49 | `GND` | `GND` | 电源 | 数字地 | - | GND |

## 按功能归类

### 调试
| 信号 | nRF 管脚 |
| --- | --- |
| `SWDIO` | 26 |
| `SWDCLK` | 27 |

### UART
| 信号 | nRF 管脚 | 外设 | 波特率 |
| --- | --- | --- | --- |
| Console TX | `P0.13` | UART0 | 115200 |
| Console RX | `P0.14` | UART0 | 115200 |
| UWB TX | `P1.00` | UART1 | 921600 |
| UWB RX | `P0.24` | UART1 | 921600 |

### SPI / LCD
| 信号 | nRF 管脚 | 方向 |
| --- | --- | --- |
| `SCK` | `P0.20` | Output |
| `MOSI` | `P0.21` | Output |
| `MISO` | `P0.12` | Input (loopback) |
| `CS` | `P0.29` | Output |
| `RS (DC)` | `P0.28` | Output |
| `RST` | `P0.02` | Output |
| `BL` | `P0.30` | Output |

### I2C (G-Sensor DA217)
| 信号 | nRF 管脚 | 方向 |
| --- | --- | --- |
| `SDA` | `P0.17` | IO |
| `SCL` | `P0.19` | IO |
| `INT1` | `P0.31` | Input |

### 控制
| 信号 | nRF 管脚 | 方向 |
| --- | --- | --- |
| `UWB_INT` | `P0.23` | Input |
| `BEEP` | `P0.22` | Output |

### 按键
| 信号 | nRF 管脚 | 丝印 |
| --- | --- | --- |
| `KEY1` | `P1.15` | S3 |
| `KEY2` | `P0.03` | S4 |
| `NRF_RESET` | `P0.18/RESET` | S5 |
