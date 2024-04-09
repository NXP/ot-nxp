/* @file ncp_bridge_glue_ot.h
 *
 *  @brief This file contains ncp bridge API functions definitions
 *
 *  Copyright 2008-2024 NXP
 *
 *  Licensed under the LA_OPT_NXP_Software_License.txt (the "Agreement")
 */

#ifndef __NCP_GLUE_OT_H__
#define __NCP_GLUE_OT_H__

#include "ncp_cmd_ot.h"

/* TLV command response */
ncp_status_t ot_bridge_send_response(uint32_t cmd, uint8_t status, uint8_t *data, size_t len);

NCPCmd_DS_COMMAND *ncp_bridge_get_ot_response_buffer();

#endif /* __NCP_GLUE_OT_H__ */
