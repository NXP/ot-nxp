/*! *********************************************************************************
 * \defgroup SHELL GATTDB
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2019-2021, 2023 NXP
 *
 *
 * \file
 *
 * This file is the interface file for the GATTDB Shell module
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ********************************************************************************** */

#ifndef SHELL_GATTDB_H
#define SHELL_GATTDB_H

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/
#define mMaxCharValueLength_d 23U
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*! *********************************************************************************
 * \brief        Entry point for "gatt" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
shell_status_t ShellGattDb_Command(shell_handle_t shellHandle, int32_t argc, char **argv);

/*! *********************************************************************************
 * \brief        Initializes the GATT Dynamic Database with default services.
 *
 * \param[in]    none
 *
 * \return       bleResult_t
 ********************************************************************************** */
bleResult_t ShellGattDb_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* SHELL_GATTDB_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */