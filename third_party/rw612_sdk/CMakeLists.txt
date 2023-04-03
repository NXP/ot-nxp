#
#  Copyright (c) 2021-2023, The OpenThread Authors.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of the copyright holder nor the
#     names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

set_property(SOURCE $ENV{NXP_RW612_SDK_ROOT}/devices/RW612/gcc/startup_RW612.S PROPERTY LANGUAGE C)

set(MIDDLEWARE_PATH $ENV{NXP_RW612_SDK_ROOT}/middleware PARENT_SCOPE)
set(LWIP_PATH $ENV{NXP_RW612_SDK_ROOT}/middleware/lwip PARENT_SCOPE)
set(WIFI_PATH $ENV{NXP_RW612_SDK_ROOT}/middleware/wifi_nxp CACHE PATH "wifi path")
set(WIFI_BT_MODULE_PATH $ENV{NXP_RW612_SDK_ROOT}/components/wifi_bt_module  CACHE PATH "wifi_bt_module path")
set(EDGEFAST_WIFI_PATH $ENV{NXP_RW612_SDK_ROOT}/middleware/edgefast_wifi CACHE PATH "edgefast_wifi path")

if (SDK_RELEASE)
    message("SDK_RELEASE driver path chosen")
    set(DRIVER_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/devices/RW612/drivers)
    set(DRIVER_CACHE_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_FLEXCOMM_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_USART_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_FLEXSPI_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_LPC_GPIO_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_IMU_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_GDMA_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_TRNG_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_COMMON_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_DEVICE_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_MRT_FILES_PATH ${DRIVER_FILES_PATH})
    set(DRIVER_CTIMER_FILES_PATH ${DRIVER_FILES_PATH})
    set(DEBUG_CONSOLE_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/devices/RW612/utilities/debug_console)
    set(STR_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/devices/RW612/utilities/str)
else ()
    message("SDK_DEV driver path chosen")
    set(DRIVER_CACHE_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/cache/cache64)
    set(DRIVER_FLEXCOMM_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/flexcomm)
    set(DRIVER_USART_FILES_PATH ${DRIVER_FLEXCOMM_FILES_PATH}/usart)
    set(DRIVER_FLEXSPI_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/flexspi)
    set(DRIVER_LPC_GPIO_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/lpc_gpio)
    set(DRIVER_IMU_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/imu)
    set(DRIVER_GDMA_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/gdma)
    set(DRIVER_TRNG_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/trng)
    set(DRIVER_COMMON_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/common)
    set(DRIVER_MRT_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/mrt)
    set(DRIVER_CTIMER_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/ctimer)
    set(DRIVER_DEVICE_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/devices/RW612/drivers)
    set(DEBUG_CONSOLE_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/platform/utilities/debug_console)
    set(STR_FILES_PATH $ENV{NXP_RW612_SDK_ROOT}/platform/utilities/str)
endif ()

