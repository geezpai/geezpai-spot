/*
 * Copyright (c) 2024-2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "app_ui_speaker_priv.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(fp_fhn, LOG_LEVEL_DBG);

int app_ui_speaker_init(void)
{
	return app_buzzer_init();
}

int app_ui_speaker_on(void)
{
	return app_buzzer_tone_set(CONFIG_APP_UI_SPEAKER_FREQ, 50U);
}

int app_ui_speaker_off(void)
{
	return app_buzzer_stop();
}

int app_ui_speaker_set_tone(uint32_t frequency_hz, uint8_t duty_percent)
{
	return app_buzzer_tone_set(frequency_hz, duty_percent);
}

int app_ui_speaker_play_sequence(const struct app_buzzer_note *notes, uint8_t count)
{
	return app_buzzer_play_sequence(notes, count, true);
}
