/*
 *  Copyright 2026 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TISTRP_CLI_H_
#define TISTRP_CLI_H_

#include <openthread/error.h>
#include <openthread/instance.h>

/*!
 * @brief Initialize the TIS/TRP DUT addon (auto-starts the DUT when OT_APP_TISTRP_AUTOSTART is set).
 */
void otAppCliTistrpInit(otInstance *aInstance);

/*!
 * @brief Handler called from ot-cli when the "tistrp" command is used.
 *
 * tistrp start [channel]   enter DUT mode (raw link, promiscuous) on channel (default 12)
 * tistrp stop              leave DUT mode
 * tistrp status            print state, channel, power and RX counters
 * tistrp reboot            reset the device (same path as the OTA REBOOT command)
 */
otError ProcessTistrp(void *aContext, uint8_t aArgsLength, char *aArgs[]);

#endif /* TISTRP_CLI_H_ */
