# Blinky — LED 闪烁

## 功能

控制板载 LED (led0) 每秒闪烁一次。嵌入式开发的标准 "Hello World"。

## 硬件要求

- **nRF52840 DK**：使用板载 led0 (P1.10)，无需额外硬件
- **Geezpai Spot**：板子无板载 LED，需外接 LED 到 J6 扩展排针：
  - LED 正极 → J6 保留 GPIO (如 P0.07)
  - LED 负极 → 串 220Ω 电阻 → GND
  - 修改 `src/main.c` 中的 GPIO 引脚定义

## 构建

```bash
cd geezpai-spot/examples/01_basics/blinky
west build -b nrf52840dk_nrf52840 -d build
```

## 烧录

```bash
west flash -d build
```

## 预期效果

- LED 每秒亮灭一次
- 串口 115200 8N1 输出 "Blinky started"

## GPIO 速查

| 信号 | GPIO | 说明 |
|------|------|------|
| led0 | P1.10 | nRF52840 DK 板载 LED (低电平点亮) |
| KEY1 | P1.15 | 用户按键 (低有效) |
| J6 Pin1 | P0.04 | 保留 GPIO，可外接 LED |
| J6 Pin3 | P0.07 | 保留 GPIO，可外接 LED |

## 下一步

- [button](../button/) — 按键检测与控制
