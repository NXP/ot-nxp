#
# Copyright (c) 2024, The OpenThread Authors.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the
# names of its contributors may be used to endorse or promote products
# derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

list(APPEND OT_PLATFORM_DEFINES
    "USE_NBU=${USE_NBU}"
    "SERIAL_USE_CONFIGURE_STRUCTURE=1"
    "SDK_COMPONENT_INTEGRATION=1"
    "SERIAL_PORT_TYPE_UART=1"
    "OT_APP_UART_INSTANCE=1"
    "gSerialManagerMaxInterfaces_c=2"
    "HAL_RPMSG_SELECT_ROLE=0"
    "TM_ENABLE_TIME_STAMP=1"
    "FSL_OSA_TASK_ENABLE=1"
    "gAspCapability_d=1"
    "gNvStorageIncluded_d=1"
    "gUnmirroredFeatureSet_d=1"
    "gNvFragmentation_Enabled_d=1"
    "gAppButtonCnt_c=2"
    "gBleBondIdentityHeaderSize_c=56"
    "gPlatformShutdownEccRamInLowPower=0"
    "gMemManagerLightExtendHeapAreaUsage=1"
    "gAppHighSystemClockFrequency_d=1"
    "DBG_IO_ENABLE"
    "gHwParamsProdDataPlacement_c=gHwParamsProdDataPlacementLegacyMode_c"
    "DEBUG_CONSOLE_TRANSFER_NON_BLOCKING=1"
)

foreach(macro ${OT_PLATFORM_DEFINES})
    mcux_add_macro(${macro})
endforeach()

# Keeping this separate since the mcux_add_macro requires
# special formatting for input strings.
list(APPEND OT_PLATFORM_DEFINES
    "SSS_CONFIG_FILE=\"fsl_sss_config_elemu.h\""
    "SSCP_CONFIG_FILE=\"fsl_sscp_config_elemu.h\""
)

mcux_add_macro(
    SSS_CONFIG_FILE=\\\"fsl_sss_config_elemu.h\\\"
    SSCP_CONFIG_FILE=\\\"fsl_sscp_config_elemu.h\\\"
)

mcux_add_configuration(
    CC "-Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-function -Wno-unused-parameter -Wno-empty-body -Wno-missing-field-initializers -Wno-clobbered -fno-strict-aliasing"
)
