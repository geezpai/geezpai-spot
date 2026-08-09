#
# Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

get_filename_component(APP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

if(SB_CONFIG_APP_DFU)
  set_config_bool(${DEFAULT_IMAGE} CONFIG_APP_DFU y)

  if(DEFINED SB_CONFIG_NETCORE_IMAGE_NAME AND NOT (SB_CONFIG_NETCORE_IMAGE_NAME STREQUAL ""))
    set(dfu_speedup_kconfig_fragment
        ${APP_ROOT}/sysbuild/common/dfu_speedup_fragment.conf
    )

    if(EXISTS ${dfu_speedup_kconfig_fragment})
      add_overlay_config(
          ${SB_CONFIG_NETCORE_IMAGE_NAME}
          ${dfu_speedup_kconfig_fragment}
      )
    else()
      message(WARNING "DFU speedup fragment for netcore not found")
    endif()
  endif()
else()
  set_config_bool(${DEFAULT_IMAGE} CONFIG_APP_DFU n)
endif()
