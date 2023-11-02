if (CMAKE_VERSION VERSION_LESS 3.1.0)
    message(FATAL_ERROR "Qt 5 SslServer module requires at least CMake version 3.1.0")
endif()

get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
# Use original install prefix when loaded through a
# cross-prefix symbolic link such as /lib -> /usr/lib.
get_filename_component(_realCurr "${_IMPORT_PREFIX}" REALPATH)
get_filename_component(_realOrig "/usr/lib/arm-linux-gnueabihf/cmake/Qt5SslServer" REALPATH)
if(_realCurr STREQUAL _realOrig)
    get_filename_component(_qt5SslServer_install_prefix "/usr/lib/arm-linux-gnueabihf/../../" ABSOLUTE)
else()
    get_filename_component(_qt5SslServer_install_prefix "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)
endif()
unset(_realOrig)
unset(_realCurr)
unset(_IMPORT_PREFIX)

# For backwards compatibility only. Use Qt5SslServer_VERSION instead.
set(Qt5SslServer_VERSION_STRING 5.12.0)

set(Qt5SslServer_LIBRARIES Qt5::SslServer)

macro(_qt5_SslServer_check_file_exists file)
    if(NOT EXISTS "${file}" )
        message(FATAL_ERROR "The imported target \"Qt5::SslServer\" references the file
   \"${file}\"
but this file does not exist.  Possible reasons include:
* The file was deleted, renamed, or moved to another location.
* An install or uninstall procedure did not complete successfully.
* The installation package was faulty and contained
   \"${CMAKE_CURRENT_LIST_FILE}\"
but not all the files it references.
")
    endif()
endmacro()


macro(_populate_SslServer_target_properties Configuration LIB_LOCATION IMPLIB_LOCATION
      IsDebugAndRelease)
    set_property(TARGET Qt5::SslServer APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

    set(imported_location "${_qt5SslServer_install_prefix}/lib/arm-linux-gnueabihf/${LIB_LOCATION}")
    _qt5_SslServer_check_file_exists(${imported_location})
    set(_deps
        ${_Qt5SslServer_LIB_DEPENDENCIES}
    )
    set(_static_deps
    )

    set_target_properties(Qt5::SslServer PROPERTIES
        "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        "IMPORTED_SONAME_${Configuration}" "libQt5SslServer.so.5"
        # For backward compatibility with CMake < 2.8.12
        "IMPORTED_LINK_INTERFACE_LIBRARIES_${Configuration}" "${_deps};${_static_deps}"
    )
    set_property(TARGET Qt5::SslServer APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 "${_deps}"
    )


endmacro()

if (NOT TARGET Qt5::SslServer)

    set(_Qt5SslServer_OWN_INCLUDE_DIRS "${_qt5SslServer_install_prefix}/include/arm-linux-gnueabihf/qt5/" "${_qt5SslServer_install_prefix}/include/arm-linux-gnueabihf/qt5/QtSslServer")
    set(Qt5SslServer_PRIVATE_INCLUDE_DIRS "")

    foreach(_dir ${_Qt5SslServer_OWN_INCLUDE_DIRS})
        _qt5_SslServer_check_file_exists(${_dir})
    endforeach()

    # Only check existence of private includes if the Private component is
    # specified.
    list(FIND Qt5SslServer_FIND_COMPONENTS Private _check_private)
    if (NOT _check_private STREQUAL -1)
        foreach(_dir ${Qt5SslServer_PRIVATE_INCLUDE_DIRS})
            _qt5_SslServer_check_file_exists(${_dir})
        endforeach()
    endif()

    set(Qt5SslServer_INCLUDE_DIRS ${_Qt5SslServer_OWN_INCLUDE_DIRS})

    set(Qt5SslServer_DEFINITIONS -DQT_SSLSERVER_LIB)
    set(Qt5SslServer_COMPILE_DEFINITIONS QT_SSLSERVER_LIB)
    set(_Qt5SslServer_MODULE_DEPENDENCIES "Network;Core")


    set(Qt5SslServer_OWN_PRIVATE_INCLUDE_DIRS ${Qt5SslServer_PRIVATE_INCLUDE_DIRS})

    set(_Qt5SslServer_FIND_DEPENDENCIES_REQUIRED)
    if (Qt5SslServer_FIND_REQUIRED)
        set(_Qt5SslServer_FIND_DEPENDENCIES_REQUIRED REQUIRED)
    endif()
    set(_Qt5SslServer_FIND_DEPENDENCIES_QUIET)
    if (Qt5SslServer_FIND_QUIETLY)
        set(_Qt5SslServer_DEPENDENCIES_FIND_QUIET QUIET)
    endif()
    set(_Qt5SslServer_FIND_VERSION_EXACT)
    if (Qt5SslServer_FIND_VERSION_EXACT)
        set(_Qt5SslServer_FIND_VERSION_EXACT EXACT)
    endif()

    set(Qt5SslServer_EXECUTABLE_COMPILE_FLAGS "")

    foreach(_module_dep ${_Qt5SslServer_MODULE_DEPENDENCIES})
        if (NOT Qt5${_module_dep}_FOUND)
            find_package(Qt5${_module_dep}
                5.12.0 ${_Qt5SslServer_FIND_VERSION_EXACT}
                ${_Qt5SslServer_DEPENDENCIES_FIND_QUIET}
                ${_Qt5SslServer_FIND_DEPENDENCIES_REQUIRED}
                PATHS "${CMAKE_CURRENT_LIST_DIR}/.." NO_DEFAULT_PATH
            )
        endif()

        if (NOT Qt5${_module_dep}_FOUND)
            set(Qt5SslServer_FOUND False)
            return()
        endif()

        list(APPEND Qt5SslServer_INCLUDE_DIRS "${Qt5${_module_dep}_INCLUDE_DIRS}")
        list(APPEND Qt5SslServer_PRIVATE_INCLUDE_DIRS "${Qt5${_module_dep}_PRIVATE_INCLUDE_DIRS}")
        list(APPEND Qt5SslServer_DEFINITIONS ${Qt5${_module_dep}_DEFINITIONS})
        list(APPEND Qt5SslServer_COMPILE_DEFINITIONS ${Qt5${_module_dep}_COMPILE_DEFINITIONS})
        list(APPEND Qt5SslServer_EXECUTABLE_COMPILE_FLAGS ${Qt5${_module_dep}_EXECUTABLE_COMPILE_FLAGS})
    endforeach()
    list(REMOVE_DUPLICATES Qt5SslServer_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES Qt5SslServer_PRIVATE_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES Qt5SslServer_DEFINITIONS)
    list(REMOVE_DUPLICATES Qt5SslServer_COMPILE_DEFINITIONS)
    list(REMOVE_DUPLICATES Qt5SslServer_EXECUTABLE_COMPILE_FLAGS)

    # It can happen that the same FooConfig.cmake file is included when calling find_package()
    # on some Qt component. An example of that is when using a Qt static build with auto inclusion
    # of plugins:
    #
    # Qt5WidgetsConfig.cmake -> Qt5GuiConfig.cmake -> Qt5Gui_QSvgIconPlugin.cmake ->
    # Qt5SvgConfig.cmake -> Qt5WidgetsConfig.cmake ->
    # finish processing of second Qt5WidgetsConfig.cmake ->
    # return to first Qt5WidgetsConfig.cmake ->
    # add_library cannot create imported target Qt5::Widgets.
    #
    # Make sure to return early in the original Config inclusion, because the target has already
    # been defined as part of the second inclusion.
    if(TARGET Qt5::SslServer)
        return()
    endif()

    set(_Qt5SslServer_LIB_DEPENDENCIES "Qt5::Network;Qt5::Core")


    add_library(Qt5::SslServer SHARED IMPORTED)


    set_property(TARGET Qt5::SslServer PROPERTY
      INTERFACE_INCLUDE_DIRECTORIES ${_Qt5SslServer_OWN_INCLUDE_DIRS})
    set_property(TARGET Qt5::SslServer PROPERTY
      INTERFACE_COMPILE_DEFINITIONS QT_SSLSERVER_LIB)

    set_property(TARGET Qt5::SslServer PROPERTY INTERFACE_QT_ENABLED_FEATURES )
    set_property(TARGET Qt5::SslServer PROPERTY INTERFACE_QT_DISABLED_FEATURES )

    # Qt 6 forward compatible properties.
    set_property(TARGET Qt5::SslServer
                 PROPERTY QT_ENABLED_PUBLIC_FEATURES
                 )
    set_property(TARGET Qt5::SslServer
                 PROPERTY QT_DISABLED_PUBLIC_FEATURES
                 )
    set_property(TARGET Qt5::SslServer
                 PROPERTY QT_ENABLED_PRIVATE_FEATURES
                 )
    set_property(TARGET Qt5::SslServer
                 PROPERTY QT_DISABLED_PRIVATE_FEATURES
                 )

    set_property(TARGET Qt5::SslServer PROPERTY INTERFACE_QT_PLUGIN_TYPES "")

    set(_Qt5SslServer_PRIVATE_DIRS_EXIST TRUE)
    foreach (_Qt5SslServer_PRIVATE_DIR ${Qt5SslServer_OWN_PRIVATE_INCLUDE_DIRS})
        if (NOT EXISTS ${_Qt5SslServer_PRIVATE_DIR})
            set(_Qt5SslServer_PRIVATE_DIRS_EXIST FALSE)
        endif()
    endforeach()

    if (_Qt5SslServer_PRIVATE_DIRS_EXIST)
        add_library(Qt5::SslServerPrivate INTERFACE IMPORTED)
        set_property(TARGET Qt5::SslServerPrivate PROPERTY
            INTERFACE_INCLUDE_DIRECTORIES ${Qt5SslServer_OWN_PRIVATE_INCLUDE_DIRS}
        )
        set(_Qt5SslServer_PRIVATEDEPS)
        foreach(dep ${_Qt5SslServer_LIB_DEPENDENCIES})
            if (TARGET ${dep}Private)
                list(APPEND _Qt5SslServer_PRIVATEDEPS ${dep}Private)
            endif()
        endforeach()
        set_property(TARGET Qt5::SslServerPrivate PROPERTY
            INTERFACE_LINK_LIBRARIES Qt5::SslServer ${_Qt5SslServer_PRIVATEDEPS}
        )

        # Add a versionless target, for compatibility with Qt6.
        if(NOT "${QT_NO_CREATE_VERSIONLESS_TARGETS}" AND NOT TARGET Qt::SslServerPrivate)
            add_library(Qt::SslServerPrivate INTERFACE IMPORTED)
            set_target_properties(Qt::SslServerPrivate PROPERTIES
                INTERFACE_LINK_LIBRARIES "Qt5::SslServerPrivate"
            )
        endif()
    endif()

    _populate_SslServer_target_properties(RELEASE "libQt5SslServer.so.5.12.0" "" FALSE)




    # In Qt 5.15 the glob pattern was relaxed to also catch plugins not literally named Plugin.
    # Define QT5_STRICT_PLUGIN_GLOB or ModuleName_STRICT_PLUGIN_GLOB to revert to old behavior.
    if (QT5_STRICT_PLUGIN_GLOB OR Qt5SslServer_STRICT_PLUGIN_GLOB)
        file(GLOB pluginTargets "${CMAKE_CURRENT_LIST_DIR}/Qt5SslServer_*Plugin.cmake")
    else()
        file(GLOB pluginTargets "${CMAKE_CURRENT_LIST_DIR}/Qt5SslServer_*.cmake")
    endif()

    macro(_populate_SslServer_plugin_properties Plugin Configuration PLUGIN_LOCATION
          IsDebugAndRelease)
        set_property(TARGET Qt5::${Plugin} APPEND PROPERTY IMPORTED_CONFIGURATIONS ${Configuration})

        set(imported_location "${_qt5SslServer_install_prefix}/lib/arm-linux-gnueabihf/qt5/plugins/${PLUGIN_LOCATION}")
        _qt5_SslServer_check_file_exists(${imported_location})
        set_target_properties(Qt5::${Plugin} PROPERTIES
            "IMPORTED_LOCATION_${Configuration}" ${imported_location}
        )

    endmacro()

    if (pluginTargets)
        foreach(pluginTarget ${pluginTargets})
            include(${pluginTarget})
        endforeach()
    endif()



    _qt5_SslServer_check_file_exists("${CMAKE_CURRENT_LIST_DIR}/Qt5SslServerConfigVersion.cmake")
endif()

# Add a versionless target, for compatibility with Qt6.
if(NOT "${QT_NO_CREATE_VERSIONLESS_TARGETS}" AND TARGET Qt5::SslServer AND NOT TARGET Qt::SslServer)
    add_library(Qt::SslServer INTERFACE IMPORTED)
    set_target_properties(Qt::SslServer PROPERTIES
        INTERFACE_LINK_LIBRARIES "Qt5::SslServer"
    )
endif()
