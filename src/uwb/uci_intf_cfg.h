/*
 * Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_UWB_UCI_INTF_CFG_H__
#define APP_UWB_UCI_INTF_CFG_H__

#define UCI_INTF_SPI_FDX    0
#define UCI_INTF_SPI_HDX    0
#define UCI_INTF_UART       1
#define UCI_INTF_UART_HSK   0
#define UCI_INTF_UCAN       0

#if (UCI_INTF_SPI_FDX + UCI_INTF_SPI_HDX + UCI_INTF_UART + UCI_INTF_UART_HSK + UCI_INTF_UCAN) > 1
#error Only one UCI interface type can be enabled
#elif (UCI_INTF_SPI_FDX + UCI_INTF_SPI_HDX + UCI_INTF_UART + UCI_INTF_UART_HSK + UCI_INTF_UCAN) == 0
#error At least one UCI interface type must be enabled
#endif

#define UCI_RECV_BUF_ITEMS      CONFIG_APP_UWB_RX_BUF_ITEMS
#define UCI_RECV_BUF_LEN        CONFIG_APP_UWB_RX_BUF_LEN
#define UCI_UWBS_PKT_SLOTS      CONFIG_APP_UWB_PKT_SLOTS

#endif /* APP_UWB_UCI_INTF_CFG_H__ */
