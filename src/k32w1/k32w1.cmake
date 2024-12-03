#
#  Copyright (c) 2021, The OpenThread Authors.
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
    "OPENTHREAD_CORE_CONFIG_PLATFORM_CHECK_FILE=\"openthread-core-${OT_NXP_PLATFORM}-config-check.h\""
)

set(OT_PLATFORM_DEFINES ${OT_PLATFORM_DEFINES} PARENT_SCOPE)

set(OT_PUBLIC_INCLUDES ${OT_PUBLIC_INCLUDES} PARENT_SCOPE)

# Propagate SDK deps to platform target
get_target_property(SdkIncludeDirs ${MCUX_SDK_PROJECT_NAME} INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND OT_PUBLIC_INCLUDES ${SdkIncludeDirs})
get_target_property(SdkCompileDefinitions ${MCUX_SDK_PROJECT_NAME} INTERFACE_COMPILE_DEFINITIONS)
list(APPEND OT_PLATFORM_DEFINES ${SdkCompileDefinitions})

set(COMM_FLAGS
    -I${PROJECT_SOURCE_DIR}/examples/k32w1/
)
#if(OT_CFLAGS MATCHES "-pedantic-errors")
#    string(REPLACE "-pedantic-errors" "" OT_CFLAGS "${OT_CFLAGS}")
#endif()

#if(OT_CFLAGS MATCHES "-Wcast-align")
#    string(REPLACE "-Wcast-align" "" OT_CFLAGS "${OT_CFLAGS}")
#endif()

add_library(openthread-${OT_NXP_PLATFORM}
    ${OT_NXP_PLATFORM_SOURCES}
    $<TARGET_OBJECTS:openthread-platform-utils>
    $<TARGET_OBJECTS:${MCUX_SDK_PROJECT_NAME}>
)

set_target_properties(openthread-${OT_NXP_PLATFORM}
    PROPERTIES
        C_STANDARD 99
        CXX_STANDARD 11
)

if (USE_NBU)
target_link_libraries(openthread-${OT_NXP_PLATFORM}
    PUBLIC
        ${OT_MBEDTLS}
        -L${PROJECT_SOURCE_DIR}/src/k32w1
        -L${SdkRootDirPath}/examples/_boards/${board}/wireless_examples/linker/gcc
        -Wl,--gc-sections,--defsym=gUseNVMLink_d=1
        -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_PROPERTY:NAME>.map,-print-memory-usage
    PRIVATE
        ot-config
)
else()
target_link_libraries(openthread-${OT_NXP_PLATFORM}
    PUBLIC
        ${OT_MBEDTLS}
        -Wl,--gc-sections,--defsym=gUseNVMLink_d=1
        -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_PROPERTY:NAME>.map,-print-memory-usage
    PRIVATE
        ot-config
)
endif()

target_compile_definitions(openthread-${OT_NXP_PLATFORM}
    PUBLIC
        ${OT_PLATFORM_DEFINES}
)

target_compile_options(openthread-${OT_NXP_PLATFORM}
    PUBLIC
        ${OT_CFLAGS}
        ${COMM_FLAGS}
)

target_include_directories(openthread-${OT_NXP_PLATFORM}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OT_NXP_PLATFORM_INCLUDES}
        ${OT_PUBLIC_INCLUDES}
)

target_include_directories(ot-config INTERFACE ${OT_PUBLIC_INCLUDES})
