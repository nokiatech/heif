@echo off
SetLocal EnableDelayedExpansion
set abiList=armeabi-v7a arm64-v8a
set buildType=Debug Release
set apiLevel=android-28

WHERE cmake > nul
IF %ERRORLEVEL% NEQ 0 (
	echo:
	echo "ERROR: CMake not found!"
	echo:
	exit /B 1
)

WHERE ninja > nul
IF %ERRORLEVEL% NEQ 0 (
	echo:
	echo "ERROR: Ninja not found!"
	echo:
	exit /B 1
)

if NOT DEFINED ANDROID_NDK (
	echo:
	echo "ERROR: ANDROID_NDK is not set!"
	echo:
	exit /B 1
) 

if "%1"=="" (
	echo:
 	echo No arguments given! Syntax:
    echo "<script> clean | configure | build | configure_and_build"
	echo:
	exit 1
)
set action=""
call :parsecommand %1
if %action%=="" (
	echo:
	echo Could not parse command "%1". Syntax:
	echo "<script> clean | configure | build | configure_and_build"
	echo:
	exit 1
)

set ORIGCD=%CD%
set num_cores=%NUMBER_OF_PROCESSORS%
if %num_cores% LSS 1 (
    num_cores=1
)
echo Cores: %num_cores%

(for %%i in (%abiList%) do (
	(for %%j in (%buildType%) do (
		IF "%action%"=="clean" ( 
			call :clean %%j %%i
		)
		IF "%action%"=="configure" ( 
			call :createdir %%j %%i
			call :configure %%j %%i
		)
		IF "%action%"=="configure_and_build" ( 
			call :createdir %%j %%i
			call :configure %%j %%i
			call :build %%j %%i
		)
		IF "%action%"=="build" (
			call :createdir %%j %%i
			call :build %%j %%i
	))
))
)

:parsecommand
	IF "%1"=="clean" set action=%1
	IF "%1"=="build" set action=%1
	IF "%1"=="configure" set action=%1
	IF "%1"=="configure_and_build" set action=%1
	exit /b 0

:clean
	cd %ORIGCD%
	IF EXIST %1 rd /s /q %1
    exit /b 0 

:createdir
	cd %ORIGCD%
	IF NOT EXIST %1 mkdir %1
	cd %1
	IF NOT EXIST %2 mkdir %2
	cd %ORIGCD%
	exit /b 0
	
:configure
	cd %ORIGCD%\%1\%2
	call cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK%/build/cmake/android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=%apiLevel% -DANDROID_ABI=%2 -DANDROID_STL=c++_static -DANDROID_STL_FORCE_FEATURES=ON -DCMAKE_BUILD_TYPE=%1 -DNO_TESTS=1 ../../../../srcs -DCMAKE_SHARED_LINKER_FLAGS=-Wl,--exclude-libs=c++_static.a
	exit /b 0
	
:build
	cd %ORIGCD%\%1\%2
	call cmake --build . -- -j%num_cores%
	cd %ORIGCD%
	exit /b 0