/* @file ncp_ot.h
 *
 *  @brief This file contains ncp bridge API functions definitions
 *
 *  Copyright 2008-2024 NXP
 *
 *  Licensed under the LA_OPT_NXP_Software_License.txt (the "Agreement")
 */

#ifndef __NCP_OT_H__
#define __NCP_OT_H__

#include "ncp_glue_ot.h"

void Copy_to_NCP_buff(const uint8_t *aBuf, uint16_t aBufLength);

ncp_status_t ot_ncp_command_handle_input(uint8_t *cmd);

uint8_t *ot_ncp_handle_response(uint8_t *pbuf, uint16_t *p_len);

ncp_status_t ot_ncp_init(void);

#endif /* __NCP_OT_H__ */
