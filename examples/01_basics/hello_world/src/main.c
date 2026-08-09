/*
 * Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 *
 * Geezpai Spot SDK Example: Hello World
 *
 * 最简示例 — 验证工具链和串口输出。
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(hello_world, LOG_LEVEL_INF);

int main(void)
{
	LOG_INF("=== Geezpai Spot SDK ===");
	LOG_INF("Hello World! The board is alive.");

	while (1) {
		LOG_INF("Uptime: %u ms", k_uptime_get_32());
		k_msleep(5000);
	}

	return 0;
}
