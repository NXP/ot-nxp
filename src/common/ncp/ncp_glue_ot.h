/* @file ncp_glue_ot.h
 *
 *  @brief This file contains ncp API functions definitions
 *
 *  Copyright 2008-2024 NXP
 *
 *  Licensed under the LA_OPT_NXP_Software_License.txt (the "Agreement")
 */

#ifndef __NCP_GLUE_OT_H__
#define __NCP_GLUE_OT_H__

#include "ncp_cmd_ot.h"

typedef enum
{
    NCP_COMMAND_READY     = 0,
    NCP_COMMAND_NOT_READY = 1
} ncp_cmd_status;

/* TLV command response */
ncp_status_t ot_send_response(uint32_t cmd, uint8_t status, uint8_t *data, size_t len);

NCPCmd_DS_COMMAND *ncp_get_ot_response_buffer();

void ot_ncp_clear_cmd_ready(void);
void ot_ncp_copy_cmd_buff(uint8_t *src, uint16_t *pLen);

ncp_cmd_status ot_ncp_check_cmd_ready(void);

#endif /* __NCP_GLUE_OT_H__ */
