set(SOURCES
    ${CONNFWK_SDK_PATH}/platform/rt1060/fwk_platform.c
    ${CONNFWK_SDK_PATH}/platform/rt1060/fwk_platform_flash.c
)

add_library(${CONNFWK_PLATFORM_LIB} ${SOURCES})

# Making those includes PUBLIC will share them to the other targets linking this lib
target_include_directories(${CONNFWK_PLATFORM_LIB}
    PUBLIC
    ${CONNFWK_SDK_PATH}/platform/include
    ${CONNFWK_SDK_PATH}/platform/rt1060
)

# Get common configs from the connfwk-config interface
target_link_libraries(${CONNFWK_PLATFORM_LIB}
    PRIVATE
    connfwk-config
    connfwk-platform-${CONNFWK_PLATFORM_FAMILY}
    PUBLIC
    ${CONNFWK_MCUX_SDK_LIB}
)