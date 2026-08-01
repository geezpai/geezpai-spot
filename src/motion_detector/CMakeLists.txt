#
# Copyright (c) 2024-2025 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(CONFIG_APP_PLATFORM_DK)
  if(CONFIG_BOARD_NRF52840DK_NRF52840)
    target_sources(app PRIVATE platform_da217.c)
  else()
    target_sources(app PRIVATE platform_emulated.c)
  endif()
elseif(CONFIG_APP_PLATFORM_REF)
  target_sources(app PRIVATE platform_adxl36x.c)
else()
  message(FATAL_ERROR "Unsupported platform")
endif()
