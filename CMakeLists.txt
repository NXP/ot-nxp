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

cmake_minimum_required(VERSION 3.10.2)
project(ot-nxp VERSION 0.2.0)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

include("${PROJECT_SOURCE_DIR}/openthread/etc/cmake/functions.cmake")

set(OT_PACKAGE_VERSION "" CACHE STRING "OpenThread Package Version")
if(OT_PACKAGE_VERSION STREQUAL "")
    ot_git_version(OT_PACKAGE_VERSION)
    message(STATUS "Setting default package version: ${OT_PACKAGE_VERSION}")
endif()
message(STATUS "Package Version: ${OT_PACKAGE_VERSION}")

message(STATUS "NXP Platform Family: ${OT_NXP_PLATFORM_FAMILY}")
message(STATUS "NXP Platform: ${OT_NXP_PLATFORM}")

if(EXISTS ${PROJECT_SOURCE_DIR}/cmake/platform/${OT_NXP_PLATFORM}.cmake)
    include(${PROJECT_SOURCE_DIR}/cmake/platform/${OT_NXP_PLATFORM}.cmake)
endif()

set(OT_PLATFORM_LIB openthread-${OT_NXP_PLATFORM})
set(NXP_DRIVER_LIB nxp-${OT_NXP_PLATFORM}-driver)

#Set default log level if not set
if (OT_STACK_ENABLE_LOG)
    if (NOT OT_LOG_LEVEL)
        set(OT_LOG_LEVEL INFO)
    endif()
    if (NOT OT_LOG_OUTPUT)
        set(OT_LOG_OUTPUT APP)
    endif()
else()
    set(OT_LOG_LEVEL NONE)
    set(OT_LOG_OUTPUT NONE)
endif()

add_subdirectory(openthread)

# If OT logging is enabled add a dependency to SDK log components
if (OT_STACK_ENABLE_LOG)
    target_link_libraries(ot-config
        INTERFACE
        ${NXP_DRIVER_LIB})
    target_compile_definitions(ot-config
        INTERFACE
        -DOT_STACK_ENABLE_LOG)
endif ()

set(CONFIG_FILE "\"openthread-core-${OT_NXP_PLATFORM}-config.h\"")
set(CORE_CONFIG_FILE ${CONFIG_FILE})
set(CORE_CONFIG_CHECK "\"openthread-core-${OT_NXP_PLATFORM}-config-check.h\"")

target_compile_definitions(ot-config INTERFACE
    OPENTHREAD_CONFIG_FILE=${CONFIG_FILE}
    OPENTHREAD_PROJECT_CORE_CONFIG_FILE=${CORE_CONFIG_FILE}
    OPENTHREAD_CORE_CONFIG_PLATFORM_CHECK_FILE=${CORE_CONFIG_CHECK}
)

if(OT_NXP_DISABLE_TCP)
    # Initially used to workaround duplicated symbols in lwip and tcplp stacks
    # In the context of the coex_app_cli, the wifi stack uses lwip while ot uses tcplp
    # Those stacks have some functions name in common (but incompatible)
    # For now, we add the possibility to disable OT TCP feature to avoid using tcplp
    # TODO: can we interface OT over lwip like done in Matter to avoid such problem ?
    target_compile_definitions(ot-config
        INTERFACE
            -DOPENTHREAD_CONFIG_TCP_ENABLE=0
    )
endif()

if(OT_NXP_PLATFORM_FAMILY)
    target_include_directories(ot-config INTERFACE
        ${PROJECT_SOURCE_DIR}/src/${OT_NXP_PLATFORM_FAMILY}/${OT_NXP_PLATFORM}
    )
else()
    target_include_directories(ot-config INTERFACE
        ${PROJECT_SOURCE_DIR}/src/${OT_NXP_PLATFORM}
        )
endif()

# Give the possibility to the user to define his own path to board files
if(OT_NXP_BOARD_PATH)
    message(STATUS  "Using user defined board folder:\n"
        "${OT_NXP_BOARD_PATH}"
    )
    if(NOT NXP_BOARD_LIB)
        message(WARNING  "The name of the board lib is not specified. It may not be linked properly by ot-nxp targets\n"
            "You can add the link dependency in your projet:\n"
            "target_link_libraries(<ot-nxp-target> <your board target>)\n"
            "Or you can define the name of your board library:\n"
            "-DNXP_BOARD_LIB=<name of the board lib>\n"
            "or set it in your project before including ot-nxp:\n"
            "set(NXP_BOARD_LIB <name of the board lib>)"
        )
    endif()
    if((NOT OT_NXP_FREERTOS_CONFIG_PATH) AND (OT_APP_CLI_FREERTOS))
        # When using custom board file, the user must specify path to FreeRTOSConfig.h
        message(FATAL_ERROR  "Path to FreeRTOSConfig.h file not specified. Please specify it using following option:\n"
            "-DOT_NXP_FREERTOS_CONFIG_PATH=<path to FreeRTOSConfig.h folder>"
            "or set it in your project"
        )
    endif()
    add_subdirectory(${OT_NXP_BOARD_PATH} ${PROJECT_BINARY_DIR}/board)
elseif(OT_NXP_BOARD) # TODO: move to "else" when all boards are supported
    if((NOT OT_NXP_FREERTOS_CONFIG_PATH) AND (OT_APP_CLI_FREERTOS))
        set(OT_NXP_FREERTOS_CONFIG_PATH ${PROJECT_SOURCE_DIR}/boards/${OT_NXP_BOARD}/freertos)
    endif()
    set(NXP_BOARD_LIB nxp-${OT_NXP_BOARD}-board)
    add_subdirectory(boards)
else()
    # TODO: enable following message when all boards are supported
    # if(NOT OT_NXP_BOARD)
    #     message(FATAL_ERROR  "Board is not defined. Please specify to board to build:\n"
    #         "-DOT_NXP_BOARD=<board name>"
    #     )
    # endif()
endif()

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(examples)

include(${PROJECT_SOURCE_DIR}/cmake/utils.cmake)

if(OT_NXP_POST_BUILD_COMMAND)
    run_post_build_command()
endif()

if(OT_NXP_EXPORT_TO_BIN)
    export_all_targets_to_bin()
endif()

if(OT_NXP_EXPORT_TO_SREC)
    export_all_targets_to_srec()
endif()
