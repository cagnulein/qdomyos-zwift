# =============================================================================
# QDomyos-Zwift: Multi-Stage Docker Build (ARM64 / Raspberry Pi)
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor: bassai-sho | AI-assisted development | License: GPL-3.0
#
# Cross-compiles QDomyos-Zwift for linux/arm64 platform (Raspberry Pi 3+).
# Uses Debian Bookworm with pinned Python 3.11.2 for stable ANT+ integration.
#
# Build stages:
#   1. Builder: Compiles Qt5/C++ application with embedded Python ANT+ support
#   2. Artifact: Minimal busybox container for file extraction only
#
# Output: /qdomyos-zwift-arm64-ant/ directory with binary, scripts, and configs
#
# Usage:
#   docker buildx build --platform linux/arm64 -f docker/rpi-ant/Dockerfile .
#   docker buildx build --build-arg ENABLE_DEBUG_LOGS=1 ...  # Debug mode
# =============================================================================

# --- Stage 1: The Builder ---
# Use the latest stable Debian release to get ongoing security updates.
FROM debian:bookworm AS builder

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install all necessary build dependencies.
# We pin the Python version to 3.11.2 (package version 3.11.2-1+b1)
# to ensure a reproducible build for the C++ interface.
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    git \
    cmake \
    qtbase5-dev \
    qtbase5-private-dev \
    qtchooser \
    qt5-qmake \
    qtbase5-dev-tools \
    qttools5-dev-tools \
    libqt5svg5-dev \
    qtmultimedia5-dev \
    libqt5charts5-dev \
    qtpositioning5-dev \
    qtconnectivity5-dev \
    libqt5websockets5-dev \
    libqt5texttospeech5-dev \
    libqt5bluetooth5 \
    libqt5networkauth5-dev \
    qml-module-qtlocation \
    qml-module-qtpositioning \
    qtlocation5-dev \
    libqt5quickcontrols2-5 \
    qtquickcontrols2-5-dev \
    qml-module-qtquick-controls2 \
    libqt5sql5-sqlite \
    libqt5sql5-mysql \
    libqt5sql5-psql \
    libsqlite3-dev \
    python3-pip \
    # --- Start of Pinned Python Version ---
    python3=3.11.2-1+b1 \
    python3-dev=3.11.2-1+b1 \
    python3-venv=3.11.2-1+b1 \
    # --- End of Pinned Python Version ---
    libusb-1.0-0-dev \
    libudev-dev

# Set up the Python virtual environment for ANT+ support
RUN python3.11 -m venv /opt/ant_venv && \
    /opt/ant_venv/bin/pip install --upgrade pip --quiet && \
    /opt/ant_venv/bin/pip install openant pyusb pybind11 --quiet

# Copy the entire project source into the container
WORKDIR /app
COPY . .

# Generate devices.ini from QML source file
# The parser creates devices.ini directly in src/ and outputs JSON cache files.
# If the QML file is not found, create an empty devices.ini to prevent build failure.
RUN if [ -f src/settings.qml ]; then \
            echo "Generating devices.ini from settings.qml..."; \
            cd /app && \
            /opt/ant_venv/bin/python3 src/devices/antlinux/generate_devices.py src/settings.qml && \
            echo "✓ devices.ini generated successfully"; \
        else \
            echo "⚠ Warning: No QML settings file found"; \
            echo "Creating empty devices.ini to prevent build failure"; \
            touch src/devices.ini; \
        fi

# Initialize only the needed submodules (avoid full recursive fetch)
RUN git submodule update --init src/smtpclient/ && \
    git submodule update --init src/qmdnsengine/ && \
    git submodule update --init src/qthttpserver/ && \
    git submodule update --init tst/googletest/

# Configure the project for an ANT+ build
RUN echo 'include(devices/antlinux/antlinux.pri)' >> src/qdomyos-zwift.pro && \
    echo '/opt/ant_venv/bin/python3' > src/.ant_venv_path && \
    sed -i '/QtHttpServer/d' src/qdomyos-zwift.pro && \
    find src -type f \( -name '*.cpp' -o -name '*.h' \) -exec sed -i 's/#include <QtHttpServer/\/\/#include <QtHttpServer/' {} + && \
    find src -type f \( -name '*.cpp' -o -name '*.h' \) -exec sed -i 's/QHttpServer/\/\/QHttpServer/' {} +

# --- OPTIMIZED BUILD STEP ---
# Accept build argument to enable debug logging (optional, defaults to release)
ARG ENABLE_DEBUG_LOGS=0

# 1. Run qmake with release or debug flags based on ENABLE_DEBUG_LOGS.
# 2. Run make to compile the application.
# 3. Run strip on the final binary ONLY for release builds.
RUN cd src && \
    if [ "$ENABLE_DEBUG_LOGS" = "1" ]; then \
        echo "Building with DEBUG LOGS ENABLED (qDebug output visible)"; \
        qmake "CONFIG+=debug" "CONFIG+=optimize_full" \
              "DEFINES-=QT_NO_DEBUG" "DEFINES-=QT_NO_DEBUG_OUTPUT" \
              "QMAKE_CXXFLAGS+=-ffile-prefix-map=/app=." qdomyos-zwift.pro; \
        make -j$(nproc); \
    else \
        echo "Building RELEASE mode (qDebug suppressed for smaller binary)"; \
        qmake "CONFIG+=release" "QMAKE_CXXFLAGS+=-ffile-prefix-map=/app=." qdomyos-zwift.pro; \
        make -j$(nproc); \
        strip qdomyos-zwift; \
    fi && \
    mv qdomyos-zwift qdomyos-zwift-bin

# --- Stage 2: Minimal Artifact Container ---
# Using busybox for minimal image size since files are only extracted, not run here.
# Users will run the binary on their host with system-installed dependencies.
FROM busybox:latest

# Create the application directory structure
RUN mkdir -p /qdomyos-zwift-arm64-ant

# Copy the compiled binary and wrapper script from the build context
COPY --from=builder /app/src/qdomyos-zwift-bin /qdomyos-zwift-arm64-ant/
COPY docker/rpi-ant/qdomyos-zwift-wrapper.sh /qdomyos-zwift-arm64-ant/qdomyos-zwift
COPY src/devices/antlinux/setup-dashboard.sh /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/bt_provider.py /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/test_ant.py /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/ant_broadcaster.py /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/README.md /qdomyos-zwift-arm64-ant/
COPY --from=builder /app/src/devices.ini /qdomyos-zwift-arm64-ant/devices.ini
RUN chmod +x /qdomyos-zwift-arm64-ant/qdomyos-zwift && \
    chmod +x /qdomyos-zwift-arm64-ant/setup-dashboard.sh || true

# Set a no-op command (required for Docker image)
CMD ["/bin/bash"]