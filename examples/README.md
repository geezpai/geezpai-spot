# Geezpai Spot SDK 示例

示例按难度分级，每个示例都是独立可构建的 Zephyr 应用。

## Level 1: 基础

无需外设，仅需串口输出即可验证。

| 示例 | 说明 | 硬件依赖 |
|------|------|---------|
| [hello_world](01_basics/hello_world/) | 串口输出 + 系统心跳 | 无 |
| [blinky](01_basics/blinky/) | GPIO 控制 LED 闪烁 | LED (板载/外接) |
| button (TODO) | 按键检测 (轮询 + 中断) | 按键 |

## Level 2: 板载外设

需要 Geezpai Spot 板载硬件。

| 示例 | 说明 | 硬件依赖 |
|------|------|---------|
| lcd_display (TODO) | TFT 彩屏显示文字/图形 | ST7735R LCD |
| gsensor (TODO) | G-Sensor 加速度采集 | DA217 |
| buzzer_music (TODO) | 蜂鸣器播放旋律 | 蜂鸣器 |
| battery_monitor (TODO) | 电池电压检测 | 电池 |

## Level 3: BLE 无线

| 示例 | 说明 | 硬件依赖 |
|------|------|---------|
| beacon (TODO) | BLE Beacon 广播 | BLE |
| peripheral (TODO) | BLE 从机 + 自定义 Service | BLE |
| central (TODO) | BLE 主机扫描 | BLE |

## Level 4: UWB 测距

| 示例 | 说明 | 硬件依赖 |
|------|------|---------|
| [device_info](04_uwb/device_info/) (TODO) | 读取 UWB 模组信息 | MQ-MMK00X |
| two_way_ranging (TODO) | TWR 双向测距 | 2 块 Geezpai Spot |

## Level 5: Find My 定位

| 示例 | 说明 | 硬件依赖 |
|------|------|---------|
| **[locator_tag](05_findmy/locator_tag/)** | ✅ FHN 定位标签 — **主力固件** | Geezpai Spot 完整 |
| uwb_tag (TODO) | FHN + UWB 联合定位 | Geezpai Spot 完整 |

## Level 6: 完整应用

| 示例 | 说明 | 硬件依赖 |
|------|------|---------|
| spot_demo (TODO) | 全功能演示 | Geezpai Spot 完整 |
| uwb_positioning (TODO) | UWB 室内定位系统 | 3+ 块 Geezpai Spot |

---

## 如何构建示例

```bash
# 1. 确保已安装 nRF Connect SDK v3.3.0
# 2. 进入示例目录
cd examples/01_basics/blinky

# 3. 构建
west build -b nrf52840dk/nrf52840 -d build

# 4. 烧录
west flash -d build
```

### 已验证构建结果 (nRF52840 DK)

| 示例 | FLASH | RAM | 状态 |
|------|-------|-----|------|
| [hello_world](01_basics/hello_world/) | 30 KB | 8.5 KB | ✅ |
| [blinky](01_basics/blinky/) | 33 KB | 8.7 KB | ✅ |
| [locator_tag](05_findmy/locator_tag/) | 311 KB | 61 KB | ✅ |

> 每个示例目录都有独立的 `README.md` 说明。
> 主力固件 (locator_tag) 使用 sysbuild 多镜像构建，最小示例 (hello_world/blinky) 仅需单镜像。
