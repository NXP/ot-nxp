/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_OT_H_
#define _APP_OT_H_

#include "EmbeddedTypes.h"
#include "fsl_os_abstraction.h"

void OT_Init(void);
void OT_Enable(void);
void OT_Disable(void);
void App_OT_Thread(osa_task_param_t param);

#endif /* _APP_OT_H_ */
