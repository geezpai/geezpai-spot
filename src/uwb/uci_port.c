/*
 * Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "uci_common.h"
#include "uci_intf_cfg.h"
#include "uci_port.h"

LOG_MODULE_REGISTER(app_uwb_port, LOG_LEVEL_INF);

#define APP_UWB_BUS_ID      0U
#define APP_UWB_DEVICE_ID   1U
#define UART_DEVICE_NODE    DT_CHOSEN(uci_interface)

typedef struct {
        uint8_t dump_buf[UCI_RECV_BUF_ITEMS][UCI_RECV_BUF_LEN];
        uint8_t buf_idx;
} uci_intf_uart_t;

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);
static uci_intf_uart_t uci_uart_ctx;

static void uci_frame_process(const uint8_t *buf, uint16_t len)
{
        int32_t ret;
        uint8_t *dump_buf = uci_uart_ctx.dump_buf[uci_uart_ctx.buf_idx];

        memcpy(dump_buf, buf, len);

        uci_uart_ctx.buf_idx++;
        uci_uart_ctx.buf_idx %= UCI_RECV_BUF_ITEMS;

        ret = uci_process_recv_pkt(APP_UWB_BUS_ID, APP_UWB_DEVICE_ID, dump_buf, len);
        if (ret == UEC_SUCCESS) {
                return;
        }

        uci_buf_release(APP_UWB_BUS_ID, APP_UWB_DEVICE_ID, dump_buf);

        if (ret == UEC_UWBS_NOT_FOUND) {
                LOG_WRN("UWB frame dropped: instance not found");
        } else if (ret == UEC_UWBS_IS_DESTROYING) {
                LOG_WRN("UWB frame dropped: instance is destroying");
        } else if (ret == UEC_PKT_SLOTS_OVERFLOW) {
                LOG_WRN("UWB frame dropped: packet slots overflow");
        } else {
                LOG_WRN("UWB frame dropped: 0x%04x", (uint16_t)ret);
        }
}

static void uart_rx_callback(const struct device *dev, void *user_data)
{
        static uint8_t rx_buf[UCI_RECV_BUF_LEN];
        static size_t rx_len;
        uint8_t c;

        ARG_UNUSED(dev);
        ARG_UNUSED(user_data);

        if (!uart_irq_update(uart_dev) || !uart_irq_rx_ready(uart_dev)) {
                return;
        }

        while (uart_fifo_read(uart_dev, &c, 1) == 1) {
                if (rx_len < sizeof(rx_buf)) {
                        rx_buf[rx_len++] = c;
                } else {
                        LOG_WRN("UWB RX overflow, dropping partial frame");
                        rx_len = 0;
                }
        }

        while (rx_len >= 4U) {
                uint16_t payload_len = ((uint16_t)rx_buf[2] << 8) | rx_buf[3];
                uint16_t frame_len = payload_len + 4U;

                if (frame_len > sizeof(rx_buf)) {
                        LOG_WRN("UWB frame too large: %u", frame_len);
                        rx_len = 0;
                        break;
                }

                if (rx_len < frame_len) {
                        break;
                }

                uci_frame_process(rx_buf, frame_len);

                if (rx_len > frame_len) {
                        memmove(rx_buf, rx_buf + frame_len, rx_len - frame_len);
                }

                rx_len -= frame_len;
        }
}

void uci_buf_release(uint8_t bus_id, uint8_t uwbs_id, const uint8_t *ptr)
{
        ARG_UNUSED(bus_id);
        ARG_UNUSED(uwbs_id);
        ARG_UNUSED(ptr);
}

void *uci_malloc(uint16_t len)
{
        return k_malloc(len);
}

void uci_free(void *ptr)
{
        k_free(ptr);
}

uint32_t uci_enter_critical(void)
{
        return irq_lock();
}

void uci_exit_critical(uint32_t key)
{
        irq_unlock(key);
}

uint32_t uci_get_tick(void)
{
        return k_uptime_get_32();
}

void uci_intf_open(uint8_t bus_id)
{
        int err;

        ARG_UNUSED(bus_id);

        if (!device_is_ready(uart_dev)) {
                LOG_ERR("UWB UART device not ready");
                return;
        }

        err = uart_irq_callback_user_data_set(uart_dev, uart_rx_callback, NULL);
        if (err) {
                LOG_ERR("UWB UART callback set failed (err %d)", err);
                return;
        }

        uart_irq_rx_enable(uart_dev);
}

void uci_intf_close(uint8_t bus_id)
{
        ARG_UNUSED(bus_id);

        if (device_is_ready(uart_dev)) {
                uart_irq_rx_disable(uart_dev);
        }
}

int32_t uci_send(uint8_t bus_id, uint8_t uwbs_id, const uint8_t *buf, uint16_t len)
{
        ARG_UNUSED(bus_id);

        if (!device_is_ready(uart_dev)) {
                return -ENODEV;
        }

        for (uint16_t i = 0; i < len; i++) {
                uart_poll_out(uart_dev, buf[i]);
        }

        uci_send_complete(bus_id, uwbs_id);

        return 0;
}

void uci_recov_from_timeout(uint8_t bus_id, uint8_t uwbs_id, const uint8_t *buf, uint16_t len)
{
        ARG_UNUSED(bus_id);
        ARG_UNUSED(uwbs_id);
        ARG_UNUSED(buf);
        ARG_UNUSED(len);

        LOG_WRN("UWB transaction timeout");
}