set(RW612_SDK_SOURCES
#freertos files
    $ENV{NXP_RW612_SDK_ROOT}/rtos/freertos/freertos-kernel/event_groups.c
    $ENV{NXP_RW612_SDK_ROOT}/rtos/freertos/freertos-kernel/croutine.c
    $ENV{NXP_RW612_SDK_ROOT}/rtos/freertos/freertos-kernel/list.c
    $ENV{NXP_RW612_SDK_ROOT}/rtos/freertos/freertos-kernel/portable/GCC/ARM_CM33_NTZ/non_secure/port.c
    $ENV{NXP_RW612_SDK_ROOT}/rtos/freertos/freertos-kernel/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c
    $ENV{NXP_RW612_SDK_ROOT}/rtos/freertos/freertos-kernel/queue.c
    $ENV{NXP_RW612_SDK_ROOT}/rtos/freertos/freertos-kernel/stream_buffer.c
    $ENV{NXP_RW612_SDK_ROOT}/rtos/freertos/freertos-kernel/tasks.c
    $ENV{NXP_RW612_SDK_ROOT}/rtos/freertos/freertos-kernel/timers.c
    $ENV{NXP_RW612_SDK_ROOT}/rtos/freertos/freertos-kernel/portable/MemMang/heap_4.c
#startup files
    $ENV{NXP_RW612_SDK_ROOT}/devices/RW612/gcc/startup_RW612.S
    $ENV{NXP_RW612_SDK_ROOT}/devices/RW612/system_RW612.c
#other SDK files
    ${DRIVER_COMMON_FILES_PATH}/fsl_common.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_clock.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_power.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_reset.c
    ${DRIVER_DEVICE_FILES_PATH}/fsl_ocotp.c
    ${DRIVER_CACHE_FILES_PATH}/fsl_cache.c
    ${DRIVER_FLEXCOMM_FILES_PATH}/fsl_flexcomm.c
    ${DRIVER_USART_FILES_PATH}/fsl_usart.c
    ${DRIVER_FLEXSPI_FILES_PATH}/fsl_flexspi.c
    ${DRIVER_LPC_GPIO_FILES_PATH}/fsl_gpio.c
    ${DRIVER_IMU_FILES_PATH}/fsl_imu.c
    ${DRIVER_GDMA_FILES_PATH}/fsl_gdma.c
    ${DRIVER_TRNG_FILES_PATH}/fsl_trng.c
    ${DEBUG_CONSOLE_FILES_PATH}/fsl_debug_console.c
    ${STR_FILES_PATH}/fsl_str.c
#SDK components
    $ENV{NXP_RW612_SDK_ROOT}/components/serial_manager/fsl_component_serial_manager.c
    $ENV{NXP_RW612_SDK_ROOT}/components/serial_manager/fsl_component_serial_port_uart.c
    $ENV{NXP_RW612_SDK_ROOT}/components/lists/fsl_component_generic_list.c
    $ENV{NXP_RW612_SDK_ROOT}/components/osa/fsl_os_abstraction_free_rtos.c
    $ENV{NXP_RW612_SDK_ROOT}/components/uart/fsl_adapter_usart.c
    $ENV{NXP_RW612_SDK_ROOT}/components/rpmsg/fsl_adapter_rfimu.c
    #$ENV{NXP_RW612_SDK_ROOT}/components/rng/fsl_adapter_software_rng.c
    $ENV{NXP_RW612_SDK_ROOT}/components/flash/mflash/rdrw612bga/mflash_drv.c
    $ENV{NXP_RW612_SDK_ROOT}/components/rng/fsl_adapter_trng.c
    $ENV{NXP_RW612_SDK_ROOT}/components/timer_manager/fsl_component_timer_manager.c
    $ENV{NXP_RW612_SDK_ROOT}/components/timer/fsl_adapter_mrt.c
    $ENV{NXP_RW612_SDK_ROOT}/components/time_stamp/fsl_adapter_ctimer_time_stamp.c
    ${DRIVER_MRT_FILES_PATH}/fsl_mrt.c
    ${DRIVER_CTIMER_FILES_PATH}/fsl_ctimer.c
# LittleFS
    $ENV{NXP_RW612_SDK_ROOT}/middleware/littlefs/lfs.c
    $ENV{NXP_RW612_SDK_ROOT}/middleware/littlefs/lfs_util.c
#Loader files
    $ENV{NXP_RW612_SDK_ROOT}/components/conn_fwloader/fsl_loader.c
    $ENV{NXP_RW612_SDK_ROOT}/components/conn_fwloader/life_cycle.c
    $ENV{NXP_RW612_SDK_ROOT}/components/conn_fwloader/nboot_hal.c
)

if (SDK_RELEASE)
    list(APPEND RW612_SDK_SOURCES
        $ENV{NXP_RW612_SDK_ROOT}/devices/RW612/utilities/format/fsl_format.c
        $ENV{NXP_RW612_SDK_ROOT}/devices/RW612/drivers/fsl_common_arm.c
)
else()
    list(APPEND RW612_SDK_SOURCES
        $ENV{NXP_RW612_SDK_ROOT}/platform/utilities/misc_utilities/fsl_format.c
        $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/common/fsl_common_arm.c
)
endif()

