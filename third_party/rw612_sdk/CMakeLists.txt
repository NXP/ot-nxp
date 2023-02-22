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
    list(APPEND RW612_SDK_SOURCES
        # els_pkc component for mbedtls els_pkc port
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/platforms/redfinchSdk_sample/mcux_els.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/platforms/redfinchSdk_sample/mcux_pkc.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/src/mcuxClEls_Aead.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/src/mcuxClEls_Cipher.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/src/mcuxClEls_Cmac.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/src/mcuxClEls_Common.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/src/mcuxClEls_Crc.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/src/mcuxClEls_Ecc.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/src/mcuxClEls_Hash.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/src/mcuxClEls_Hmac.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/src/mcuxClEls_Kdf.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/src/mcuxClEls_KeyManagement.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/src/mcuxClEls_Rng.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClHash/src/mcuxClHash_api_multipart_common.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClHash/src/mcuxClHash_api_multipart_compute.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClHash/src/mcuxClHash_api_oneshot_compute.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClHash/src/mcuxClHash_core_els_sha2.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClHash/src/mcuxClHash_internal_els_sha2.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClSession/src/mcuxClSession.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMemory/src/mcuxClMemory.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxCsslMemory/src/mcuxCsslMemory_Clear.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxCsslMemory/src/mcuxCsslMemory_Compare.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxCsslMemory/src/mcuxCsslMemory_Copy.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxCsslMemory/src/mcuxCsslMemory_Set.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxCsslParamIntegrity/src/mcuxCsslParamIntegrity.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRandom/src/mcuxClRandom_DRBG.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRandom/src/mcuxClRandom_PRNG.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRandomModes/src/mcuxClRandomModes_CtrDrbg.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRandomModes/src/mcuxClRandomModes_CtrDrbg_Els.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRandomModes/src/mcuxClRandomModes_CtrDrbg_PrDisabled.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRandomModes/src/mcuxClRandomModes_ElsMode.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRandomModes/src/mcuxClRandomModes_NormalMode.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRandomModes/src/mcuxClRandomModes_PrDisabled.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAes/src/mcuxClAes_KeyTypes.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClKey/src/mcuxClKey.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClKey/src/mcuxClKey_KeyTypes.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClKey/src/mcuxClKey_Protection.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Constants.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_EdDSA_GenerateKeyPair.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_EdDSA_GenerateKeyPair_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_EdDSA_GenerateSignature.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_EdDSA_Internal_SetupEnvironment.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_EdDSA_VerifySignature.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_BlindedScalarMult.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_Convert_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_GenerateMultiplicativeBlinding.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_InterleaveTwoScalars.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_Interleave_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_RecodeAndReorderScalar.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_SetupEnvironment.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_SetupEnvironment_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Internal_Types.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_KeyTypes.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_DhKeyAgreement.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_DhKeyGeneration.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_Internal_DhSetupEnvironment.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_Internal_MontDhX.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_Internal_MontDhX_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_Internal_SecureScalarMult_XZMontLadder.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Mont_Internal_SecureScalarMult_XZMontLadder_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_SignatureMechanisms.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_FixScalarMult.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PlainFixScalarMult25519.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PlainPtrSelectComb.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PlainPtrSelectML.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PlainVarScalarMult.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PointArithmeticEd25519.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PointArithmeticEd25519_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PointValidation_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_PrecPointImportAndValidate.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_VarScalarMult.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_TwEd_Internal_VarScalarMult_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_WeierECC_Internal_GenerateCustomKeyType.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_WeierECC_Internal_GenerateDomainParams.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_WeierECC_Internal_GenerateDomainParams_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_WeierECC_Internal_SetupEnvironment.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_ConvertPoint_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_KeyGen.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_KeyGen_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_PointArithmetic.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_PointArithmetic_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_PointCheck.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_PointCheck_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_PointMult.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_SecurePointMult_CoZMontLadder.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_SecurePointMult_CoZMontLadder_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Internal_SetupEnvironment.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_KeyGen.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_PointMult.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_PointMult_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Sign.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Sign_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Verify.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/src/mcuxClEcc_Weier_Verify_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClPrng/src/mcuxClPrng_ELS.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_ExactDivide.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_ExactDivideOdd.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_ExactDivideOdd_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_ModExp_SqrMultL2R.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_ModInv.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_ModInv_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_NDash.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_NDash_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_QDash.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_QDash_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_ReduceModEven.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_SecModExp.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_SecModExp_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/src/mcuxClMath_Utils.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClPkc/src/mcuxClPkc_Calculate.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClPkc/src/mcuxClPkc_ImportExport.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClPkc/src/mcuxClPkc_Initialize.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClPkc/src/mcuxClPkc_UPTRT.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_ComputeD.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_ComputeD_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_GenerateProbablePrime.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_KeyGeneration_Crt.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_KeyGeneration_Crt_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_KeyGeneration_Plain.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_Mgf1.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_MillerRabinTest.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_MillerRabinTest_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_ModInv.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_NoEncode.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_NoVerify.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_Pkcs1v15Encode_sign.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_Pkcs1v15Verify.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_PrivateCrt.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_PrivateCrt_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_PrivatePlain.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_PrivatePlain_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_PssEncode.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_PssVerify.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_Public.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_Public_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_Sign.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_TestPQDistance.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_TestPQDistance_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_TestPrimeCandidate.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_TestPrimeCandidate_FUP.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_Verify.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/src/mcuxClRsa_VerifyE.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMacModes/src/mcuxClMacModes.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMacModes/src/mcuxClMacModes_ELS_Aes.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMacModes/src/mcuxClMacModes_ELS_CBCMAC.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMacModes/src/mcuxClMacModes_ELS_CMAC.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMacModes/src/mcuxClMacModes_ELS_HMAC.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMacModes/src/mcuxClMacModes_Helper.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMacModes/src/mcuxClMacModes_Modes.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMac/src/mcuxClMac.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClPadding/src/mcuxClPadding.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_AesCcm.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_AesGcm.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_CcmEngineAes.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_Constants.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_GcmEngineAes.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_MultiPart.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAeadModes/src/mcuxClAeadModes_ELS_OneShot.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAead/src/mcuxClAead.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClCipherModes/src/mcuxClCipherModes_ELS_Aes.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClCipherModes/src/mcuxClCipherModes_ELS_EngineAes.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClCipherModes/src/mcuxClCipherModes_Helper.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClCipherModes/src/mcuxClCipherModes_Modes.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClCipher/src/mcuxClCipher.c
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClTrng/src/mcuxClTrng_RNG4.c
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
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/platforms/redfinchSdk_sample
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/platforms/redfinchSdk_sample/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/inc/impl
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEls/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClHash/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClSession/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClSession/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxCsslFlowProtection/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClCore/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxCsslSecureCounter/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxCsslCPreProcessor/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMemory/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxCsslMemory/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxCsslParamIntegrity/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/compiler
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRandom/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRandom/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRandomModes/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRandomModes/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAes/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAes/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClKey/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClEcc/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClTrng/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClTrng/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClPrng/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClPrng/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMath/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClPkc/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClRsa/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMacModes/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMacModes/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMac/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClMac/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClPadding/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClPadding/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAeadModes/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAeadModes/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAead/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClAead/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClCipherModes/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClCipherModes/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClCipher/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClCipher/inc/internal
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClTrng/inc
        $ENV{NXP_RW612_SDK_ROOT}/components/els_pkc_ear4/src/comps/mcuxClTrng/inc/internal
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
    -DSERIAL_PORT_TYPE_UART=1
    -DSERIAL_MANAGER_NON_BLOCKING_MODE=1
    -DSERIAL_MANAGER_TASK_STACK_SIZE=1024
    -DSERIAL_MANAGER_TASK_PRIORITY=3
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
# Select RW610 platform
set(CONNFWK_PLATFORM rdrw610)
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
