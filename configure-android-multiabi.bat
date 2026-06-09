@echo off
REM Configure CMake for Android Multi-ABI and open in Qt Creator
REM This script properly configures the project for multi-ABI build

echo.
echo ============================================
echo  Configuring Android Multi-ABI Project
echo ============================================
echo.

REM Set paths
set QT_DIR=C:\Qt\6.8.2
set ANDROID_SDK_ROOT=C:\Users\%USERNAME%\AppData\Local\Android\Sdk
set ANDROID_NDK_ROOT=%ANDROID_SDK_ROOT%\ndk\25.1.8937393

REM Fallback to other NDK versions if preferred one doesn't exist
if not exist "%ANDROID_NDK_ROOT%" set ANDROID_NDK_ROOT=%ANDROID_SDK_ROOT%\ndk\26.1.10909125
if not exist "%ANDROID_NDK_ROOT%" set ANDROID_NDK_ROOT=%ANDROID_SDK_ROOT%\ndk\21.3.6528147

REM Verify paths
if not exist "%QT_DIR%\android_arm64_v8a" (
    echo ERROR: Qt Android not found at %QT_DIR%
    pause
    exit /b 1
)

if not exist "%ANDROID_NDK_ROOT%" (
    echo ERROR: Android NDK not found at %ANDROID_NDK_ROOT%
    echo Please install NDK 21.4.7075529 through Android Studio
    pause
    exit /b 1
)

echo Using Qt: %QT_DIR%
echo Using NDK: %ANDROID_NDK_ROOT%
echo.

REM Clean previous build directories
if exist build-android-multiabi (
    echo Cleaning previous multi-ABI build...
    rmdir /s /q build-android-multiabi
)

REM Create build directory
mkdir build-android-multiabi
cd build-android-multiabi

echo.
echo ============================================
echo  Configuring CMake for Multi-ABI
echo ============================================
echo.

REM Configure CMake with explicit multi-ABI settings
cmake .. ^
    -DCMAKE_TOOLCHAIN_FILE="%ANDROID_NDK_ROOT%\build\cmake\android.toolchain.cmake" ^
    -DQT_HOST_PATH="%QT_DIR%\msvc2022_64" ^
    -DCMAKE_PREFIX_PATH="%QT_DIR%\android_arm64_v8a" ^
    -DANDROID_ABI=arm64-v8a ^
    -DANDROID_PLATFORM=android-23 ^
    -DANDROID_STL=c++_shared ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DQT_ANDROID_BUILD_ALL_ABIS=ON ^
    -DQT_ANDROID_ABIS="armeabi-v7a;arm64-v8a;x86;x86_64" ^
    -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=BOTH ^
    -DANDROID_SDK_ROOT="%ANDROID_SDK_ROOT%" ^
    -DANDROID_NDK_ROOT="%ANDROID_NDK_ROOT%"

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration failed!
    echo Check the error messages above.
    cd ..
    pause
    exit /b 1
)

echo.
echo ============================================
echo  Configuration successful!
echo ============================================
echo.

REM Go back to project root
cd ..

echo Project configured for Android Multi-ABI build.
echo Build directory: build-android-multiabi
echo.
echo To build from command line:
echo   cd build-android-multiabi
echo   cmake --build . --parallel
echo.
echo To open in Qt Creator:
echo   1. Open Qt Creator
echo   2. File -^> Open File or Project
echo   3. Select: %CD%\build-android-multiabi\CMakeCache.txt
echo   4. Or just open the CMakeLists.txt and select the configured build
echo.

REM Ask if user wants to open Qt Creator
set /p OPEN_QTC="Open Qt Creator now? (y/n): "
if /i "%OPEN_QTC%"=="y" (
    echo Opening Qt Creator...
    start "" "%ProgramFiles%\Qt\QtCreator\bin\qtcreator.exe" "%CD%"
)

pause