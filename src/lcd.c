/*
 * Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#include <string.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(fp_fhn, LOG_LEVEL_INF);

#include "app_battery.h"
#include "app_board_pins.h"
#include "app_lcd.h"

#define LCD_MAX_RECT_WIDTH      160U
#define LCD_FILL_CHUNK_LINES    16U
#define LCD_FILL_CHUNK_PIXELS   (LCD_MAX_RECT_WIDTH * LCD_FILL_CHUNK_LINES)

#define LCD_FONT_WIDTH          5U
#define LCD_FONT_HEIGHT         7U
#define LCD_FONT_SMALL_SCALE    1U
#define LCD_FONT_LARGE_SCALE    2U

#define LCD_COLOR_BG            0x0000
#define LCD_COLOR_HEADER        0x001F
#define LCD_COLOR_CARD_OK       0x03E0
#define LCD_COLOR_CARD_INFO     0x0017
#define LCD_COLOR_CARD_ALERT    0xF800
#define LCD_COLOR_TEXT          0xFFFF
#define LCD_COLOR_MUTED         0x39E7
#define LCD_COLOR_CYAN          0x07FF
#define LCD_COLOR_PANEL         0x0841
#define LCD_COLOR_PANEL_INNER   0x0000
#define LCD_MOTION_EVENT_HOLD_MS 800

#define LCD_HEADER_HEIGHT       18U
#define LCD_BATTERY_X           60U
#define LCD_BATTERY_Y           5U
#define LCD_MAIN_CARD_X         6U
#define LCD_MAIN_CARD_Y         24U
#define LCD_MAIN_CARD_HEIGHT    44U
#define LCD_STATUS_TITLE_Y      74U
#define LCD_TILE_WIDTH          34U
#define LCD_TILE_HEIGHT         22U

#define LCD_DIRTY_STATIC        BIT(0)
#define LCD_DIRTY_BATTERY       BIT(1)
#define LCD_DIRTY_MAIN_CARD     BIT(2)
#define LCD_DIRTY_TILE_PROV     BIT(3)
#define LCD_DIRTY_TILE_ADV      BIT(4)
#define LCD_DIRTY_TILE_RING     BIT(5)
#define LCD_DIRTY_TILE_MOVE     BIT(6)
#define LCD_DIRTY_TILE_ID       BIT(7)
#define LCD_DIRTY_TILE_DFU      BIT(8)
#define LCD_DIRTY_ALL_DYNAMIC   (LCD_DIRTY_BATTERY | LCD_DIRTY_MAIN_CARD | LCD_DIRTY_TILE_PROV | \
                                 LCD_DIRTY_TILE_ADV | LCD_DIRTY_TILE_RING | LCD_DIRTY_TILE_MOVE | \
                                 LCD_DIRTY_TILE_ID | LCD_DIRTY_TILE_DFU)
#define LCD_DIRTY_ALL           (LCD_DIRTY_STATIC | LCD_DIRTY_ALL_DYNAMIC)

struct lcd_font_glyph {
        char c;
        uint8_t rows[LCD_FONT_HEIGHT];
};

static const struct lcd_font_glyph lcd_font[] = {
        {' ', {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
        {'A', {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
        {'B', {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}},
        {'C', {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}},
        {'D', {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}},
        {'E', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}},
        {'F', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}},
        {'G', {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}},
        {'H', {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
        {'I', {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F}},
        {'J', {0x1F, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0C}},
        {'K', {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}},
        {'L', {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}},
        {'M', {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}},
        {'N', {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}},
        {'O', {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
        {'P', {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}},
        {'Q', {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}},
        {'R', {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}},
        {'S', {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}},
        {'T', {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
        {'U', {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
        {'V', {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}},
        {'W', {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}},
        {'X', {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}},
        {'Y', {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}},
        {'Z', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}},
};

static const struct device *lcd_gpio;
static const struct device *lcd_display;
static struct display_capabilities lcd_caps;
static bool lcd_ready;
static atomic_t lcd_ui_state[ATOMIC_BITMAP_SIZE(APP_UI_STATE_COUNT)];
static atomic_t lcd_dirty_flags;
static struct k_work_delayable lcd_refresh_work;
static struct k_work_delayable lcd_motion_event_clear_work;
static uint8_t lcd_battery_level = 100U;
static bool lcd_motion_event_active;

static int lcd_dashboard_render(atomic_val_t dirty_flags);
static void lcd_motion_event_clear_work_handler(struct k_work *work);

static int lcd_backlight_init(void)
{
        lcd_gpio = DEVICE_DT_GET(DT_NODELABEL(gpio0));
        if (!device_is_ready(lcd_gpio)) {
                return -ENODEV;
        }

        return gpio_pin_configure(lcd_gpio, APP_LCD_BL_PIN, GPIO_OUTPUT_HIGH);
}

int app_lcd_backlight_set(bool on)
{
        if (!lcd_gpio) {
                return -ENODEV;
        }

        return gpio_pin_set(lcd_gpio, APP_LCD_BL_PIN, on ? 0 : 1);
}

static int lcd_write_solid_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                                uint16_t color)
{
        struct display_buffer_descriptor desc;
        static uint16_t chunk[LCD_FILL_CHUNK_PIXELS];
        const uint16_t pixel = sys_cpu_to_be16(color);

        if (!lcd_ready) {
                return -ENODEV;
        }

        if ((width == 0U) || (height == 0U) || (width > LCD_MAX_RECT_WIDTH) ||
            ((uint32_t)width * height > LCD_FILL_CHUNK_PIXELS)) {
                return -EINVAL;
        }

        for (uint32_t i = 0; i < (uint32_t)width * height; i++) {
                chunk[i] = pixel;
        }

        desc.width = width;
        desc.height = height;
        desc.pitch = width;
        desc.buf_size = width * height * sizeof(uint16_t);

        return display_write(lcd_display, x, y, &desc, chunk);
}

int app_lcd_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
        int err;

        if (!lcd_ready) {
                return -ENODEV;
        }

        if ((x >= lcd_caps.x_resolution) || (y >= lcd_caps.y_resolution) ||
            (width == 0U) || (height == 0U)) {
                return -EINVAL;
        }

        if ((x + width) > lcd_caps.x_resolution) {
                width = lcd_caps.x_resolution - x;
        }

        if ((y + height) > lcd_caps.y_resolution) {
                height = lcd_caps.y_resolution - y;
        }

        for (uint16_t row = 0; row < height; row += LCD_FILL_CHUNK_LINES) {
                uint16_t chunk_height = MIN((uint16_t)LCD_FILL_CHUNK_LINES,
                                            (uint16_t)(height - row));

                err = lcd_write_solid_rect(x, y + row, width, chunk_height, color);
                if (err) {
                        return err;
                }
        }

        return 0;
}

int app_lcd_fill_screen(uint16_t color)
{
        if (!lcd_ready) {
                return -ENODEV;
        }

        return app_lcd_fill_rect(0U, 0U, lcd_caps.x_resolution, lcd_caps.y_resolution, color);
}

static const uint8_t *lcd_font_glyph_find(char c)
{
        for (size_t i = 0; i < ARRAY_SIZE(lcd_font); i++) {
                if (lcd_font[i].c == c) {
                        return lcd_font[i].rows;
                }
        }

        return lcd_font[0].rows;
}

static uint16_t lcd_text_width(const char *text, uint8_t scale)
{
        size_t len = strlen(text);

        if (len == 0U) {
                return 0U;
        }

        return (uint16_t)((len * LCD_FONT_WIDTH + (len - 1U)) * scale);
}

static int lcd_draw_char(int16_t x, int16_t y, char c, uint8_t scale, uint16_t color)
{
        const uint8_t *rows = lcd_font_glyph_find(c);
        int err;

        for (uint8_t row = 0; row < LCD_FONT_HEIGHT; row++) {
                int8_t run_start = -1;
                uint8_t bits = rows[row];

                for (uint8_t col = 0; col <= LCD_FONT_WIDTH; col++) {
                        bool pixel_on = (col < LCD_FONT_WIDTH) &&
                                        ((bits & BIT(LCD_FONT_WIDTH - 1U - col)) != 0U);

                        if (pixel_on && (run_start < 0)) {
                                run_start = (int8_t)col;
                        } else if (!pixel_on && (run_start >= 0)) {
                                uint16_t run_width = (uint16_t)(col - (uint8_t)run_start) * scale;

                                err = app_lcd_fill_rect((uint16_t)(x + run_start * scale),
                                                        (uint16_t)(y + row * scale),
                                                        run_width, scale, color);
                                if (err) {
                                        return err;
                                }

                                run_start = -1;
                        }
                }
        }

        return 0;
}

static int lcd_draw_text(int16_t x, int16_t y, const char *text, uint8_t scale, uint16_t color)
{
        int err;
        int16_t cursor_x = x;

        for (size_t i = 0; text[i] != '\0'; i++) {
                err = lcd_draw_char(cursor_x, y, text[i], scale, color);
                if (err) {
                        return err;
                }

                cursor_x += (LCD_FONT_WIDTH + 1) * scale;
        }

        return 0;
}

static int lcd_draw_text_centered(int16_t y, const char *text, uint8_t scale, uint16_t color)
{
        uint16_t text_width = lcd_text_width(text, scale);
        int16_t x = (int16_t)((lcd_caps.x_resolution - text_width) / 2U);

        if (x < 0) {
                x = 0;
        }

        return lcd_draw_text(x, y, text, scale, color);
}

static int lcd_draw_status_text(int16_t y, const char *line1, const char *line2,
                                uint16_t color)
{
        int err = lcd_draw_text_centered(y, line1, LCD_FONT_LARGE_SCALE, color);

        if (err) {
                return err;
        }

        return lcd_draw_text_centered(y + (int16_t)(LCD_FONT_HEIGHT * LCD_FONT_LARGE_SCALE) + 6,
                                      line2, LCD_FONT_SMALL_SCALE, color);
}

static int lcd_panel_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                          uint16_t border_color, uint16_t fill_color)
{
        int err;

        err = app_lcd_fill_rect(x, y, width, height, border_color);
        if (err) {
                return err;
        }

        if ((width <= 2U) || (height <= 2U)) {
                return 0;
        }

        return app_lcd_fill_rect(x + 1U, y + 1U, width - 2U, height - 2U, fill_color);
}

static int lcd_battery_draw(uint16_t x, uint16_t y)
{
        int err;
        uint8_t bars = 0U;
        uint16_t color = LCD_COLOR_MUTED;

        if (lcd_battery_level != 0xFFU) {
                if (lcd_battery_level > 66U) {
                        bars = 3U;
                        color = LCD_COLOR_CARD_OK;
                } else if (lcd_battery_level > 25U) {
                        bars = 2U;
                        color = LCD_COLOR_CARD_INFO;
                } else if (lcd_battery_level > 10U) {
                        bars = 1U;
                        color = 0xFD20;
                } else {
                        bars = 1U;
                        color = LCD_COLOR_CARD_ALERT;
                }
        }

        err = lcd_panel_draw(x, y, 14U, 8U, LCD_COLOR_TEXT, LCD_COLOR_PANEL_INNER);
        if (err) {
                return err;
        }

        err = app_lcd_fill_rect(x + 14U, y + 2U, 2U, 4U, LCD_COLOR_TEXT);
        if (err) {
                return err;
        }

        for (uint8_t i = 0U; i < bars; i++) {
                err = app_lcd_fill_rect(x + 2U + i * 4U, y + 2U, 3U, 4U, color);
                if (err) {
                        return err;
                }
        }

        return 0;
}

static bool lcd_state_is_active(enum app_ui_state state)
{
        return atomic_test_bit(lcd_ui_state, state);
}

static void lcd_refresh_schedule(atomic_val_t dirty_flags)
{
        (void)atomic_or(&lcd_dirty_flags, dirty_flags);

        if (lcd_ready) {
                (void)k_work_reschedule(&lcd_refresh_work, K_NO_WAIT);
        }
}

static const char *lcd_main_status_text_get(void)
{
        if (lcd_state_is_active(APP_UI_STATE_DFU_MODE)) {
                return "DFU";
        }

        if (lcd_state_is_active(APP_UI_STATE_RINGING)) {
                return "RING";
        }

        if (lcd_state_is_active(APP_UI_STATE_RECOVERY_MODE)) {
                return "RCVR";
        }

        if (lcd_state_is_active(APP_UI_STATE_ID_MODE)) {
                return "ID";
        }

        if (!lcd_state_is_active(APP_UI_STATE_APP_RUNNING)) {
                return "BOOT";
        }

        if (!lcd_state_is_active(APP_UI_STATE_PROVISIONED) &&
            lcd_state_is_active(APP_UI_STATE_FP_ADV)) {
                return "PAIR";
        }

        if (lcd_state_is_active(APP_UI_STATE_PROVISIONED)) {
                return "READY";
        }

        return "WAIT";
}

static const char *lcd_main_status_subtext_get(void)
{
        if (lcd_state_is_active(APP_UI_STATE_DFU_MODE)) {
                return "OTA UPDATE";
        }

        if (lcd_state_is_active(APP_UI_STATE_RINGING)) {
                return "LOCATE REQ";
        }

        if (lcd_state_is_active(APP_UI_STATE_RECOVERY_MODE)) {
                return "RECOVERY";
        }

        if (lcd_state_is_active(APP_UI_STATE_ID_MODE)) {
                return "IDENTIFY";
        }

        if (!lcd_state_is_active(APP_UI_STATE_APP_RUNNING)) {
                return "STARTING";
        }

        if (!lcd_state_is_active(APP_UI_STATE_PROVISIONED) &&
            lcd_state_is_active(APP_UI_STATE_FP_ADV)) {
                return "READY LINK";
        }

        if (lcd_state_is_active(APP_UI_STATE_PROVISIONED)) {
                return "TRACK ONLINE";
        }

        return "PHONE APP";
}

static uint16_t lcd_main_status_color_get(void)
{
        if (lcd_state_is_active(APP_UI_STATE_DFU_MODE)) {
                return LCD_COLOR_CARD_INFO;
        }

        if (lcd_state_is_active(APP_UI_STATE_RINGING)) {
                return LCD_COLOR_CARD_ALERT;
        }

        if (lcd_state_is_active(APP_UI_STATE_RECOVERY_MODE) ||
            lcd_state_is_active(APP_UI_STATE_ID_MODE) ||
            lcd_state_is_active(APP_UI_STATE_FP_ADV)) {
                return LCD_COLOR_CARD_INFO;
        }

        return LCD_COLOR_CARD_OK;
}

static bool lcd_motion_tile_is_active(void)
{
        return lcd_motion_event_active;
}

static int lcd_status_tile_draw(uint16_t x, uint16_t y, const char *label, bool active,
                                uint16_t accent_color)
{
        int err;
        uint16_t label_width = lcd_text_width(label, LCD_FONT_SMALL_SCALE);
        uint16_t state_width = lcd_text_width(active ? "ON" : "OFF", LCD_FONT_SMALL_SCALE);
        int16_t label_x = (int16_t)(x + ((34U - label_width) / 2U));
        int16_t state_x = (int16_t)(x + ((34U - state_width) / 2U));

        err = lcd_panel_draw(x, y, 34U, 22U, active ? accent_color : LCD_COLOR_MUTED,
                             LCD_COLOR_PANEL_INNER);
        if (err) {
                return err;
        }

        err = app_lcd_fill_rect(x + 1U, y + 1U, 32U, 4U, active ? accent_color : LCD_COLOR_MUTED);
        if (err) {
                return err;
        }

        err = lcd_draw_text(label_x, (int16_t)(y + 7U), label, LCD_FONT_SMALL_SCALE,
                            LCD_COLOR_TEXT);
        if (err) {
                return err;
        }

        return lcd_draw_text(state_x, (int16_t)(y + 15U), active ? "ON" : "OFF",
                             LCD_FONT_SMALL_SCALE, LCD_COLOR_TEXT);
}

static int lcd_static_layout_draw(void)
{
        int err;

        err = app_lcd_fill_screen(LCD_COLOR_BG);
        if (err) {
                return err;
        }

        err = app_lcd_fill_rect(0U, 0U, lcd_caps.x_resolution, LCD_HEADER_HEIGHT, LCD_COLOR_HEADER);
        if (err) {
                return err;
        }

        err = lcd_draw_text(4, 6, "NEARBY", LCD_FONT_SMALL_SCALE, LCD_COLOR_TEXT);
        if (err) {
                return err;
        }

        err = lcd_draw_text_centered(LCD_STATUS_TITLE_Y, "STATUS", LCD_FONT_SMALL_SCALE,
                                     LCD_COLOR_TEXT);
        if (err) {
                return err;
        }

        return 0;
}

static int lcd_main_card_draw(void)
{
        int err;

        err = lcd_panel_draw(LCD_MAIN_CARD_X, LCD_MAIN_CARD_Y, lcd_caps.x_resolution - 12U,
                             LCD_MAIN_CARD_HEIGHT, lcd_main_status_color_get(),
                             LCD_COLOR_PANEL_INNER);
        if (err) {
                return err;
        }

        err = app_lcd_fill_rect(LCD_MAIN_CARD_X + 1U, LCD_MAIN_CARD_Y + 1U,
                                lcd_caps.x_resolution - 14U, 8U, lcd_main_status_color_get());
        if (err) {
                return err;
        }

        return lcd_draw_status_text(32, lcd_main_status_text_get(),
                                    lcd_main_status_subtext_get(), LCD_COLOR_TEXT);
}

static int lcd_status_tiles_draw(atomic_val_t dirty_flags)
{
        int err;

        if (dirty_flags & LCD_DIRTY_TILE_PROV) {
                err = lcd_status_tile_draw(4U, 84U, "PROV",
                                           lcd_state_is_active(APP_UI_STATE_PROVISIONED),
                                           LCD_COLOR_CARD_OK);
                if (err) {
                        return err;
                }
        }

        if (dirty_flags & LCD_DIRTY_TILE_ADV) {
                err = lcd_status_tile_draw(42U, 84U, "ADV",
                                           lcd_state_is_active(APP_UI_STATE_FP_ADV),
                                           LCD_COLOR_CARD_INFO);
                if (err) {
                        return err;
                }
        }

        if (dirty_flags & LCD_DIRTY_TILE_RING) {
                err = lcd_status_tile_draw(4U, 110U, "RING",
                                           lcd_state_is_active(APP_UI_STATE_RINGING),
                                           LCD_COLOR_CARD_ALERT);
                if (err) {
                        return err;
                }
        }

        if (dirty_flags & LCD_DIRTY_TILE_MOVE) {
                err = lcd_status_tile_draw(42U, 110U, "MOVE",
                                           lcd_motion_tile_is_active(),
                                           LCD_COLOR_CYAN);
                if (err) {
                        return err;
                }
        }

        if (dirty_flags & LCD_DIRTY_TILE_ID) {
                err = lcd_status_tile_draw(4U, 136U, "ID",
                                           lcd_state_is_active(APP_UI_STATE_ID_MODE),
                                           LCD_COLOR_CARD_INFO);
                if (err) {
                        return err;
                }
        }

        if (dirty_flags & LCD_DIRTY_TILE_DFU) {
                err = lcd_status_tile_draw(42U, 136U, "DFU",
                                           lcd_state_is_active(APP_UI_STATE_DFU_MODE),
                                           LCD_COLOR_CARD_INFO);
                if (err) {
                        return err;
                }
        }

        return 0;
}

static int lcd_dashboard_render(atomic_val_t dirty_flags)
{
        int err;

        if (!lcd_ready) {
                return -ENODEV;
        }

        if (dirty_flags & LCD_DIRTY_STATIC) {
                err = lcd_static_layout_draw();
                if (err) {
                        return err;
                }

                dirty_flags |= LCD_DIRTY_ALL_DYNAMIC;
        }

        if (dirty_flags & LCD_DIRTY_BATTERY) {
                err = lcd_battery_draw(LCD_BATTERY_X, LCD_BATTERY_Y);
                if (err) {
                        return err;
                }
        }

        if (dirty_flags & LCD_DIRTY_MAIN_CARD) {
                err = lcd_main_card_draw();
                if (err) {
                        return err;
                }
        }

        err = lcd_status_tiles_draw(dirty_flags);
        if (err) {
                return err;
        }

        return 0;
}

int app_lcd_show_test_pattern(void)
{
        static const uint16_t bars[] = {
                0xF800, /* red */
                0x07E0, /* green */
                0x001F, /* blue */
                0xFFE0, /* yellow */
                0xF81F, /* magenta */
                0x07FF, /* cyan */
                0xFFFF, /* white */
                0x0000, /* black */
        };
        const uint16_t bar_height = MAX(1U, lcd_caps.y_resolution / ARRAY_SIZE(bars));
        int err;

        if (!lcd_ready) {
                return -ENODEV;
        }

        for (uint16_t i = 0; i < ARRAY_SIZE(bars); i++) {
                uint16_t y = i * bar_height;
                uint16_t height = bar_height;

                if (y >= lcd_caps.y_resolution) {
                        break;
                }

                if ((y + height) > lcd_caps.y_resolution) {
                        height = lcd_caps.y_resolution - y;
                }

                err = app_lcd_fill_rect(0U, y, lcd_caps.x_resolution, height, bars[i]);
                if (err) {
                        return err;
                }
        }

        err = app_lcd_fill_rect(0U, 0U, MIN(8U, lcd_caps.x_resolution),
                                MIN(8U, lcd_caps.y_resolution), 0xFFFF);
        if (err) {
                return err;
        }

        err = app_lcd_fill_rect(lcd_caps.x_resolution - MIN(8U, lcd_caps.x_resolution), 0U,
                                MIN(8U, lcd_caps.x_resolution),
                                MIN(8U, lcd_caps.y_resolution), 0xFFFF);
        if (err) {
                return err;
        }

        err = app_lcd_fill_rect(0U, lcd_caps.y_resolution - MIN(8U, lcd_caps.y_resolution),
                                MIN(8U, lcd_caps.x_resolution),
                                MIN(8U, lcd_caps.y_resolution), 0xFFFF);
        if (err) {
                return err;
        }

        return app_lcd_fill_rect(lcd_caps.x_resolution - MIN(8U, lcd_caps.x_resolution),
                                 lcd_caps.y_resolution - MIN(8U, lcd_caps.y_resolution),
                                 MIN(8U, lcd_caps.x_resolution),
                                 MIN(8U, lcd_caps.y_resolution), 0xFFFF);
}

