# Google Nearby — Fast Pair Locator Tag (完整例程)

## 概述

这是 Geezpai Spot 的**主力固件**，实现完整的 **Google Find Hub Network (FHN) 定位标签**。
固件基于 Nordic nRF Connect SDK 的 [Fast Pair Locator Tag sample](https://github.com/nrfconnect/sdk-nrf)，
针对 Geezpai Spot 硬件深度定制。

**一句话**: 让你的 Geezpai Spot 变成一个能被 Android 手机通过 Find Hub app 远程查找的智能防丢标签。

---

## 功能矩阵

| 功能模块 | 说明 | Kconfig | 状态 |
|---------|------|---------|------|
| **Fast Pair 配网** | 靠近 Android 手机自动弹窗配对 | `CONFIG_BT_FAST_PAIR` | ✅ |
| **FHN 定位** | 通过众包网络定位标签位置 | `CONFIG_BT_FAST_PAIR_FHN` | ✅ |
| **UWB 通信** | UCI 协议栈，查询设备信息 | `CONFIG_APP_UWB` | ✅ 基础 |
| **UWB 测距** | 精密测距 (Precision Finding) | `CONFIG_APP_RANGING` | 🚧 开发中 |
| **TFT 仪表盘** | 80×160 彩屏，显示状态/电池/距离 | `CONFIG_APP_UI` | ✅ |
| **G-Sensor 防跟踪** | DULT 运动检测 + 未知标签告警 | `CONFIG_BT_FAST_PAIR_FHN_DULT_MOTION_DETECTOR` | ✅ |
| **蜂鸣器响铃** | Find My 查找响铃 (3 组件) | — | ✅ |
| **电池监测** | ADC 电压采集 + 电量百分比 | `CONFIG_APP_BATTERY` | ✅ |
| **DFU OTA 升级** | MCUboot + SMP BLE 无线升级 | `CONFIG_APP_DFU` | ✅ |
| **工厂复位** | 按键触发 + FHN 自动复位 | — | ✅ |

---

## 工作流程

```
┌──────────────────────────────────────────────────────────┐
│                    上电启动                               │
│  bootup_buzzer_test_play() → 蜂鸣器开机旋律               │
│  app_lcd_init() → TFT 屏初始化                           │
│  app_uwb_init() → UWB 模组复位+信息查询                   │
│  bt_enable() → BLE 协议栈启动                            │
│  settings_load() → 加载持久化配置                         │
│  fast_pair_prepare() → Fast Pair 就绪                    │
│  fhn_prepare() → FHN 就绪                                │
└─────────────┬────────────────────────────────────────────┘
              │
              ▼
┌──────────────────────────────────────────────────────────┐
│  状态 A: 未配网 (Unprovisioned)                           │
│                                                          │
│  • 广播 Fast Pair discoverable 包                        │
│  • Android 手机靠近 → 半屏弹窗 → 一键配对                 │
│  • Account Key 写入 → 自动进入 FHN Provisioning          │
│  • 5 分钟内未完成 → 自动恢复出厂 → 重新广播               │
└─────────────┬────────────────────────────────────────────┘
              │ FHN Provisioning 成功
              ▼
┌──────────────────────────────────────────────────────────┐
│  状态 B: 已配网 (Provisioned)                             │
│                                                          │
│  • 广播 FHN payload (众包定位)                            │
│  • 附近 Android 设备匿名上报位置给 Find Hub 网络           │
│  • 用户在 Find Hub app 查看标签位置                        │
│  • 可远程触发 Ring (蜂鸣器响铃)                            │
│  • 支持 BLE CS/UWB 精密测距 (Precision Finding)           │
│                                                          │
│  辅助模式:                                                │
│  • Recovery Mode — 按键长按 3s 进入，允许新手机接管       │
│  • Identification Mode — 临时开放识别信息读取 (DULT)      │
└──────────────────────────────────────────────────────────┘
```

---

## 构建与烧录

### 方式 A: 从根目录构建 (推荐)

```bash
cd geezpai-spot
west build -b nrf52840dk_nrf52840 -d build
west flash -d build
```

### 方式 B: 从示例目录构建

```bash
cd geezpai-spot/examples/05_findmy/locator_tag
west build -b nrf52840dk_nrf52840 -d build
west flash -d build
```

### 烧录 (J-Link)

```bash
# 多镜像 sysbuild 烧录 (mcuboot + app + provisioning_data)
west flash -d build

# 或使用脚本
./download_jlink.bat
```

### 串口日志

```
波特率: 115200 8N1
TX: P0.13
RX: P0.14
```

---

## 硬件要求

| 组件 | 型号 | 连接 | 必需? |
|------|------|------|-------|
| 主控板 | Geezpai_Spot-v1.0 | — | ✅ 必需 |
| UWB 模组 | MQ-MMK00X | UART1 + INT | 可选 (CONFIG_APP_UWB) |
| TFT 屏 | 0.96" ST7735R | SPI2 | 可选 |
| G-Sensor | DA217 | I2C0 | 可选 (DULT) |
| J-Link | — | SWD (J6 Pin14-15) | ✅ 烧录用 |

> 非 Geezpai Spot 板（nRF52840 DK / Thingy:53 等）也能构建但不支持 UWB 和 LCD。

---

## 配置说明

### 敏感配置 (必须)

```bash
# 1. 从模板创建配置
cp Kconfig.sysbuild.template Kconfig.sysbuild

# 2. 编辑 Kconfig.sysbuild 填入真实值:
#    - CONFIG_BT_FAST_PAIR_MODEL_ID = 你的 Model ID (0xXXXXXX)
#    - CONFIG_BT_FAST_PAIR_ANTI_SPOOFING_PRIVATE_KEY = 你的密钥
```

> ⚠️ `Kconfig.sysbuild` 已加入 `.gitignore`，不会被提交到 Git。

### 功能开关

```bash
# 在 configuration/prj.conf 或通过 menuconfig 切换:

# UWB 模组
CONFIG_APP_UWB=y

# DFU 无线升级
CONFIG_APP_DFU=y

# 运动检测 (防跟踪)
CONFIG_BT_FAST_PAIR_FHN_DULT_MOTION_DETECTOR=y

# Debug 日志
CONFIG_LOG=y
CONFIG_LOG_DEFAULT_LEVEL=4
```

---

## 关键源文件

| 文件 | 行数 | 说明 |
|------|------|------|
| [src/main.c](../../src/main.c) | ~790 | **主入口**：init 流程、FHN 状态机、Fast Pair 回调 |
| [src/lcd.c](../../src/lcd.c) | ~700 | ST7735R 驱动 + 仪表盘渲染 |
| [src/uwb/app_uwb.c](../../src/uwb/app_uwb.c) | ~250 | UWB 初始化 + 事件回调 + 事件线程 |
| [src/uwb/uci_port.c](../../src/uwb/uci_port.c) | ~195 | UCI UART 传输层 (921600bps) |
| [src/ranging.c](../../src/ranging.c) | ~1167 | **Precision Finding**: BLE CS 测距状态机 |
| [src/buzzer.c](../../src/buzzer.c) | ~150 | PWM 蜂鸣器 (音符序列) |
| [src/ring.c](../../src/ring.c) | — | Find My 响铃动作 |
| [src/motion_detector/platform_da217.c](../../src/motion_detector/platform_da217.c) | — | DA217 G-Sensor 驱动 |
| [src/factory_reset.c](../../src/factory_reset.c) | — | 恢复出厂 + 回调框架 |

---

## UWB 集成状态

```
初始化 ✅
  ├── DEVICE_RESET      ✅ 固件已实现
  ├── GET_DEVICE_INFO   ✅ 固件已实现 (日志输出 UCI/MAC/PHY 版本)
  └── GET_CAPS_INFO     ✅ 固件已实现 (日志输出能力集)

测距 🚧
  ├── SESSION_INIT          供应商 API 可用，应用层未调用
  ├── SET_APP_CONFIG        供应商 API 可用 (40+ TLV packers)
  ├── RANGE_START           供应商 API 可用
  ├── RANGE_DATA 回调       供应商 API 可用
  └── RANGE_STOP            供应商 API 可用

Precision Finding ⚠️
  └── BLE CS (ranging.c)   仅 nRF54 系列支持，nRF52840 无硬件 CS
                           测距方案需以 UWB 为主
```

---

## 多板支持

| 板子目标 | FHN | UWB | BLE CS | LCD |
|---------|-----|-----|--------|-----|
| `nrf52840dk_nrf52840` | ✅ | ✅ | ❌ | ✅ |
| `nrf52833dk_nrf52833` | ✅ | ❌ | ❌ | ❌ |
| `nrf52dk_nrf52832` | ✅ | ❌ | ❌ | ❌ |
| `nrf5340dk_nrf5340_cpuapp` | ✅ | ❌ | ❌ | ❌ |
| `thingy53_nrf5340_cpuapp` | ✅ | ❌ | ❌ | ❌ |
| `nrf54l15dk_nrf54l15_cpuapp` | ✅ | ❌ | ✅ | ❌ |
| `nrf54l15tag_nrf54l15_cpuapp` | ✅ | ❌ | ✅ | ❌ |

> UWB 和 LCD 依赖 Geezpai Spot 自定义 devicetree overlay (`configuration/boards/nrf52840dk_nrf52840.overlay`)。

---

## 开发提示

### 新增功能模块

参考开发指南 (内部文档) 中的 "新增功能模块" 章节。模式为：

1. 新建 `Kconfig.<module>` → 在 `Kconfig` 中 `rsource`
2. 新建 `include/app_<module>.h` 声明 API
3. 新建 `src/<module>/` 实现 + CMakeLists.txt
4. 在 `CMakeLists.txt` 的 `# NORDIC SDK APP END` 前注册
5. 在 `src/main.c` 的 `init_work_handle()` 中调用 init

### 调试技巧

- **Debug 构建**: 使用 `configuration/prj.conf` (默认)
- **Release 构建**: 使用 `configuration/prj_release.conf` (关闭串口日志，减小固件体积)
- **UWB 调试**: 可断开 J4/J5 跳线抓 UART 波形
- **DA217 中断**: 已知 bug，见 [debug-da217-motion-interrupt.md](../../debug-da217-motion-interrupt.md)

---

> 🔗 **参考链接**
> - [Fast Pair Find Hub Network extension](https://developers.google.com/nearby/fast-pair/fhn)
> - [nRF Connect SDK 文档](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/)
> - [Geezpai Spot 硬件说明](../../doc/Geezpai_Spot_开发板说明.md)
