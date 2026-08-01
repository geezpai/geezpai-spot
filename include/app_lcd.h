/*
 * Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_LCD_H__
#define APP_LCD_H__

#include <stdbool.h>
#include <stdint.h>

#include "app_ui.h"

int app_lcd_init(void);
int app_lcd_backlight_set(bool on);
int app_lcd_fill_screen(uint16_t color);
int app_lcd_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
int app_lcd_show_test_pattern(void);
int app_lcd_run_test_sequence(void);
int app_lcd_state_change_indicate(enum app_ui_state state, bool active);
int app_lcd_battery_level_set(uint8_t level);
int app_lcd_motion_event_indicate(void);

#endif
