#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include <bluetooth/fast_pair/fhn/fhn.h>

#include "app_board_pins.h"
#include "app_lcd.h"
#include "app_motion_detector.h"
#include "app_ui.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(fp_fhn, LOG_LEVEL_INF);

#define DA217_REG_SPI_CONFIG           0x00
#define DA217_REG_CHIP_ID              0x01
#define DA217_REG_MOTION_FLAG          0x09
#define DA217_REG_RESOLUTION_RANGE     0x0F
#define DA217_REG_ODR_AXIS             0x10
#define DA217_REG_MODE_BW              0x11
#define DA217_REG_INT_SET1             0x16
#define DA217_REG_INT_MAP1             0x19
#define DA217_REG_INT_CONFIG           0x20
#define DA217_REG_INT_LATCH            0x21
#define DA217_REG_ACTIVE_DUR           0x27
#define DA217_REG_ACTIVE_THS           0x28

#define DA217_CHIP_ID_VALUE            0x13

#define DA217_MOTION_FLAG_ACTIVE_INT   BIT(2)

#define DA217_INT_SET1_ACTIVE_X_EN     BIT(0)
#define DA217_INT_SET1_ACTIVE_Y_EN     BIT(1)
#define DA217_INT_SET1_ACTIVE_Z_EN     BIT(2)

#define DA217_INT_MAP1_INT1_ACTIVE     BIT(2)

#define DA217_MODE_BW_PWR_OFF          BIT(7)

#define DA217_ODR_31_25_HZ             0x05
#define DA217_FS_2G_14BIT_WDT_1MS      0x40
#define DA217_ACTIVE_THS_DEFAULT       0x14
#define DA217_ACTIVE_DUR_2_SAMPLES     0x11

struct da217_motion_detector {
	const struct device *i2c_dev;
	const struct device *gpio_dev;
	struct gpio_callback gpio_cb;
	struct k_work work;
	bool active;
	bool motion_detected;
};

static void da217_work_handle(struct k_work *work);

static struct da217_motion_detector motion_detector = {
	.i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0)),
	.gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0)),
};

static int da217_reg_read(uint8_t reg, uint8_t *val)
{
	return i2c_reg_read_byte(motion_detector.i2c_dev, APP_DA217_I2C_ADDR, reg, val);
}

static int da217_reg_write(uint8_t reg, uint8_t val)
{
	return i2c_reg_write_byte(motion_detector.i2c_dev, APP_DA217_I2C_ADDR, reg, val);
}

static int da217_reg_update(uint8_t reg, uint8_t mask, uint8_t val)
{
	return i2c_reg_update_byte(motion_detector.i2c_dev, APP_DA217_I2C_ADDR, reg, mask, val);
}

static int da217_power_set(bool enable)
{
	return da217_reg_update(DA217_REG_MODE_BW,
				DA217_MODE_BW_PWR_OFF,
				enable ? 0 : DA217_MODE_BW_PWR_OFF);
}

static int da217_motion_int_set(bool enable)
{
	uint8_t val = enable ?
		(DA217_INT_SET1_ACTIVE_X_EN | DA217_INT_SET1_ACTIVE_Y_EN |
		 DA217_INT_SET1_ACTIVE_Z_EN) : 0;

	int err = da217_reg_write(DA217_REG_INT_SET1, val);
	if (err) {
		return err;
	}

        return da217_reg_write(DA217_REG_INT_MAP1, enable ? DA217_INT_MAP1_INT1_ACTIVE : 0);
}

static int da217_motion_flag_read(uint8_t *motion_flag)
{
	return da217_reg_read(DA217_REG_MOTION_FLAG, motion_flag);
}

static void da217_gpio_callback(const struct device *port,
				struct gpio_callback *cb,
				uint32_t pins)
{
	ARG_UNUSED(port);
	ARG_UNUSED(cb);
        ARG_UNUSED(pins);

	(void)k_work_submit(&motion_detector.work);
}

static void da217_work_handle(struct k_work *work)
{
	ARG_UNUSED(work);

	if (!motion_detector.active) {
		return;
	}

	uint8_t motion_flag = 0;
	int err = da217_motion_flag_read(&motion_flag);
	if (err) {
		LOG_ERR("Failed to read DA217 motion flag: %d", err);
		return;
	}

	if (motion_flag & DA217_MOTION_FLAG_ACTIVE_INT) {
		LOG_INF("DA217 motion interrupt detected");
		motion_detector.motion_detected = true;
                (void)app_lcd_motion_event_indicate();
	}
}

static int da217_gpio_int_configure(bool enable)
{
	gpio_flags_t flags = GPIO_INPUT;
	int err = gpio_pin_configure(motion_detector.gpio_dev, APP_GSENSOR_INT_PIN, flags);
	if (err) {
		return err;
	}

        err = gpio_pin_interrupt_configure(motion_detector.gpio_dev,
                                           APP_GSENSOR_INT_PIN,
                                           enable ? GPIO_INT_EDGE_RISING : GPIO_INT_DISABLE);
        return err;
}

