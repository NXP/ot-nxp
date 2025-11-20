/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CUSTOM_PREINCLUDE_H_
#define _CUSTOM_PREINCLUDE_H_

#ifdef __cplusplus
extern "C" {
#endif

#undef advState_tag
#define advState_tag                                                                             \
    my_dummy_tag                                                                                 \
    {                                                                                            \
        uint8_t tmp;                                                                             \
    }                                                                                            \
    my_dummy_t;                                                                                  \
    static void hsdkObserverGATTClientRegisterIndicationCallback(bleEvtContainer_t *pContainer); \
    void        my_f()                                                                           \
    {                                                                                            \
        hsdkObserverGATTClientRegisterIndicationCallback(NULL);                                  \
    };                                                                                           \
    typedef struct advState_tag

#undef gAppLedCnt_c
#undef gAppButtonCnt_c
#undef gAppUseSerialManager_c

#include "app_preinclude.h"

#undef gAppLedCnt_c
#undef gAppButtonCnt_c
#undef gAppUseSerialManager_c
#define gAppUseSerialManager_c 1

#undef gWuart_AutoStart_c
#define gWuart_AutoStart_c 1

#undef gWuart_AutoStartGapRole_c
#define gWuart_AutoStartGapRole_c gGapPeripheral_c

#undef gWuart_CentralRole_c
#define gWuart_CentralRole_c 0

#undef gWuart_PeripheralRole_c
#define gWuart_PeripheralRole_c 1

#ifdef __cplusplus
}
#endif

#endif /* _CUSTOM_PREINCLUDE_H_ */
