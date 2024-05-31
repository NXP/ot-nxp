/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_PREINCLUDE_H_
#define _APP_PREINCLUDE_H_

#ifdef __cplusplus
extern "C" {
#endif

#undef FWK_RNG_DEPRECATED_API

#undef gAppButtonCnt_c
#undef gBleBondIdentityHeaderSize_c
#undef gNvStorageIncluded_d
#undef gNvFragmentation_Enabled_d
#undef gUnmirroredFeatureSet_d

#define gUseHciTransportDownward_d 1
#define GENERIC_LIST_LIGHT 0
#define MULTICORE_APP 1

#define FSL_OSA_TASK_ENABLE 1
#define FSL_OSA_BM_TIMER_CONFIG FSL_OSA_BM_TIMER_SYSTICK

#define PRINTF_ADVANCED_ENABLE 1
#define SCANF_ADVANCED_ENABLE 1

#define gAppUseSensors_d 0

#define SHELL_NON_BLOCKING_MODE 1
#define SHELL_PRINT_COPYRIGHT 0
#define SHELL_BUFFER_SIZE 256

/*!
 *  Application specific configuration file only
 *  Board Specific Configuration shall be added to board.h file directly such as :
 *  - Number of button on the board,
 *  - Number of LEDs,
 *  - etc...
 */
/*! *********************************************************************************
 *     Board Configuration
 ********************************************************************************** */
/* Number of Button required by the application */
#define gAppButtonCnt_c 0

/* Number of LED required by the application */
#define gAppLedCnt_c 0

/*! Enable Debug Console (PRINTF) */
#define gDebugConsoleEnable_d 1

/*! *********************************************************************************
 *     App Configuration
 ********************************************************************************** */
/*! Maximum number of connections supported for this application */
#define gAppMaxConnections_c 8U

/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d 0

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d 0

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d 0

#define gPasskeyValue_c 999999

/*! Repeated Attempts - Mitigation for pairing attacks */
#define gRepeatedAttempts_d 0

/* Enable Advertising Extension shell commands */
#define BLE_SHELL_AE_SUPPORT 1

#if BLE_SHELL_AE_SUPPORT

#define gGapSimultaneousEAChainedReports_c 2

/* User defined payload pattern and length of extended advertising data */
#define SHELL_EXT_ADV_DATA_PATTERN "\n\rEXTENDED_ADVERTISING_DATA_LARGE_PAYLOAD"
#define SHELL_EXT_ADV_DATA_SIZE (500U)
#endif /* BLE_SHELL_AE_SUPPORT */

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d 1

/*! The minimum heap size needed:
    4 blocks of 32
    6 blocks of 80
    16 blocks of 288
    1 block of 312
    2 blocks of 400
*/
#define MinimalHeapSize_c 6328

/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */
#define gMaxServicesCount_d 6
#define gMaxServiceCharCount_d 6

/* Enable Serial Manager interface */
#define gAppUseSerialManager_c 0

/* Enable 5.0 optional features */
#define gBLE50_d 1

/* Enable 5.1 optional features */
#define gBLE51_d 1

/* Enable 5.2 optional features */
#define gBLE52_d 1

/* Enable EATT */
#define gEATT_d 1

/* Enable Dynamic GATT database */
#define gGattDbDynamic_d 1

/*! *********************************************************************************
 *     BLE LL Configuration
 ***********************************************************************************/
/*  ble_ll_config.h file lists the parameters with their default values. User can override
 *    the parameter here by defining the parameter to a user defined value. */

#define gAppExtAdvEnable_d 1
#define gLlScanPeriodicAdvertiserListSize_c (8U)
/* disable autonomous feature exchange */
#define gL1AutonomousFeatureExchange_d 0

/*
 * Specific configuration of LL pools by block size and number of blocks for this application.
 * Optimized using the MEM_OPTIMIZE_BUFFER_POOL feature in MemManager,
 * we find that the most optimized combination for LL buffers.
 *
 * If LlPoolsDetails_c is not defined, default LL buffer configuration in app_preinclude_common.h
 * will be applied.
 */

/* Include common configuration file and board configuration file */
#include "app_preinclude_common.h"

#ifdef __cplusplus
}
#endif

#endif /* _APP_PREINCLUDE_H_ */