static int da217_hw_init(void)
{
	uint8_t chip_id = 0;
	int err;

	err = da217_reg_write(DA217_REG_SPI_CONFIG, BIT(7));
	if (err) {
		LOG_ERR("Failed to soft reset DA217: %d", err);
		return err;
	}

	k_msleep(5);

	err = da217_reg_read(DA217_REG_CHIP_ID, &chip_id);
	if (err) {
		LOG_ERR("Failed to read DA217 chip id: %d", err);
		return err;
	}

	if (chip_id != DA217_CHIP_ID_VALUE) {
		LOG_ERR("Unexpected DA217 chip id: 0x%02x", chip_id);
		return -ENODEV;
	}

        LOG_INF("DA217 chip id ok: 0x%02x", chip_id);

	err = da217_reg_write(DA217_REG_RESOLUTION_RANGE, DA217_FS_2G_14BIT_WDT_1MS);
	if (err) {
		return err;
	}

	err = da217_reg_write(DA217_REG_ODR_AXIS, DA217_ODR_31_25_HZ);
	if (err) {
		return err;
	}

	err = da217_power_set(true);
	if (err) {
		return err;
	}

	err = da217_reg_write(DA217_REG_INT_LATCH, 0x00);
	if (err) {
		return err;
	}

	err = da217_reg_write(DA217_REG_INT_CONFIG, 0x00);
	if (err) {
		return err;
	}

	err = da217_reg_write(DA217_REG_ACTIVE_DUR, DA217_ACTIVE_DUR_2_SAMPLES);
	if (err) {
		return err;
	}

	err = da217_reg_write(DA217_REG_ACTIVE_THS, DA217_ACTIVE_THS_DEFAULT);
	if (err) {
		return err;
	}

        LOG_INF("DA217 threshold config: odr=0x%02x range=0x%02x active_dur=0x%02x active_ths=0x%02x",
                DA217_ODR_31_25_HZ,
                DA217_FS_2G_14BIT_WDT_1MS,
                DA217_ACTIVE_DUR_2_SAMPLES,
                DA217_ACTIVE_THS_DEFAULT);

	err = da217_motion_int_set(false);
	if (err) {
		return err;
	}

	return da217_power_set(false);
}

static void state_set(bool enable)
{
	int err;

	if (motion_detector.active == enable) {
		LOG_DBG("Motion detection already %sabled", enable ? "en" : "dis");
		return;
	}

	motion_detector.motion_detected = false;

	if (enable) {
		err = da217_power_set(true);
		if (err) {
			LOG_ERR("Failed to power on DA217: %d", err);
			return;
		}

		err = da217_motion_int_set(true);
		if (err) {
			LOG_ERR("Failed to enable DA217 motion interrupt: %d", err);
			return;
		}

		err = da217_gpio_int_configure(true);
		if (err) {
			LOG_ERR("Failed to enable G-sensor GPIO interrupt: %d", err);
			return;
		}
	} else {
		(void)da217_gpio_int_configure(false);
		(void)da217_motion_int_set(false);
		(void)da217_power_set(false);
	}

	motion_detector.active = enable;
	LOG_INF("Motion detection: %sabled", motion_detector.active ? "en" : "dis");
}

static void fhn_motion_detector_start(void)
{
	__ASSERT_NO_MSG(!k_is_preempt_thread());
	__ASSERT_NO_MSG(!k_is_in_isr());

        __ASSERT_NO_MSG(!motion_detector.active);

	LOG_INF("FHN: motion detector started");
	state_set(true);
	app_ui_state_change_indicate(APP_UI_STATE_MOTION_DETECTOR_ACTIVE, motion_detector.active);
}

static bool fhn_motion_detector_period_expired(void)
{
	bool ret = motion_detector.motion_detected;

	__ASSERT_NO_MSG(!k_is_preempt_thread());
	__ASSERT_NO_MSG(!k_is_in_isr());
	__ASSERT_NO_MSG(motion_detector.active);

	LOG_INF("FHN: motion detector period expired. Reporting that the motion was %sdetected",
		ret ? "" : "not ");

	motion_detector.motion_detected = false;
	return ret;
}

static void fhn_motion_detector_stop(void)
{
	__ASSERT_NO_MSG(!k_is_preempt_thread());
	__ASSERT_NO_MSG(!k_is_in_isr());

	__ASSERT_NO_MSG(motion_detector.active);

	LOG_INF("FHN: motion detector stopped");
	state_set(false);
	app_ui_state_change_indicate(APP_UI_STATE_MOTION_DETECTOR_ACTIVE, motion_detector.active);
}

static const struct bt_fast_pair_fhn_motion_detector_cb fhn_motion_detector_cb = {
	.start = fhn_motion_detector_start,
	.period_expired = fhn_motion_detector_period_expired,
	.stop = fhn_motion_detector_stop,
};

int app_motion_detector_init(void)
{
	int err;

	if (!device_is_ready(motion_detector.i2c_dev)) {
		LOG_ERR("DA217 I2C device not ready");
		return -ENODEV;
	}

	if (!device_is_ready(motion_detector.gpio_dev)) {
		LOG_ERR("DA217 GPIO device not ready");
		return -ENODEV;
	}

	k_work_init(&motion_detector.work, da217_work_handle);
	gpio_init_callback(&motion_detector.gpio_cb,
			   da217_gpio_callback,
			   BIT(APP_GSENSOR_INT_PIN));

	err = gpio_add_callback(motion_detector.gpio_dev, &motion_detector.gpio_cb);
	if (err) {
		LOG_ERR("Failed to add G-sensor GPIO callback: %d", err);
		return err;
	}

	err = da217_hw_init();
	if (err) {
		LOG_ERR("Failed to initialize DA217: %d", err);
		return err;
	}

	err = bt_fast_pair_fhn_motion_detector_cb_register(&fhn_motion_detector_cb);
	if (err) {
		LOG_ERR("FHN: bt_fast_pair_fhn_motion_detector_cb_register failed (err %d)", err);
		return err;
	}

	LOG_INF("DA217 motion detector initialized");

	return 0;
}
