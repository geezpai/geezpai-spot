#
# Copyright (c) 2024-2026 Nordic Semiconductor ASA
# Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
# Geezpai Spot: Google Fast Pair + FHN Locator Tag
#
# 这是 Geezpai Spot 的主力固件。
# 构建方式:
#   方式 A (推荐): 从项目根目录构建
#     cd geezpai-spot && west build -b nrf52840dk_nrf52840
#   方式 B: 从本示例目录构建
#     cd examples/05_findmy/locator_tag && west build -b nrf52840dk_nrf52840
#
# 功能: Fast Pair 配网 + FHN 定位 + UWB + TFT 仪表盘 + G-Sensor + 蜂鸣器 + DFU
#

cmake_minimum_required(VERSION 3.20.0)

# Resolve the project root (two levels up from this example directory).
get_filename_component(APP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

# Reuse the project root's Kconfig and devicetree configuration.
set(APPLICATION_CONFIG_DIR "${APP_ROOT}/configuration")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(geezpai_spot_locator_tag)

# === Source files from the project root ===

# NORDIC SDK APP START
target_sources(app PRIVATE
  ${APP_ROOT}/src/main.c
  ${APP_ROOT}/src/buzzer.c
  ${APP_ROOT}/src/lcd.c
  ${APP_ROOT}/src/factory_reset.c
)

zephyr_linker_sources(SECTIONS ${APP_ROOT}/src/factory_reset_callbacks.ld)

if(CONFIG_APP_DFU)
  target_sources(app PRIVATE ${APP_ROOT}/src/dfu.c)
endif()

if(CONFIG_APP_RANGING)
  target_sources(app PRIVATE ${APP_ROOT}/src/ranging.c)
endif()

if(NOT CONFIG_BT_FAST_PAIR_FHN_RING_COMP_NONE)
  target_sources(app PRIVATE ${APP_ROOT}/src/ring.c)
endif()

target_include_directories(app PRIVATE ${APP_ROOT}/include)

add_subdirectory(${APP_ROOT}/src/ui ui_build)
add_subdirectory(${APP_ROOT}/src/battery battery_build)

if(CONFIG_BT_FAST_PAIR_FHN_DULT_MOTION_DETECTOR)
  add_subdirectory(${APP_ROOT}/src/motion_detector motion_detector_build)
endif()

if(CONFIG_APP_UWB)
  add_subdirectory(${APP_ROOT}/src/uwb uwb_build)
endif()
# NORDIC SDK APP END
