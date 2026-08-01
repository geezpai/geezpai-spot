/*
 * Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "app_uwb.h"

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "uci_api.h"
#include "uci_common.h"

LOG_MODULE_REGISTER(app_uwb, LOG_LEVEL_INF);

#define APP_UWB_BUS_ID                  0U
#define APP_UWB_DEVICE_ID               1U
#define APP_UWB_THREAD_STACK_SIZE       3072
#define APP_UWB_THREAD_PRIORITY         7
#define APP_UWB_SEGT_BUF_WORDS          DIV_ROUND_UP(CONFIG_APP_UWB_RX_BUF_LEN, sizeof(uint32_t))

static const struct gpio_dt_spec uwb_int_gpio = GPIO_DT_SPEC_GET(DT_ALIAS(uwb_int), gpios);

static struct gpio_callback uwb_int_cb_data;
static struct k_thread uwb_thread_data;
static K_THREAD_STACK_DEFINE(uwb_thread_stack, APP_UWB_THREAD_STACK_SIZE);
static K_SEM_DEFINE(uwb_loop_sem, 0, 1);

static uint32_t uwb_segt_buf[APP_UWB_SEGT_BUF_WORDS];
static uwbs_t *uwb_inst;
static bool uwb_thread_started;
static uint32_t uwb_next_due = UINT32_MAX;

static void uwb_loop_kick(void)
{
        k_sem_give(&uwb_loop_sem);
}

void uci_sched_me(void)
{
        uwb_loop_kick();
}

static void uwb_log_result(const char *name, int32_t ret)
{
        if (ret != UEC_SUCCESS) {
                LOG_ERR("UWB %s failed: 0x%04x", name, (uint16_t)ret);
        }
}

static void uwb_evt_callback(uwbs_t *uwbs, const uci_cb_evt_t *evt)
{
        ARG_UNUSED(uwbs);

        switch (evt->whoami) {
        case UCI_GET_DEV_INFO: {
                const uci_dev_info_t *info = &evt->u.aret->dev_info;

                LOG_INF("UWB info: UCI %u.%u MAC %u.%u PHY %u.%u",
                        info->uci_major_ver, info->uci_minor_ver,
                        info->mac_major_ver, info->mac_minor_ver,
                        info->phy_major_ver, info->phy_minor_ver);
                break;
        }

        case UCI_GET_CAPS_INFO: {
                const uci_dev_caps_info_t *caps = &evt->u.aret->caps_info;

                LOG_INF("UWB caps: msg=%u payload=%u channels=0x%02x roles=0x%04x",
                        caps->max_msg_size, caps->max_pld_size,
                        caps->channels, caps->devie_roles);
                break;
        }

        case UCI_DEV_STATUS:
                LOG_INF("UWB device state: 0x%02x", evt->u.ntf->dev_status.state);
                break;

        case UCI_SESSION_STATUS:
                LOG_INF("UWB session 0x%08x state=0x%02x code=0x%02x",
                        evt->u.ntf->sess_state.session,
                        evt->u.ntf->sess_state.state,
                        evt->u.ntf->sess_state.code);
                break;

        case UCI_UWBS_DESTROYED:
                LOG_INF("UWB instance destroyed");
                break;

        default:
                LOG_DBG("UWB event 0x%04x received", evt->whoami);
                break;
        }
}

static void uwb_irq_callback(const struct device *port, struct gpio_callback *cb, uint32_t pins)
{
        ARG_UNUSED(port);
        ARG_UNUSED(cb);
        ARG_UNUSED(pins);

        uwb_loop_kick();
}

static void uwb_thread(void *arg1, void *arg2, void *arg3)
{
        ARG_UNUSED(arg1);
        ARG_UNUSED(arg2);
        ARG_UNUSED(arg3);

        while (true) {
                k_timeout_t timeout = K_FOREVER;

                if (uwb_next_due != UINT32_MAX) {
                        uint32_t now = k_uptime_get_32();

                        timeout = (uwb_next_due <= now) ? K_NO_WAIT : K_MSEC(uwb_next_due - now);
                }

                (void)k_sem_take(&uwb_loop_sem, timeout);

                if (uwb_inst == NULL) {
                        continue;
                }

                do {
                        uint32_t ttw = uci_host_loop();

                        if (ttw == UINT32_MAX) {
                                uwb_next_due = UINT32_MAX;
                        } else {
                                uwb_next_due = uci_get_tick() + ttw;
                        }
                } while ((uwb_next_due != UINT32_MAX) &&
                         (uwb_next_due <= uci_get_tick() ||
                          (k_sem_take(&uwb_loop_sem, K_NO_WAIT) == 0)));
        }
}

static int uwb_irq_init(void)
{
        int err;

        if (!gpio_is_ready_dt(&uwb_int_gpio)) {
                        return -ENODEV;
        }

        err = gpio_pin_configure_dt(&uwb_int_gpio, GPIO_INPUT | GPIO_PULL_UP);
        if (err) {
                return err;
        }

        gpio_init_callback(&uwb_int_cb_data, uwb_irq_callback, BIT(uwb_int_gpio.pin));

        err = gpio_add_callback(uwb_int_gpio.port, &uwb_int_cb_data);
        if (err) {
                return err;
        }

        err = gpio_pin_interrupt_configure_dt(&uwb_int_gpio, GPIO_INT_EDGE_BOTH);
        if (err) {
                gpio_remove_callback(uwb_int_gpio.port, &uwb_int_cb_data);
                return err;
        }

        return 0;
}

static void uwb_thread_start_once(void)
{
        if (uwb_thread_started) {
                return;
        }

        (void)k_thread_create(&uwb_thread_data, uwb_thread_stack,
                              K_THREAD_STACK_SIZEOF(uwb_thread_stack),
                              uwb_thread, NULL, NULL, NULL,
                              K_PRIO_PREEMPT(APP_UWB_THREAD_PRIORITY), 0, K_NO_WAIT);
        k_thread_name_set(&uwb_thread_data, "app_uwb");
        uwb_thread_started = true;
}

int app_uwb_init(void)
{
        int err;
        int32_t ret;

        if (uwb_inst != NULL) {
                return 0;
        }

        err = uwb_irq_init();
        if (err) {
                LOG_ERR("UWB INT init failed (err %d)", err);
                return err;
        }

        uwb_thread_start_once();

        LOG_INF("UWB host version: %s", uci_host_version());

        uci_host_set_response_timeout(CONFIG_APP_UWB_RESPONSE_TIMEOUT_MS);
        uci_host_set_reassemble_timeout(CONFIG_APP_UWB_REASSEMBLE_TIMEOUT_MS);
        uci_host_set_auto_flush(true);

        uwb_inst = uci_create_uwbs(APP_UWB_BUS_ID, APP_UWB_DEVICE_ID,
                                   CONFIG_APP_UWB_PKT_SLOTS, uwb_evt_callback);
        if (uwb_inst == NULL) {
                LOG_ERR("UWB instance create failed");
                return -ENOMEM;
        }

        ret = uci_assign_uwbs_segts_buffer(uwb_inst, uwb_segt_buf, sizeof(uwb_segt_buf));
        if (ret != UEC_SUCCESS) {
                uwb_log_result("assign seg buffer", ret);
        }

        ret = uci_core_device_reset(uwb_inst, 0);
        if (ret != UEC_SUCCESS) {
                uwb_log_result("device reset", ret);
                return -EIO;
        }

        ret = uci_core_get_device_info(uwb_inst);
        if (ret != UEC_SUCCESS) {
                uwb_log_result("get device info", ret);
                return -EIO;
        }

        ret = uci_core_get_caps_info(uwb_inst);
        if (ret != UEC_SUCCESS) {
                uwb_log_result("get caps info", ret);
                return -EIO;
        }

        LOG_INF("UWB initialized: bus=%u id=%u int_pin=%u",
                APP_UWB_BUS_ID, APP_UWB_DEVICE_ID, uwb_int_gpio.pin);

        uwb_loop_kick();

        return 0;
}
