#
#  Copyright (c) 2024, The OpenThread Authors.
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

list(APPEND OT_PLATFORM_DEFINES
    "OPENTHREAD_CORE_CONFIG_PLATFORM_CHECK_FILE=\"openthread-core-mcxw72-config-check.h\""
)

set(OT_PLATFORM_DEFINES ${OT_PLATFORM_DEFINES} PARENT_SCOPE)

set(OT_PUBLIC_INCLUDES ${OT_PUBLIC_INCLUDES} PARENT_SCOPE)

set(COMM_FLAGS
    -I${PROJECT_SOURCE_DIR}/examples/mcxw/mcxw72/
)
#if(OT_CFLAGS MATCHES "-pedantic-errors")
#    string(REPLACE "-pedantic-errors" "" OT_CFLAGS "${OT_CFLAGS}")
#endif()

#if(OT_CFLAGS MATCHES "-Wcast-align")
#    string(REPLACE "-Wcast-align" "" OT_CFLAGS "${OT_CFLAGS}")
#endif()

add_library(openthread-mcxw72
    ${MCXW72_SOURCES}
    $<TARGET_OBJECTS:openthread-platform-utils>
)

set_target_properties(openthread-mcxw72
    PROPERTIES
        C_STANDARD 99
        CXX_STANDARD 11
)


if (USE_NBU)
    SET (MCXW72_LINKER_FILE
        # TODO: use the stadard linker file -> modify the existing one and submit upstream PR
        -T${PROJECT_SOURCE_DIR}/src/k32w1/k32w1/K32W1480_connectivity.ld
)
else()
    SET (MCXW72_LINKER_FILE
        -T$ENV{NXP_MCXW72_SDK_ROOT}/middleware/wireless/framework/Common/devices/kw47_mcxw72x/gcc/connectivity.ld
)
endif()

if (USE_NBU)
    SET (NBU_BINARY_IMAGE_PATH
        $ENV{NXP_MCXW72_SDK_ROOT}/${MIDLWR_DIR}/wireless/ieee-802.15.4/bin/kw47/

)
endif()

target_link_libraries(ot-config
    INTERFACE
    ${NXP_DRIVER_LIB}
)

if (USE_NBU)
target_link_libraries(openthread-mcxw72
    PUBLIC
        ${OT_MBEDTLS}
        -L${PROJECT_SOURCE_DIR}/src/mcxw/mcxw72
        -L${NBU_BINARY_IMAGE_PATH}
        -L$ENV{NXP_MCXW72_SDK_ROOT}/middleware/wireless/framework/Common/devices/kw47_mcxw72x/gcc
        ${MCXW72_LINKER_FILE}
        -Wl,--gc-sections,--defsym=gUseNVMLink_d=1
        -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_PROPERTY:NAME>.map,-print-memory-usage
    PRIVATE
        ${NXP_DRIVER_LIB}
        ot-config
)
else()
target_link_libraries(openthread-mcxw72
    PUBLIC
        ${OT_MBEDTLS}
        ${MCXW72_LINKER_FILE}
        -Wl,--gc-sections,--defsym=gUseNVMLink_d=1
        -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_PROPERTY:NAME>.map,-print-memory-usage
    PRIVATE
        ${NXP_DRIVER_LIB}
        ot-config
)
endif()

target_compile_definitions(openthread-mcxw72
    PUBLIC
        ${OT_PLATFORM_DEFINES}
)

target_compile_options(openthread-mcxw72
    PUBLIC
        ${OT_CFLAGS}
        ${COMM_FLAGS}
)

target_include_directories(openthread-mcxw72
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/mcxw72
        ${MCXW72_INCLUDES}
        ${OT_PUBLIC_INCLUDES}
)

target_include_directories(ot-config INTERFACE ${OT_PUBLIC_INCLUDES})
target_compile_definitions(ot-config INTERFACE ${OT_PLATFORM_DEFINES})