if(OT_NXP_MBEDTLS_PORT)
    set(ELS_PKC_CL_PATH $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc)
    list(APPEND RW612_SDK_SOURCES
        # els_pkc component for mbedtls els_pkc port
        ${ELS_PKC_CL_PATH}/src/platforms/redfinchSdk_sample/mcux_els.c
        ${ELS_PKC_CL_PATH}/src/platforms/redfinchSdk_sample/mcux_pkc.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/src/mcuxClEls_Aead.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/src/mcuxClEls_Cipher.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/src/mcuxClEls_Cmac.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/src/mcuxClEls_Common.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/src/mcuxClEls_Crc.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/src/mcuxClEls_Ecc.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/src/mcuxClEls_Hash.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/src/mcuxClEls_Hmac.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/src/mcuxClEls_Kdf.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/src/mcuxClEls_KeyManagement.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/src/mcuxClEls_Rng.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClHash/src/mcuxClHash_api_multipart_common.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClHash/src/mcuxClHash_api_multipart_compute.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClHash/src/mcuxClHash_api_oneshot_compute.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClHash/src/mcuxClHash_core_els_sha2.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClHash/src/mcuxClHash_internal_els_sha2.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClSession/src/mcuxClSession.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMemory/src/mcuxClMemory.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxCsslMemory/src/mcuxCsslMemory_Clear.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxCsslMemory/src/mcuxCsslMemory_Compare.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxCsslMemory/src/mcuxCsslMemory_Copy.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxCsslMemory/src/mcuxCsslMemory_Set.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxCsslParamIntegrity/src/mcuxCsslParamIntegrity.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRandom/src/mcuxClRandom_DRBG.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRandom/src/mcuxClRandom_PRNG.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRandomModes/src/mcuxClRandomModes_CtrDrbg.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRandomModes/src/mcuxClRandomModes_CtrDrbg_Els.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRandomModes/src/mcuxClRandomModes_CtrDrbg_PrDisabled.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRandomModes/src/mcuxClRandomModes_ElsMode.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRandomModes/src/mcuxClRandomModes_NormalMode.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRandomModes/src/mcuxClRandomModes_PrDisabled.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAes/src/mcuxClAes_KeyTypes.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClKey/src/mcuxClKey.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClKey/src/mcuxClKey_KeyTypes.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClKey/src/mcuxClKey_Protection.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Constants.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_EdDSA_GenerateKeyPair.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_EdDSA_GenerateKeyPair_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_EdDSA_GenerateSignature.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_EdDSA_Internal_SetupEnvironment.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_EdDSA_VerifySignature.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_BlindedScalarMult.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_Convert_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_GenerateMultiplicativeBlinding.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_InterleaveTwoScalars.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_Interleave_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_RecodeAndReorderScalar.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_SetupEnvironment.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_SetupEnvironment_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_Types.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_KeyTypes.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_DhKeyAgreement.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_DhKeyGeneration.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_Internal_DhSetupEnvironment.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_Internal_MontDhX.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_Internal_MontDhX_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_Internal_SecureScalarMult_XZMontLadder.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_Internal_SecureScalarMult_XZMontLadder_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_SignatureMechanisms.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_FixScalarMult.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PlainFixScalarMult25519.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PlainPtrSelectComb.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PlainPtrSelectML.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PlainVarScalarMult.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PointArithmeticEd25519.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PointArithmeticEd25519_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PointValidation_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PrecPointImportAndValidate.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_VarScalarMult.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_VarScalarMult_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_WeierECC_Internal_GenerateCustomKeyType.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_WeierECC_Internal_GenerateDomainParams.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_WeierECC_Internal_GenerateDomainParams_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_WeierECC_Internal_SetupEnvironment.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_ConvertPoint_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_KeyGen.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_KeyGen_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_PointArithmetic.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_PointArithmetic_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_PointCheck.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_PointCheck_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_PointMult.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_SecurePointMult_CoZMontLadder.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_SecurePointMult_CoZMontLadder_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_SetupEnvironment.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_KeyGen.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_PointMult.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_PointMult_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Sign.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Sign_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Verify.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Verify_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClPrng/src/mcuxClPrng_ELS.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_ExactDivide.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_ExactDivideOdd.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_ExactDivideOdd_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_ModExp_SqrMultL2R.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_ModInv.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_ModInv_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_NDash.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_NDash_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_QDash.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_QDash_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_ReduceModEven.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_SecModExp.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_SecModExp_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/src/mcuxClMath_Utils.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClPkc/src/mcuxClPkc_Calculate.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClPkc/src/mcuxClPkc_ImportExport.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClPkc/src/mcuxClPkc_Initialize.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClPkc/src/mcuxClPkc_UPTRT.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_ComputeD.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_ComputeD_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_GenerateProbablePrime.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_KeyGeneration_Crt.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_KeyGeneration_Crt_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_KeyGeneration_Plain.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_Mgf1.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_MillerRabinTest.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_MillerRabinTest_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_ModInv.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_NoEncode.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_NoVerify.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_Pkcs1v15Encode_sign.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_Pkcs1v15Verify.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_PrivateCrt.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_PrivateCrt_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_PrivatePlain.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_PssEncode.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_PssVerify.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_Public.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_Public_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_Sign.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_TestPQDistance.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_TestPQDistance_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_TestPrimeCandidate.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_TestPrimeCandidate_FUP.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_Verify.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/src/mcuxClRsa_VerifyE.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMacModes/src/mcuxClMacModes.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMacModes/src/mcuxClMacModes_ELS_Aes.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMacModes/src/mcuxClMacModes_ELS_CBCMAC.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMacModes/src/mcuxClMacModes_ELS_CMAC.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMacModes/src/mcuxClMacModes_ELS_HMAC.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMacModes/src/mcuxClMacModes_Helper.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMacModes/src/mcuxClMacModes_Modes.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMac/src/mcuxClMac.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClPadding/src/mcuxClPadding.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_AesCcm.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_AesGcm.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_CcmEngineAes.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_Constants.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_GcmEngineAes.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_MultiPart.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_OneShot.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAead/src/mcuxClAead.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClCipherModes/src/mcuxClCipherModes_ELS_Aes.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClCipherModes/src/mcuxClCipherModes_ELS_EngineAes.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClCipherModes/src/mcuxClCipherModes_Helper.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClCipherModes/src/mcuxClCipherModes_Modes.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClCipher/src/mcuxClCipher.c
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClTrng/src/mcuxClTrng_RNG4.c
    )
