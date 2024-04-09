/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __OT_OPCODE_H__
#define __OT_OPCODE_H__

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

//#include <stdint.h>

/* -------------------------------------------------------------------------- */
/*                                  Function prototypes                       */
/* -------------------------------------------------------------------------- */

int8_t ot_get_opcode(uint8_t *userinputcmd, uint8_t otcmdlen);

#endif /* __OT_OPCODE_H__ */
