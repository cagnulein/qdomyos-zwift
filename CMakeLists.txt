cmake_minimum_required(VERSION 3.23)
project(qdomyos-zwift VERSION 2.20.5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Qt6 path based on platform and compiler
if(WIN32 AND NOT DEFINED CMAKE_PREFIX_PATH)
    if(ANDROID)
        # Android builds - use all Android ABIs
        set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/android_arm64_v8a")
    elseif(MSVC)
        # Windows MSVC builds
        set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/msvc2022_64")
    elseif(MINGW)
        # Windows MinGW builds
        set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/mingw_64")
    else()
        # Default Windows path
        set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/msvc2022_64")
    endif()
endif()

# Find required Qt6 components
if(ANDROID)
    # Android specific components (some modules not available)
    find_package(Qt6 REQUIRED COMPONENTS 
        Core 
        Quick 
        Qml
        QuickControls2
        Network 
        Gui 
        Xml 
        Positioning 
        NetworkAuth 
        WebSockets 
        Location 
        Multimedia 
        Charts
        Sql
    )
    # Try to find optional Android modules
    find_package(Qt6Bluetooth QUIET)
    if(Qt6Bluetooth_FOUND)
        message(STATUS "Qt6Bluetooth found for Android")
    else()
        message(WARNING "Qt6Bluetooth not found for Android - Bluetooth features will be disabled")
    endif()
    
    find_package(Qt6Core5Compat QUIET)
    if(Qt6Core5Compat_FOUND)
        message(STATUS "Qt6Core5Compat found for Android")
    else()
        message(WARNING "Qt6Core5Compat not found for Android - Qt5 compatibility features will be disabled")
    endif()
    
    find_package(Qt6TextToSpeech QUIET)
    if(Qt6TextToSpeech_FOUND)
        message(STATUS "Qt6TextToSpeech found for Android")
    else()
        message(WARNING "Qt6TextToSpeech not found for Android - Text-to-speech features will be disabled")
    endif()
else()
    # Desktop/full components
    find_package(Qt6 REQUIRED COMPONENTS 
        Core 
        Widgets 
        Bluetooth 
        Xml 
        Positioning 
        Quick 
        Qml
        QuickControls2
        NetworkAuth 
        WebSockets 
        TextToSpeech 
        Location 
        Multimedia 
        MultimediaWidgets 
        Network 
        Gui 
        WebView
        Charts
        Core5Compat
    )
endif()

# Qt6 policies
qt_policy(SET QTP0001 NEW)

# Android Multi-ABI support
if(ANDROID)
    # Enable multi-ABI only if explicitly requested
    if(DEFINED QT_ANDROID_BUILD_ALL_ABIS AND QT_ANDROID_BUILD_ALL_ABIS)
        message(STATUS "Configuring for Android Multi-ABI build")
        set(QT_ANDROID_BUILD_ALL_ABIS TRUE CACHE BOOL "Build for all available Android ABIs")
        set(QT_ANDROID_ABIS "arm64-v8a" CACHE STRING "Android ABIs to build for")
        message(STATUS "Target ABIs: ${QT_ANDROID_ABIS}")
    else()
        message(STATUS "Configuring for single Android ABI: ${ANDROID_ABI}")
    endif()
endif()

# Add the main application
add_subdirectory(src)

# Add tests for non-mobile platforms
if(NOT ANDROID AND NOT IOS)
    enable_testing()
    add_subdirectory(tst)
endif()