static void lcd_refresh_work_handler(struct k_work *work)
{
        atomic_val_t dirty_flags;

        ARG_UNUSED(work);

        if (!lcd_ready) {
                return;
        }

        dirty_flags = atomic_set(&lcd_dirty_flags, 0);
        if (dirty_flags == 0) {
                return;
        }

        (void)lcd_dashboard_render(dirty_flags);
}

static void lcd_motion_event_clear_work_handler(struct k_work *work)
{
        ARG_UNUSED(work);

        lcd_motion_event_active = false;

        if (lcd_ready) {
                lcd_refresh_schedule(LCD_DIRTY_TILE_MOVE);
        }
}

int app_lcd_run_test_sequence(void)
{
        if (!lcd_ready) {
                return -ENODEV;
        }

        return app_lcd_show_test_pattern();
}

int app_lcd_state_change_indicate(enum app_ui_state state, bool active)
{
        atomic_val_t dirty_flags = 0;

        __ASSERT_NO_MSG(state < APP_UI_STATE_COUNT);

        atomic_set_bit_to(lcd_ui_state, state, active);

        switch (state) {
        case APP_UI_STATE_APP_RUNNING:
        case APP_UI_STATE_RECOVERY_MODE:
                dirty_flags = LCD_DIRTY_MAIN_CARD;
                break;
        case APP_UI_STATE_PROVISIONED:
                dirty_flags = LCD_DIRTY_MAIN_CARD | LCD_DIRTY_TILE_PROV;
                break;
        case APP_UI_STATE_FP_ADV:
                dirty_flags = LCD_DIRTY_MAIN_CARD | LCD_DIRTY_TILE_ADV;
                break;
        case APP_UI_STATE_RINGING:
                dirty_flags = LCD_DIRTY_MAIN_CARD | LCD_DIRTY_TILE_RING;
                break;
        case APP_UI_STATE_ID_MODE:
                dirty_flags = LCD_DIRTY_MAIN_CARD | LCD_DIRTY_TILE_ID;
                break;
        case APP_UI_STATE_DFU_MODE:
                dirty_flags = LCD_DIRTY_MAIN_CARD | LCD_DIRTY_TILE_DFU;
                break;
        case APP_UI_STATE_MOTION_DETECTOR_ACTIVE:
                break;
        default:
                dirty_flags = LCD_DIRTY_MAIN_CARD;
                break;
        }

        if (dirty_flags != 0) {
                lcd_refresh_schedule(dirty_flags);
        }

        return 0;
}

