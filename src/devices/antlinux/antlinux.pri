# =============================================================================
# QDomyos-Zwift: ANT+ qmake Configuration (Linux)
#
# Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
# Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
#
# Conditionally enables ANT+ feature for Linux builds, detects Python venv,
# configures include paths and generates embedded script headers.
# =============================================================================

# This feature is only enabled for Linux desktop builds (g++ or clang).
linux-g++:!android|linux-clang:!android {
    # Find the .ant_venv_path file, which is created by the build script in the project's root.
    # Paths are relative from this .pri file's location.
    VENV_PATH_FILE = $$clean_path($$PWD/../../.ant_venv_path)
    !exists($$VENV_PATH_FILE) {
        # Fallback for out-of-source builds (e.g., build directory is sibling to source directory)
        VENV_PATH_FILE = $$clean_path($$OUT_PWD/../src/.ant_venv_path)
    }

    exists($$VENV_PATH_FILE) {
        message("================ ANT+ Footpod Support ====================")
        message("Found ANT venv path file at: $$VENV_PATH_FILE")

        VENV_PYTHON_RAW = $$cat($$VENV_PATH_FILE, blob)
        VENV_PYTHON = $$replace(VENV_PYTHON_RAW, "\n", "")

        !exists($$VENV_PYTHON) {
            error("Python executable not found at $$VENV_PYTHON as specified in .ant_venv_path")
        }

        # Get Python and pybind11 configuration by executing the virtual environment's Python
        PYVER = $$system("\"$$VENV_PYTHON\" -c \"import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')\"")
        PYBIND11_INC = $$system("\"$$VENV_PYTHON\" -c \"import pybind11; print(pybind11.get_include())\"")

        !isEmpty(PYVER):!isEmpty(PYBIND11_INC) {
            DEFINES += ANT_LINUX_ENABLED
            QT += bluetooth # Module dependency
            message(">>> ANT+ ENABLED for build <<<")
            message("Python version: $$PYVER, pybind11 include path: $$PYBIND11_INC")

            # ccache optimization
            CCACHE_PATH = $$system(which ccache 2>/dev/null)
            !isEmpty(CCACHE_PATH) {
                QMAKE_CC = ccache $$QMAKE_CC
                QMAKE_CXX = ccache $$QMAKE_CXX
                message("Using ccache: $$CCACHE_PATH")
            }

            # --- Python script embedding rule ---
            generate_header.target = $$OUT_PWD/ant_footpod_script.h
            generate_header.commands = "$$VENV_PYTHON" $$PWD/py_to_header.py $$PWD/ant_broadcaster.py $$generate_header.target
            generate_header.depends = $$PWD/ant_broadcaster.py $$PWD/py_to_header.py
            QMAKE_EXTRA_TARGETS += generate_header
            PRE_TARGETDEPS += $$generate_header.target

            # --- Source and Header Definitions ---
            # --- NEW: Take exclusive control of the source files ---
            # First, REMOVE the files from SOURCES in case they were added by a generic rule
            # in the main .pro file. This prevents "multiple definition" errors.
            SOURCES -= $$PWD/AntManager.cpp
            SOURCES -= $$PWD/AntWorker.cpp
            HEADERS -= $$PWD/AntManager.h
            HEADERS -= $$PWD/AntWorker.h

            # Now, ADD them back under the control of this .pri file.
            SOURCES += $$PWD/AntManager.cpp $$PWD/AntWorker.cpp
            HEADERS += $$PWD/AntManager.h $$PWD/AntWorker.h

            # --- Add Python-specific paths and libraries ---
            INCLUDEPATH += $$PYBIND11_INC $$OUT_PWD

            # --- Determine if we need cross-compilation setup ---
            # Get host architecture
            HOST_ARCH = $$system(uname -m)
            
            # Get target architecture from compiler
            TARGET_ARCH = $$system($$QMAKE_CXX -dumpmachine | cut -d'-' -f1)
            
            # Normalize architecture names for comparison
            equals(HOST_ARCH, aarch64): HOST_ARCH = arm64
            equals(HOST_ARCH, x86_64): HOST_ARCH = x64
            equals(TARGET_ARCH, aarch64): TARGET_ARCH = arm64
            equals(TARGET_ARCH, x86_64): TARGET_ARCH = x64
            
            message("Host architecture: $$HOST_ARCH")
            message("Target architecture: $$TARGET_ARCH")
            
            # Determine if this is cross-compilation
            IS_CROSS_COMPILE = false
            !equals(HOST_ARCH, $$TARGET_ARCH) {
                IS_CROSS_COMPILE = true
            }
            
            equals(IS_CROSS_COMPILE, true):contains(TARGET_ARCH, arm64) {
                message("Configuring for ARM64 Cross-Compilation ($$HOST_ARCH -> $$TARGET_ARCH)")
                PYTHON_HEADERS_PATH = $$OUT_PWD/python-arm64-headers/python$$PYVER
                PYTHON_LIBS_PATH = $$OUT_PWD/python-arm64-libs
                INCLUDEPATH += $$PYTHON_HEADERS_PATH
                LIBS += -L$$PYTHON_LIBS_PATH -lpython$$PYVER
                QMAKE_LFLAGS += -Wl,-rpath-link,$$PYTHON_LIBS_PATH
            } else {
                message("Configuring for Native Compilation ($$HOST_ARCH)")
                PY_CONFIG = $$system(which python$$PYVER-config)
                QMAKE_CXXFLAGS += $$system($$PY_CONFIG --cflags)
                LIBS += $$system($$PY_CONFIG --ldflags --embed)
            }

            # Disable Link-Time Optimization (LTO) for only these two files to prevent
            # linking errors with the Python library, which is a common issue.
            $$OBJECTS_DIR/AntManager.o.CXXFLAGS += -fno-lto
            $$OBJECTS_DIR/AntWorker.o.CXXFLAGS += -fno-lto
            message("LTO disabled specifically for AntManager.o and AntWorker.o")

            message("ANT+ configuration completed successfully")
        } else {
            warning("ANT+ disabled - could not determine Python/pybind11 configuration.")
        }
        message("==========================================================")
    } else {
        message("ANT+ disabled - .ant_venv_path file not found.")
    }
}