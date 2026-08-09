/*
 * Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 *
 * Geezpai Spot SDK Example: Blinky
 *
 * GPIO 控制 LED 闪烁 — 嵌入式世界的 "Hello World"。
 *
 * 使用 nRF52840 DK 的 led0 (P1.10, 低电平点亮)。
 * 如果你在 Geezpai Spot 板上运行，可外接 LED 到 J6 扩展排针的 GPIO。
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(blinky, LOG_LEVEL_INF);

/* 使用 devicetree 别名 led0，由板级 DTS 定义 */
#define LED_NODE DT_ALIAS(led0)

#if DT_NODE_EXISTS(LED_NODE)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
#else
#error "Missing led0 alias in devicetree"
#endif

int main(void)
{
	int err;

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED GPIO is not ready");
		return -ENODEV;
	}

	err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (err) {
		LOG_ERR("LED GPIO config failed (err %d)", err);
		return err;
	}

	LOG_INF("Blinky started — LED will toggle every second");

	while (1) {
		err = gpio_pin_toggle_dt(&led);
		if (err) {
			LOG_ERR("LED toggle failed (err %d)", err);
		}
		k_msleep(1000);
	}

	return 0;
}
