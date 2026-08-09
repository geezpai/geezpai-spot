#
# Copyright (c) 2026 Nordic Semiconductor ASA
# Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

set(APP_UWB_HOST_ROOT
    ""
    CACHE PATH
    "Path to MKSEMI UCI Host SDK (uci_host_v0.8.1_r0). Required when CONFIG_APP_UWB=y.")

if(CONFIG_APP_UWB)
  if(NOT APP_UWB_HOST_ROOT)
    message(FATAL_ERROR
      "APP_UWB_HOST_ROOT is not set.\n"
      "  Set it on the cmake command line:\n"
      "    -DAPP_UWB_HOST_ROOT=<path>/uci_host_v0.8.1_r0\n"
      "  Or set the environment variable:\n"
      "    set APP_UWB_HOST_ROOT=<path>/uci_host_v0.8.1_r0")
  endif()

  if(NOT EXISTS "${APP_UWB_HOST_ROOT}/src/uci_api/uci_api.c")
    message(FATAL_ERROR "APP_UWB_HOST_ROOT is invalid: ${APP_UWB_HOST_ROOT}")
  endif()

target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${APP_UWB_HOST_ROOT}/src/platform
  ${APP_UWB_HOST_ROOT}/src/uci_api
  ${APP_UWB_HOST_ROOT}/src/uci_lib
  ${APP_UWB_HOST_ROOT}/src/lib
)

target_sources(app PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/app_uwb.c
  ${CMAKE_CURRENT_LIST_DIR}/uci_port.c
  ${APP_UWB_HOST_ROOT}/src/lib/mk_list.c
  ${APP_UWB_HOST_ROOT}/src/uci_api/uci_api.c
  ${APP_UWB_HOST_ROOT}/src/uci_lib/uci_app.c
  ${APP_UWB_HOST_ROOT}/src/uci_lib/uci_core.c
  ${APP_UWB_HOST_ROOT}/src/uci_lib/uci_helper.c
  ${APP_UWB_HOST_ROOT}/src/uci_lib/uci_lib.c
  ${APP_UWB_HOST_ROOT}/src/uci_lib/uci_msg.c
  ${APP_UWB_HOST_ROOT}/src/uci_lib/uci_ntf.c
  ${APP_UWB_HOST_ROOT}/src/uci_lib/uci_resp.c
)

endif() # CONFIG_APP_UWB
