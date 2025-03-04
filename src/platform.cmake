#
#  Copyright (c) 2024-2025, The OpenThread Authors.
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

get_filename_component(OT_NXP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../ REALPATH)

set(OT_NXP_PLATFORM_SOURCES "")

if (OT_NXP_PLATFORM MATCHES "^(mcxw71|mcxw72)$" OR
    MCUX_HW_DEVICE_MCXW716C OR
    MCUX_HW_DEVICE_MCXW727C
)
  list(APPEND OT_NXP_PLATFORM_SOURCES
      ${OT_NXP_ROOT}/src/${OT_NXP_PLATFORM_FAMILY}/platform/alarm.c
      ${OT_NXP_ROOT}/src/${OT_NXP_PLATFORM_FAMILY}/platform/diag.c
      ${OT_NXP_ROOT}/src/${OT_NXP_PLATFORM_FAMILY}/platform/entropy.c
      ${OT_NXP_ROOT}/src/${OT_NXP_PLATFORM_FAMILY}/platform/logging.c
      ${OT_NXP_ROOT}/src/${OT_NXP_PLATFORM_FAMILY}/platform/misc.c
      ${OT_NXP_ROOT}/src/${OT_NXP_PLATFORM_FAMILY}/platform/radio.c
      ${OT_NXP_ROOT}/src/${OT_NXP_PLATFORM_FAMILY}/platform/system.c
      ${OT_NXP_ROOT}/src/${OT_NXP_PLATFORM_FAMILY}/platform/uart.c
      ${OT_NXP_ROOT}/src/common/flash_nvm.c
      ${OT_NXP_ROOT}/openthread/examples/apps/cli/cli_uart.cpp
  )
endif()

