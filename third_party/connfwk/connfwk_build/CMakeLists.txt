# Copyright 2022-2023 NXP
# All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.21)

enable_language(C CXX ASM)
set(CMAKE_C_STANDARD 99)

project(connfwk VERSION 6.1.0)

message(STATUS "************ CONNFWK CONFIGURATION ************")
message(STATUS "CONNFWK_PLATFORM: " ${CONNFWK_PLATFORM})

# The user must fill the connfwk-config with SDK infos (paths, compile options, definitions...)
# The framework will fill it with its own infos
add_library(connfwk-config INTERFACE)

# CONNFWK_BOARD_LIB_EXTERNAL gives the possibility to link to an externally built board lib
if(NOT CONNFWK_BOARD_LIB_EXTERNAL)
    if(NOT CONNFWK_FREERTOS_CONFIG_PATH)
        set(CONNFWK_FREERTOS_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${CONNFWK_BOARD}/freertos)
    endif()

    set(CONNFWK_BOARD_LIB connfwk-board-${CONNFWK_BOARD})
else()
    set(CONNFWK_BOARD_LIB ${CONNFWK_BOARD_LIB_EXTERNAL})
endif()

message(STATUS "CONNFWK_BOARD_LIB: ${CONNFWK_BOARD_LIB}")

# CONNFWK_MCUX_SDK_LIB_EXTERNAL gives the possibility to link to an externally built sdk lib
if(NOT CONNFWK_MCUX_SDK_LIB_EXTERNAL)
    set(CONNFWK_MCUX_SDK_LIB mcux-sdk-${CONNFWK_PLATFORM})
else()
    set(CONNFWK_MCUX_SDK_LIB ${CONNFWK_MCUX_SDK_LIB_EXTERNAL})
endif()

message(STATUS "CONNFWK_MCUX_SDK_LIB: ${CONNFWK_MCUX_SDK_LIB}")

set(CONNFWK_PLATFORM_LIB connfwk-platform-${CONNFWK_PLATFORM})
message(STATUS "CONNFWK_PLATFORM_LIB: ${CONNFWK_PLATFORM_LIB}")

# Set common paths for the framework
target_include_directories(connfwk-config INTERFACE
    ${CONNFWK_SDK_PATH}/Common
    ${CONNFWK_SDK_PATH}/services/FunctionLib
    ${CONNFWK_SDK_PATH}/services/FileSystem
)

# Configure the platform lib
add_subdirectory(${CONNFWK_PLATFORM} ${PROJECT_BINARY_DIR}/framework/platform/${CONNFWK_PLATFORM})

if(CONNFWK_PLATFORM_FAMILY)
    target_include_directories(connfwk-config INTERFACE
        ${CONNFWK_SDK_PATH}/platform/${CONNFWK_PLATFORM_FAMILY}/${CONNFWK_PLATFORM}
    )
    add_subdirectory(${CONNFWK_PLATFORM_FAMILY} ${PROJECT_BINARY_DIR}/framework/platform/${CONNFWK_PLATFORM_FAMILY})
endif()

if(CONNFWK_FLIB)
    set(FLIB_SOURCES
        ${CONNFWK_SDK_PATH}/services/FunctionLib/FunctionLib.c
    )

    add_library(connfwk-FunctionLib ${FLIB_SOURCES})

    target_include_directories(connfwk-FunctionLib
        PUBLIC
        ${CONNFWK_SDK_PATH}/services/FunctionLib
    )

    # Get common configs from the connfwk-config interface
    target_link_libraries(connfwk-FunctionLib PRIVATE connfwk-config ${CONNFWK_BOARD_LIB}
        ${CONNFWK_MCUX_SDK_LIB})
endif()

if(CONNFWK_FILESYSTEM)
    set(FW_SOURCES
        ${CONNFWK_SDK_PATH}/services/FileSystem/fwk_filesystem.c

        # temporary hack until the module is deprecated by FSA
        # do not include fwk_platform_flash.c from platform lib as the content of the file
        # has been moved to fwk_lfs_mflash.c
        # However, to keep current compatibility until FSA is enabled, do as it is part of this module
        ${CONNFWK_SDK_PATH}/platform/${CONNFWK_PLATFORM}/fwk_platform_flash.c
    )
    add_library(connfwk-FileSystem ${FW_SOURCES})

    target_include_directories(connfwk-FileSystem
        PUBLIC
        ${CONNFWK_SDK_PATH}/services/FileSystem/
    )

    # Get common configs from the connfwk-config interface
    target_link_libraries(connfwk-FileSystem
        PRIVATE
        connfwk-config
        ${CONNFWK_PLATFORM_LIB}
        ${CONNFWK_BOARD_LIB}
        ${CONNFWK_MCUX_SDK_LIB}
    )
endif()

if(CONNFWK_OTW)
    list(APPEND OTW_INCLUDES
        ${CONNFWK_SDK_PATH}/services/OTW/Interface
    )

    # Check if the k32w0 transceiver is enabled
    string(FIND ${CONNFWK_TRANSCEIVER} "k32w0" outString)

    if(${outString} GREATER_EQUAL 0)
        list(APPEND OTW_SOURCES
            ${CONNFWK_SDK_PATH}/services/OTW/k32w0_transceiver/fwk_otw.c
        )
    else()
        message(FATAL_ERROR "No implementation for OTW")
    endif()

    add_library(connfwk-OTW ${OTW_SOURCES})

    # Making those includes PUBLIC will share them to the other targets linking this lib
    target_include_directories(connfwk-OTW PUBLIC ${OTW_INCLUDES})

    target_compile_definitions(connfwk-OTW PRIVATE ${COMPILE_PRIVATE_DEFINITIONS})

    # Get common configs from the connfwk-config interface
    target_link_libraries(connfwk-OTW
        PRIVATE
        connfwk-config
        ${CONNFWK_MCUX_SDK_LIB}
    )
endif()