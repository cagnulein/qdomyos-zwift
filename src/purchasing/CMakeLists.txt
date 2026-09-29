cmake_minimum_required(VERSION 3.23)

# Create the purchasing library
add_library(purchasing STATIC)

# Core sources
target_sources(purchasing PRIVATE
    qmltypes/inappproductqmltype.cpp
    qmltypes/inappstoreqmltype.cpp
    inapp/inappproduct.cpp
    inapp/inapppurchasebackend.cpp
    inapp/inappstore.cpp
    inapp/inapptransaction.cpp
)

# Core headers
target_sources(purchasing PRIVATE
    qmltypes/inappproductqmltype.h
    qmltypes/inappstoreqmltype.h
    inapp/inappproduct.h
    inapp/inapppurchasebackend.h
    inapp/inappstore.h
    inapp/inapptransaction.h
)

# Android-specific sources
if(ANDROID)
    target_sources(purchasing PRIVATE
        android/androidinappproduct.cpp
        android/androidinapppurchasebackend.cpp
        android/androidinapptransaction.cpp
        android/androidjni.cpp
        android/androidinappproduct.h
        android/androidinapppurchasebackend.h
        android/androidinapptransaction.h
    )
endif()

# iOS-specific sources
if(IOS)
    target_sources(purchasing PRIVATE
        ios/iosinapppurchasebackend.h
        ios/iosinapppurchaseproduct.h
        ios/iosinapppurchasetransaction.h
        ios/iosinapppurchasebackend.mm
        ios/iosinapppurchaseproduct.mm
        ios/iosinapppurchasetransaction.mm
    )
    
    # iOS frameworks
    find_library(STOREKIT_FRAMEWORK StoreKit)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    target_link_libraries(purchasing PRIVATE 
        ${STOREKIT_FRAMEWORK} 
        ${FOUNDATION_FRAMEWORK}
    )
endif()

# Link Qt6 libraries
target_link_libraries(purchasing PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickControls2
)

# Android-specific Qt dependencies
if(ANDROID)
    target_link_libraries(purchasing PRIVATE Qt6::CorePrivate)
endif()

# Include directories
target_include_directories(purchasing PUBLIC
    .
    qmltypes
    inapp
)

# Set target properties
set_target_properties(purchasing PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)