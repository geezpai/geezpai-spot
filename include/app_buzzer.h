/*
 * Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_BUZZER_H__
#define APP_BUZZER_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct app_buzzer_note {
	uint32_t frequency_hz;
	uint32_t duration_ms;
	uint8_t duty_percent;
};

int app_buzzer_init(void);
int app_buzzer_tone_set(uint32_t frequency_hz, uint8_t duty_percent);
int app_buzzer_stop(void);
int app_buzzer_play_sequence(const struct app_buzzer_note *notes, size_t count, bool stop_after);

#endif
