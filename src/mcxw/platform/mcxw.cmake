#
#  Copyright (c) 2021-2025, The OpenThread Authors.
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

set(OT_PUBLIC_INCLUDES ${OT_PUBLIC_INCLUDES} PARENT_SCOPE)

# Propagate SDK deps to platform target
get_target_property(SdkIncludeDirs ${OT_MCUX_SDK_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND OT_PUBLIC_INCLUDES ${SdkIncludeDirs})
get_target_property(SdkCompileDefinitions ${OT_MCUX_SDK_TARGET} INTERFACE_COMPILE_DEFINITIONS)
list(APPEND OT_PLATFORM_DEFINES ${SdkCompileDefinitions})

set(OT_PLATFORM_DEFINES ${OT_PLATFORM_DEFINES} PARENT_SCOPE)

add_library(${OT_PLATFORM_LIB}
    ${OT_NXP_PLATFORM_SOURCES}
    $<TARGET_OBJECTS:openthread-platform-utils>
)

set_target_properties(${OT_PLATFORM_LIB}
    PROPERTIES
        C_STANDARD 99
        CXX_STANDARD 11
)

target_link_libraries(${OT_PLATFORM_LIB}
    PUBLIC
        ${OT_MCUX_SDK_TARGET}
        -Wl,--gc-sections,--defsym=gUseNVMLink_d=1
        -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_PROPERTY:NAME>.map,-print-memory-usage
    PRIVATE
        ot-config
)

# Openthread libs need to have openthread platform dependencies
target_link_libraries(ot-config
    INTERFACE
        ${OT_PLATFORM_LIB}
)

target_compile_definitions(${OT_PLATFORM_LIB}
    PUBLIC
        ${OT_PLATFORM_DEFINES}
)

target_compile_options(${OT_PLATFORM_LIB}
    PUBLIC
        ${OT_CFLAGS}
)

target_include_directories(${OT_PLATFORM_LIB}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OT_NXP_PLATFORM_INCLUDES}
        ${OT_PUBLIC_INCLUDES}
)

target_include_directories(ot-config INTERFACE ${OT_PUBLIC_INCLUDES})
