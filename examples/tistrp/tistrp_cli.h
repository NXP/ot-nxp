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
 * @brief Initialize the TIS/TRP DUT addon.
 *
 * With OT_APP_TISTRP_AUTOSTART=ON (set by the iw610_spi_tistrp flavor of script/build_rt1060) the DUT mode is
 * entered at boot on channel 12, so the device can be driven over the air without any serial command (chamber
 * use). Otherwise (addon default OFF) it is started manually with "tistrp start".
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