endif()

add_library(${NXP_DRIVER_LIB}
    ${RW612_SDK_SOURCES}
)

set(RW612_SDK_INCLUDES
#freertos includes
        $ENV{NXP_RW612_SDK_ROOT}/rtos/freertos/freertos-kernel/include
        $ENV{NXP_RW612_SDK_ROOT}/rtos/freertos/freertos-kernel/portable/GCC/ARM_CM33_NTZ/non_secure
#CMSIS includes
        $ENV{NXP_RW612_SDK_ROOT}/CMSIS/Core/Include
#other SDK files
        $ENV{NXP_RW612_SDK_ROOT}/devices/RW612/drivers
        $ENV{NXP_RW612_SDK_ROOT}/devices/RW612
        $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/mrt
        $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/ctimer
        ${DRIVER_COMMON_FILES_PATH}
        ${DRIVER_DEVICE_FILES_PATH}
        ${DRIVER_FLEXSPI_FILES_PATH}
        ${DEBUG_CONSOLE_FILES_PATH}
        ${DRIVER_CACHE_FILES_PATH}
        ${DRIVER_FLEXCOMM_FILES_PATH}
        ${DRIVER_USART_FILES_PATH}
        ${DRIVER_LPC_GPIO_FILES_PATH}
        ${DRIVER_IMU_FILES_PATH}
        ${DRIVER_GDMA_FILES_PATH}
        ${DRIVER_TRNG_FILES_PATH}
        ${STR_FILES_PATH}
        $ENV{NXP_RW612_SDK_ROOT}/platform/utilities/misc_utilities
        $ENV{NXP_RW612_SDK_ROOT}/platform/drivers/common
#SDK components
        $ENV{NXP_RW612_SDK_ROOT}/components/uart
        $ENV{NXP_RW612_SDK_ROOT}/components/serial_manager
        $ENV{NXP_RW612_SDK_ROOT}/components/lists
        $ENV{NXP_RW612_SDK_ROOT}/components/osa
        $ENV{NXP_RW612_SDK_ROOT}/components/rpmsg
        $ENV{NXP_RW612_SDK_ROOT}/components/rng
        $ENV{NXP_RW612_SDK_ROOT}/components/flash/mflash
        $ENV{NXP_RW612_SDK_ROOT}/components/flash/mflash/rdrw612bga
        $ENV{NXP_RW612_SDK_ROOT}/components/timer_manager
        $ENV{NXP_RW612_SDK_ROOT}/components/timer
        $ENV{NXP_RW612_SDK_ROOT}/components/time_stamp
# LittleFS
        $ENV{NXP_RW612_SDK_ROOT}/middleware/littlefs
#Loader files
        $ENV{NXP_RW612_SDK_ROOT}/components/conn_fwloader/include
)

