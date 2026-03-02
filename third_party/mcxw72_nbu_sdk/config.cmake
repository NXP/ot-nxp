#
# Copyright (c) 2025, The OpenThread Authors.
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

# for framework.sfc
mcux_add_include(
    BASE_PATH ${SdkRootDirPath}/middleware/wireless/ble_controller
    INCLUDES
            lib/mll_inc
            src/KW4x
)

mcux_remove_armgcc_linker_script(
    BASE_PATH ${SdkRootDirPath}
    TARGETS debug release
    LINKER ${device_root}/${soc_portfolio}/${soc_series}/${device}/gcc/${CONFIG_MCUX_TOOLCHAIN_LINKER_DEVICE_PREFIX}_flash.ld
)

mcux_add_armgcc_linker_script(
    BASE_PATH ${MAKE_CURRENT_LIST_DIR}
    TARGETS debug release
    LINKER mcxw72_nbu.ld
)

# for fwk_platform_genfsk.c
mcux_remove_macro(
    gPlatformEnableDcdcOnNbu_d=1
)

mcux_add_configuration(
    CC "${OT_CFLAGS}"
    CC "-Wno-error -Wno-implicit-function-declaration -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-empty-body -Wno-int-conversion -Wno-int-in-bool-context -Wno-memset-elt-size -Wno-parentheses"
)

if (OT_APP_LOWPOWER)
    target_compile_definitions(${OT_MCUX_SDK_TARGET} PUBLIC
        gAppLowpowerEnabled_d=1
    )
else()
    target_compile_definitions(${OT_MCUX_SDK_TARGET} PUBLIC
        gAppLowpowerEnabled_d=0
    )
endif()

target_compile_definitions(${OT_MCUX_SDK_TARGET} PUBLIC
    USE_NBU=${USE_NBU}
    HAL_RPMSG_SELECT_ROLE=1
    LPTMR_USE_FREE_RUNNING=1
    FSL_OSA_TASK_ENABLE=1
    gMemManagerLightExtendHeapAreaUsage=1
    SERIAL_MANAGER_NON_BLOCKING_MODE=1
    gPlatformUseLptmr_d=1
)

target_compile_options(${OT_MCUX_SDK_TARGET} PUBLIC
    -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-function -Wno-unused-parameter -Wno-empty-body -Wno-missing-field-initializers -Wno-clobbered -fno-strict-aliasing
)