int app_lcd_battery_level_set(uint8_t level)
{
        lcd_battery_level = level;

        lcd_refresh_schedule(LCD_DIRTY_BATTERY);

        return 0;
}

int app_lcd_motion_event_indicate(void)
{
        lcd_motion_event_active = true;

        if (lcd_ready) {
                lcd_refresh_schedule(LCD_DIRTY_TILE_MOVE);
                (void)k_work_reschedule(&lcd_motion_event_clear_work,
                                        K_MSEC(LCD_MOTION_EVENT_HOLD_MS));
        }

        return 0;
}

int app_lcd_init(void)
{
        int err = lcd_backlight_init();

        if (err) {
                return err;
        }

        err = app_lcd_backlight_set(true);
        if (err) {
                return err;
        }

#if DT_HAS_CHOSEN(zephyr_display)
        lcd_display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
        if (!device_is_ready(lcd_display)) {
                return -ENODEV;
        }

        display_get_capabilities(lcd_display, &lcd_caps);
        lcd_ready = true;

        LOG_INF("LCD ready: %ux%u", lcd_caps.x_resolution, lcd_caps.y_resolution);

        (void)display_set_pixel_format(lcd_display, PIXEL_FORMAT_RGB_565);
        (void)display_blanking_off(lcd_display);

        k_work_init_delayable(&lcd_refresh_work, lcd_refresh_work_handler);
        k_work_init_delayable(&lcd_motion_event_clear_work, lcd_motion_event_clear_work_handler);
        lcd_battery_level = (uint8_t)MAX(app_battery_level_get(), 0);

        err = lcd_dashboard_render(LCD_DIRTY_ALL);
        if (err) {
                LOG_WRN("LCD dashboard render failed (err %d)", err);
        }

        return 0;
#else
        return -ENODEV;
#endif
}
