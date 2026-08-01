/*
 * Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_BOARD_PINS_H__
#define APP_BOARD_PINS_H__

#define APP_GPIO_PIN_MAP(port, pin)    (((port) * 32U) + (pin))

#define APP_I2C_SDA_PORT               0U
#define APP_I2C_SDA_PIN                17U
#define APP_I2C_SDA_GPIO               APP_GPIO_PIN_MAP(APP_I2C_SDA_PORT, APP_I2C_SDA_PIN)

#define APP_I2C_SCL_PORT               0U
#define APP_I2C_SCL_PIN                19U
#define APP_I2C_SCL_GPIO               APP_GPIO_PIN_MAP(APP_I2C_SCL_PORT, APP_I2C_SCL_PIN)

#define APP_DA217_I2C_ADDR             0x27U

#define APP_GSENSOR_INT_PORT           0U
#define APP_GSENSOR_INT_PIN            31U
#define APP_GSENSOR_INT_GPIO           APP_GPIO_PIN_MAP(APP_GSENSOR_INT_PORT, APP_GSENSOR_INT_PIN)

#define APP_UWB_INT_PORT               0U
#define APP_UWB_INT_PIN                23U
#define APP_UWB_INT_GPIO               APP_GPIO_PIN_MAP(APP_UWB_INT_PORT, APP_UWB_INT_PIN)

#define APP_UWB_UART_RX_PORT           0U
#define APP_UWB_UART_RX_PIN            24U
#define APP_UWB_UART_RX_GPIO           APP_GPIO_PIN_MAP(APP_UWB_UART_RX_PORT, APP_UWB_UART_RX_PIN)

#define APP_UWB_UART_TX_PORT           1U
#define APP_UWB_UART_TX_PIN            0U
#define APP_UWB_UART_TX_GPIO           APP_GPIO_PIN_MAP(APP_UWB_UART_TX_PORT, APP_UWB_UART_TX_PIN)

#define APP_LCD_RST_PORT               0U
#define APP_LCD_RST_PIN                2U
#define APP_LCD_RST_GPIO               APP_GPIO_PIN_MAP(APP_LCD_RST_PORT, APP_LCD_RST_PIN)

#define APP_LCD_SPI_SCK_PORT           0U
#define APP_LCD_SPI_SCK_PIN            20U
#define APP_LCD_SPI_SCK_GPIO           APP_GPIO_PIN_MAP(APP_LCD_SPI_SCK_PORT, APP_LCD_SPI_SCK_PIN)

#define APP_LCD_SPI_MOSI_PORT          0U
#define APP_LCD_SPI_MOSI_PIN           21U
#define APP_LCD_SPI_MOSI_GPIO          APP_GPIO_PIN_MAP(APP_LCD_SPI_MOSI_PORT, APP_LCD_SPI_MOSI_PIN)

#define APP_LCD_RS_PORT                0U
#define APP_LCD_RS_PIN                 28U
#define APP_LCD_RS_GPIO                APP_GPIO_PIN_MAP(APP_LCD_RS_PORT, APP_LCD_RS_PIN)

#define APP_LCD_CS_PORT                0U
#define APP_LCD_CS_PIN                 29U
#define APP_LCD_CS_GPIO                APP_GPIO_PIN_MAP(APP_LCD_CS_PORT, APP_LCD_CS_PIN)

#define APP_LCD_BL_PORT                0U
#define APP_LCD_BL_PIN                 30U
#define APP_LCD_BL_GPIO                APP_GPIO_PIN_MAP(APP_LCD_BL_PORT, APP_LCD_BL_PIN)

#define APP_KEY1_PORT                  1U
#define APP_KEY1_PIN                   15U
#define APP_KEY1_GPIO                  APP_GPIO_PIN_MAP(APP_KEY1_PORT, APP_KEY1_PIN)

#define APP_KEY2_PORT                  0U
#define APP_KEY2_PIN                   3U
#define APP_KEY2_GPIO                  APP_GPIO_PIN_MAP(APP_KEY2_PORT, APP_KEY2_PIN)

#define APP_BEEP_PORT                  0U
#define APP_BEEP_PIN                   22U
#define APP_BEEP_GPIO                  APP_GPIO_PIN_MAP(APP_BEEP_PORT, APP_BEEP_PIN)

#define APP_NRF_RESET_PORT             0U
#define APP_NRF_RESET_PIN              18U

#define APP_LOG_UART_TX_PORT           0U
#define APP_LOG_UART_TX_PIN            13U
#define APP_LOG_UART_TX_GPIO           APP_GPIO_PIN_MAP(APP_LOG_UART_TX_PORT, APP_LOG_UART_TX_PIN)

#define APP_LOG_UART_RX_PORT           0U
#define APP_LOG_UART_RX_PIN            14U
#define APP_LOG_UART_RX_GPIO           APP_GPIO_PIN_MAP(APP_LOG_UART_RX_PORT, APP_LOG_UART_RX_PIN)

#define APP_GPIO_RSVD_P0_04_PORT       0U
#define APP_GPIO_RSVD_P0_04_PIN        4U

#define APP_GPIO_RSVD_P0_05_PORT       0U
#define APP_GPIO_RSVD_P0_05_PIN        5U

#define APP_GPIO_RSVD_P0_07_PORT       0U
#define APP_GPIO_RSVD_P0_07_PIN        7U

#define APP_GPIO_RSVD_P0_08_PORT       0U
#define APP_GPIO_RSVD_P0_08_PIN        8U

#define APP_GPIO_RSVD_P0_11_PORT       0U
#define APP_GPIO_RSVD_P0_11_PIN        11U

#define APP_GPIO_RSVD_P0_12_PORT       0U
#define APP_GPIO_RSVD_P0_12_PIN        12U

#define APP_GPIO_RSVD_P1_08_PORT       1U
#define APP_GPIO_RSVD_P1_08_PIN        8U

#define APP_GPIO_RSVD_P1_09_PORT       1U
#define APP_GPIO_RSVD_P1_09_PIN        9U

#endif