if(OT_NXP_MBEDTLS_PORT)
    list(APPEND RW612_SDK_INCLUDES
        # els_pkc component for mbedtls els_pkc port
        ${ELS_PKC_CL_PATH}
        ${ELS_PKC_CL_PATH}/src/platforms/redfinchSdk_sample
        ${ELS_PKC_CL_PATH}/src/platforms/redfinchSdk_sample/inc
        ${ELS_PKC_CL_PATH}/src/inc
        ${ELS_PKC_CL_PATH}/src/inc/impl
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEls/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClHash/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClSession/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClSession/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxCsslFlowProtection/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClCore/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxCsslSecureCounter/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxCsslCPreProcessor/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMemory/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxCsslMemory/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxCsslParamIntegrity/inc
        ${ELS_PKC_CL_PATH}/src/compiler
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRandom/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRandom/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRandomModes/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRandomModes/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAes/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAes/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClKey/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClEcc/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClTrng/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClTrng/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClPrng/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClPrng/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMath/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClPkc/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClRsa/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMacModes/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMacModes/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMac/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClMac/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClPadding/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClPadding/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAeadModes/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAeadModes/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAead/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClAead/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClCipherModes/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClCipherModes/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClCipher/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClCipher/inc/internal
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClTrng/inc
        ${ELS_PKC_CL_PATH}/src/comps/mcuxClTrng/inc/internal
    )
endif()

if (SDK_RELEASE)
    list(APPEND RW612_SDK_INCLUDES
        $ENV{NXP_RW612_SDK_ROOT}/devices/RW612/utilities/format
)
else()
    list(APPEND RW612_SDK_INCLUDES
        $ENV{NXP_RW612_SDK_ROOT}/platform/utilities/misc_utilities
)
endif()

#freertos
target_include_directories(${NXP_DRIVER_LIB}
    PUBLIC
    ${RW612_SDK_INCLUDES}
    ${OT_NXP_FREERTOS_CONFIG_PATH}
)

target_compile_definitions(${NXP_DRIVER_LIB}
    PUBLIC
    -DCPU_RW612ETA1I
    -DCPU3
    -D__STARTUP_CLEAR_BSS
    -DSDK_OS_FREE_RTOS
    -DOSA_USED
    -DFSL_OSA_MAIN_FUNC_ENABLE=0
    -DFSL_RTOS_FREE_RTOS
    -DSDK_DEBUGCONSOLE_UART=1
    -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING
    -DSERIAL_PORT_TYPE_UART=1
    #-DSERIAL_MANAGER_NON_BLOCKING_MODE=1
    -DSERIAL_MANAGER_TASK_STACK_SIZE=1024
    -DSERIAL_MANAGER_TASK_PRIORITY=PRIORITY_RTOS_TO_OSA\(2\)
    -DTM_ENABLE_TIME_STAMP=1
    -DIMU_TASK_STACK_SIZE=1024
    -DgNvStorageIncluded_d=1
    -DAPP_FLEXSPI_AMBA_BASE=0x08000000
    -DOT_PLAT_SAVE_NVM_DATA_ON_IDLE=0
    -DgAspCapability_d=1
    -DFFU_CNS_TX_PWR_TABLE_CALIBRATION=1
    -DMBEDTLS_MCUX_ELS_PKC_API # Needed to compile els_pkc port for mbedtls
    -DIMU_TASK_PRIORITY=2 # OSA priority: configMAX_PRIORITIES - (osa_prio) - 2 = FreeRTOS priority
)

target_compile_options(${NXP_DRIVER_LIB}
    PRIVATE
        ${OT_CFLAGS}
        -Wno-unknown-pragmas
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-empty-body
        -Wno-int-conversion
        -Wno-int-in-bool-context
        -Wno-memset-elt-size
        -Wno-parentheses
)

# Use Connectivity Framework CMake build system to build required modules
# Select RW61x platform
set(CONNFWK_PLATFORM rw61x)
# Enable FunctionLib and FileSystem modules
set(CONNFWK_FLIB ON)
set(CONNFWK_FILESYSTEM ON)
# Remove some platform features not needed
set(CONNFWK_IGNORE_PLATFORM_LOWPOWER ON)
set(CONNFWK_IGNORE_PLATFORM_SENSORS ON)
set(CONNFWK_IGNORE_PLATFORM_BLE ON)
# Add the Framework as a subdirectory
add_subdirectory($ENV{NXP_RW612_SDK_ROOT}/middleware/wireless/framework ${PROJECT_BINARY_DIR}/framework)
# Forward SDK and OT include paths, compile definitions, etc... to the framework
target_link_libraries(connfwk-config
    INTERFACE
        ${NXP_DRIVER_LIB}
        ${NXP_BOARD_LIB}
)

target_link_libraries(${NXP_DRIVER_LIB}
    PRIVATE
        ${NXP_BOARD_LIB} # board.h needed by mflash driver TODO: check if the dependency can be removed in this driver
)
