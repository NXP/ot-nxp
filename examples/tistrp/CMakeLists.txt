#
#  Copyright 2026 NXP
#  All rights reserved.
#
#  SPDX-License-Identifier: BSD-3-Clause
#

target_sources(ot-cli-addons PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/tistrp_cli.c
)

target_include_directories(ot-cli-addons PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(ot-cli-addons PRIVATE
    -DOT_APP_CLI_TISTRP_ADDON
)

if(OT_APP_TISTRP_AUTOSTART)
    target_compile_definitions(ot-cli-addons PRIVATE -DOT_APP_TISTRP_AUTOSTART=1)
endif()
