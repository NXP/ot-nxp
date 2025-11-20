/*
 *  Copyright (c) 2025, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PLATFORM_MCXW72_NBU_H_
#define PLATFORM_MCXW72_NBU_H_

#include <openthread-core-config.h>
#include <openthread/config.h>
#include <openthread/instance.h>

#include "EmbeddedTypes.h"
#include <stdint.h>

#define PLAT_SETTINGS_BUFF_SIZE 1028

struct plat_settings_t
{
    uint8_t ram_buff[PLAT_SETTINGS_BUFF_SIZE];
    uint8_t status;
} __attribute__((packed));

/**
 * This function initializes the alarm service used by OpenThread.
 *
 */
void otPlatAlarmInit(void);

/**
 * This function performs alarm driver processing.
 *
 * @param[in]  aInstance  The OpenThread instance structure.
 *
 */
void otPlatAlarmProcess(otInstance *aInstance);

/**
 * This function performs uart driver processing.
 *
 */
void otPlatUartProcess();

/**
 * This function initializes the radio service used by OpenThread.
 *
 */
void otPlatRadioInit(void);

/**
 * This function performs radio driver processing.
 *
 * @param[in]  aInstance  The OpenThread instance structure.
 *
 */
void otPlatRadioProcess(otInstance *aInstance);

/**
 * This function initializes the random number service used by OpenThread.
 *
 */
void otPlatRandomInit(void);

#if (OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_PLATFORM_DEFINED)
/**
 * This function initializes the platform defined logging.
 *
 */
void otPlatLogInit();
#endif /* OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_PLATFORM_DEFINED */

void PLATFORM_GetIeee802_15_4Addr(uint8_t *eui64);

void   PWR_AllowDeviceToSleep();
void   PWR_DisallowDeviceToSleep();
bool_t plat_lp_allowed();

void plat_settings_load(void *data);
void plat_settings_save(void *data);

#endif // PLATFORM_MCXW72_NBU_H_
