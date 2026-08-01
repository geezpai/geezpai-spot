/*
 * Copyright (c) 2026 Shenzhen Geezpai Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_UWB_UCI_DEBUG_H__
#define APP_UWB_UCI_DEBUG_H__

#include <zephyr/sys/__assert.h>

#define UCI_TRACE(msg, ...)
#define UCI_TRACE0(msg, ...)
#define UCI_TRACE1(msg, ...)
#define UCI_TRACE2(msg, ...)
#define UCI_TRACE3(msg, ...)
#define UCI_TRACE4(msg, ...)
#define UCI_TRACE5(msg, ...)
#define UCI_ASSERT(exp, msg, ...) __ASSERT(exp, msg)

#endif /* APP_UWB_UCI_DEBUG_H__ */
