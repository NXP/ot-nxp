set(ZEPHYR_CONN_FRAMEWORK_FILES_PATH ${CONNFWK_SDK_PATH}/zephyr)

set(SOURCES
    ${CONNFWK_SDK_PATH}/platform/rt1060/fwk_platform.c
    ${CONNFWK_SDK_PATH}/platform/rt1060/fwk_platform_extflash.c

    #NVS
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/port/nvs/fwk_nvs_flash.c
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/port/nvs/fwk_nvs_ExternalFlash.c
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/subsys/fs/nvs/nvs.c
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/subsys/settings/src/settings.c
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/subsys/settings/src/settings_init.c
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/subsys/settings/src/settings_store.c
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/subsys/settings/src/settings_nvs.c
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/lib/crc/crc8_sw.c
)

add_library(${CONNFWK_PLATFORM_LIB} ${SOURCES})

# Making those includes PUBLIC will share them to the other targets linking this lib
target_include_directories(${CONNFWK_PLATFORM_LIB}
    PUBLIC
    ${CONNFWK_SDK_PATH}/platform/include
    ${CONNFWK_SDK_PATH}/platform/rt1060
    ${CONNFWK_SDK_PATH}/platform/rt1060/configs
    ${CONNFWK_SDK_PATH}/platform/Common

    #NVS
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/include
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/include/zephyr/fs
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/include/zephyr/settings
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/include/zephyr/sys
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/port/common
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/port/nvs
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/port/settings
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/subsys/fs/nvs
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/subsys/settings/include
    ${ZEPHYR_CONN_FRAMEWORK_FILES_PATH}/subsys/settings/include/settings
)

# Get common configs from the connfwk-config interface
target_link_libraries(${CONNFWK_PLATFORM_LIB}
    PRIVATE
    connfwk-config
    connfwk-platform-${CONNFWK_PLATFORM_FAMILY}
    PUBLIC
    ${CONNFWK_MCUX_SDK_LIB}
)