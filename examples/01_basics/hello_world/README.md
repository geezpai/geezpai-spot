# Hello World

## 功能

最简固件示例，验证工具链、构建、烧录、串口输出是否正常。

## 硬件要求

- Geezpai Spot 开发板 (或 nRF52840 DK)
- USB Type-C 供电
- 串口工具连接 Console UART (P0.13 TX / P0.14 RX, 115200 8N1)

## 构建

```bash
cd geezpai-spot/examples/01_basics/hello_world
west build -b nrf52840dk_nrf52840 -d build
```

## 烧录

```bash
west flash -d build
```

## 预期输出

串口 115200 8N1 应输出：

```
[00:00:00.000,000] <inf> hello_world: === Geezpai Spot SDK ===
[00:00:00.000,000] <inf> hello_world: Hello World! The board is alive.
[00:00:05.000,000] <inf> hello_world: Uptime: 5000 ms
[00:00:10.000,000] <inf> hello_world: Uptime: 10000 ms
...
```

## 下一步

- [blinky](../blinky/) — GPIO 控制 LED 闪烁
