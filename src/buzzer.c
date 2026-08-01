/*
 * Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(fp_fhn, LOG_LEVEL_INF);

#include "app_buzzer.h"

#define APP_BUZZER_NODE DT_ALIAS(pwm_spk)

BUILD_ASSERT(DT_NODE_EXISTS(APP_BUZZER_NODE), "Missing pwm-spk devicetree alias");

static const struct pwm_dt_spec buzzer_pwm = PWM_DT_SPEC_GET(APP_BUZZER_NODE);
static bool buzzer_initialized;

static int buzzer_apply(uint32_t frequency_hz, uint8_t duty_percent)
{
	uint32_t period_ns;
	uint32_t pulse_ns;

	if (frequency_hz == 0U || duty_percent == 0U) {
		return pwm_set_dt(&buzzer_pwm, PWM_MSEC(20), 0U);
	}

	if (duty_percent > 100U) {
		return -EINVAL;
	}

	period_ns = PWM_HZ(frequency_hz);
	pulse_ns = (uint32_t)(((uint64_t)period_ns * duty_percent) / 100U);

	return pwm_set_dt(&buzzer_pwm, period_ns, pulse_ns);
}

int app_buzzer_init(void)
{
	int err;

	if (!device_is_ready(buzzer_pwm.dev)) {
		LOG_ERR("Buzzer PWM device is not ready");
		return -ENODEV;
	}

	err = buzzer_apply(0U, 0U);
	if (err) {
		LOG_ERR("Failed to initialize buzzer PWM: %d", err);
		return err;
	}

	buzzer_initialized = true;
	return 0;
}

int app_buzzer_tone_set(uint32_t frequency_hz, uint8_t duty_percent)
{
	if (!buzzer_initialized) {
		return -EACCES;
	}

	return buzzer_apply(frequency_hz, duty_percent);
}

int app_buzzer_stop(void)
{
	return app_buzzer_tone_set(0U, 0U);
}

int app_buzzer_play_sequence(const struct app_buzzer_note *notes, size_t count, bool stop_after)
{
	int err;

	if (!buzzer_initialized) {
		return -EACCES;
	}

	if (!notes && count) {
		return -EINVAL;
	}

	for (size_t i = 0; i < count; i++) {
		err = buzzer_apply(notes[i].frequency_hz, notes[i].duty_percent);
		if (err) {
			LOG_ERR("Failed to play buzzer note %u: %d", (uint32_t)i, err);
			if (stop_after) {
				(void)app_buzzer_stop();
			}
			return err;
		}

		if (notes[i].duration_ms) {
			k_msleep(notes[i].duration_ms);
		}
	}

	if (stop_after) {
		err = app_buzzer_stop();
		if (err) {
			LOG_ERR("Failed to stop buzzer after sequence: %d", err);
			return err;
		}
	}

	return 0;
}
