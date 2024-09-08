/*
 *  Copyright 2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __OT_NCP_HOST_CLI_H__
#define __OT_NCP_HOST_CLI_H__

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include <stdint.h>

/* -------------------------------------------------------------------------- */
/*                                  Constants                                 */
/* -------------------------------------------------------------------------- */

#define INBAND_WAKEUP_PARAM '0'
#define OUTBAND_WAKEUP_PARAM '1'

#define USB_PM2_ENTER_PARAM '1'
#define USB_PM2_EXIT_PARAM '2'

/* -------------------------------------------------------------------------- */
/*                             Function prototypes                            */
/* -------------------------------------------------------------------------- */

uint32_t ot_ncp_host_cli_init(void);

#endif /* __OT_NCP_HOST_CLI_H__ */
