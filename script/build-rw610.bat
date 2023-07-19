@echo off

if "%NXP_RW610_SDK_ROOT%" == "" (
	echo Variable NXP_RW610_SDK_ROOT is not set.
	echo Please set it to the location of the SDK and then re-build.
	goto :end
)

if not exist %NXP_RW610_SDK_ROOT% (
	echo '%NXP_RW610_SDK_ROOT%' is not a valid path.
	echo 'Please set NXP_RW610_SDK_ROOT to a valid path pointing to the SDK'
	goto :end
)


if exist %NXP_RW610_SDK_ROOT%\SW-Content-Register.txt (
	set SDK_RELEASE=ON
) else if exist %NXP_RW610_SDK_ROOT%\.gitmodules (
	set SDK_RELEASE=OFF
) else (
	echo Could not find a valid SDK at %NXP_RW610_SDK_ROOT%
	goto :end
)

echo NXP_RW610_SDK_ROOT set to %NXP_RW610_SDK_ROOT%
echo SDK_RELASE set to %SDK_RELEASE%

set OT_OPTIONS=-DCMAKE_TOOLCHAIN_FILE=src/rw/rw610/arm-none-eabi.cmake^
 -DCMAKE_BUILD_TYPE=Debug^
 -DOT_PLATFORM=external^
 -DOT_CSL_RECEIVER=OFF^
 -DOT_SLAAC=ON^
 -DOT_BUILD_RW=ON^
 -DOT_BUILD_RW610=ON^
 -DOT_APP_CLI_FREERTOS=ON^
 -DOT_SETTINGS_RAM=ON^
 -DOT_STACK_ENABLE_LOG=OFF^
 -DOT_APP_NCP=OFF^
 -DOT_APP_RCP=OFF^
 -DOT_RCP=OFF^
 -DOT_APP_CLI=OFF^
 -DOT_COMPILE_WARNING_ASS_ERROR=ON^
 -DOT_THREAD_VERSION=1.1^
 -DSDK_RELEASE=%SDK_RELEASE%
echo OT_OPTIONS %OT_OPTIONS%

set OT_SRCDIR=%CD%

if exist build_rw610 (
	rmdir build_rw610 \s
)

mkdir build_rw610
cd build_rw610

cmake -GNinja -DUSE_NBU=0 -DOT_COMPILE_WARNING_AS_ERROR=ON %OT_OPTIONS% %OT_SRCDIR%
ninja

cd examples\\cli\\rtos
arm-none-eabi-objcopy -O srec ot-cli-rw610 ot-cli-rw610.srec
arm-none-eabi-objcopy -O binary ot-cli-rw610 ot-cli-rw610.bin

cd %OT_SRCDIR%

:end