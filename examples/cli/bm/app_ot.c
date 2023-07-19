/*!
 *
 * Copyright 2021 NXP
 *
 *
 * Common OpenThread APIs for application
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */

#include "app_ot.h"
#include "Phy.h"
#include "assert.h"

/* OT includes */
#include <openthread-system.h>
#include "openthread/cli.h"
#include "openthread/tasklet.h"
#include "openthread/thread.h"

void        App_OT_StackToAppHandler(uint32_t flags, void *param);
extern void otAppCliInit(otInstance *aInstance);

/*! *********************************************************************************
*************************************************************************************
* Private memory definition
*************************************************************************************
********************************************************************************** */
static otInstance *openThreadInstance = NULL;
static char        initStr[]          = "sdk_app";

static otPanId         gPanId    = 0xaaaa;
static uint8_t         gChannel  = 18U;
static otExtendedPanId gExtPanId = {
    .m8 = {0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0xca, 0xfe},
};
static otNetworkKey gMasterKey = {
    .m8 = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
};

void OT_Init(void)
{
    char *argv[1] = {0};
    argv[0]       = &initStr[0];

    otSysInit(1, argv);

    openThreadInstance = otInstanceInitSingle();
    assert(openThreadInstance);

    otAppCliInit(openThreadInstance);

    otSetStateChangedCallback(openThreadInstance, App_OT_StackToAppHandler, NULL);

    otCliOutputFormat("OpenThread Stack initialized\n\r");

    /* Directly configure OT stack and start activity */
    otLinkSetPanId(openThreadInstance, gPanId);
    otThreadSetExtendedPanId(openThreadInstance, &gExtPanId);
    otThreadSetNetworkKey(openThreadInstance, &gMasterKey);
    otLinkSetChannel(openThreadInstance, gChannel);
}

void OT_Enable(void)
{
    if (openThreadInstance != NULL)
    {
#if defined(USE_NBU) && (USE_NBU == 1)
        /* Phy is implemented on NBU */
#else
        PhyRadioReinit();
#endif
        otThreadSetEnabled(openThreadInstance, TRUE);
        otPlatRadioEnable(openThreadInstance);
        App_OT_Thread(0);
    }
}

void OT_Disable(void)
{
    if (openThreadInstance != NULL)
    {
        otThreadSetEnabled(openThreadInstance, FALSE);
        otPlatRadioDisable(openThreadInstance);
        App_OT_Thread(0);
    }
}

void App_OT_Thread(osa_task_param_t param)
{
    (void)param;

    otTaskletsProcess(openThreadInstance);
    otSysProcessDrivers(openThreadInstance);
}

void App_OT_StackToAppHandler(uint32_t flags, void *param)
{
    if ((flags & OT_CHANGED_THREAD_ROLE) != 0)
    {
        otDeviceRole devRole = otThreadGetDeviceRole(openThreadInstance);

        switch (devRole)
        {
        case OT_DEVICE_ROLE_CHILD:
            otCliOutputFormat("Node has taken Child role\n\r");
            break;

        case OT_DEVICE_ROLE_ROUTER:
            otCliOutputFormat("Node has taken Router role\n\r");
            break;

        case OT_DEVICE_ROLE_LEADER:
            otCliOutputFormat("Node has taken Leader role\n\r");
            break;

        case OT_DEVICE_ROLE_DETACHED:
            otCliOutputFormat("Node is detached\n\r");
            break;

        case OT_DEVICE_ROLE_DISABLED:
            otCliOutputFormat("Thread interface disabled\n\r");
            break;

        default:
            break;
        }
    }
}
