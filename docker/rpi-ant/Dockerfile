# =============================================================================
# Dockerfile for QDomyos-Zwift (Raspberry Pi 64-bit with ANT+)
#
# This file defines a multi-stage build to cross-compile the application
# for the linux/arm64 platform.
#
# It uses the latest 'bookworm' base image for security but pins the Python
# version to ensure a stable build for the ANT+ feature.
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
RUN python3 -m venv /opt/ant_venv && \
    /opt/ant_venv/bin/pip install --upgrade pip --quiet && \
    /opt/ant_venv/bin/pip install openant pyusb pybind11 --quiet

# Copy the entire project source into the container
WORKDIR /app
COPY . .

# Initialize git submodules
RUN git submodule update --init --recursive

# Configure the project for an ANT+ build
RUN echo 'include(devices/antlinux/antlinux.pri)' >> src/qdomyos-zwift.pro && \
    echo '/opt/ant_venv/bin/python3' > src/.ant_venv_path && \
    sed -i '/QtHttpServer/d' src/qdomyos-zwift.pro && \
    find src -type f \( -name '*.cpp' -o -name '*.h' \) -exec sed -i 's/#include <QtHttpServer/\/\/#include <QtHttpServer/' {} + && \
    find src -type f \( -name '*.cpp' -o -name '*.h' \) -exec sed -i 's/QHttpServer/\/\/QHttpServer/' {} +

# --- OPTIMIZED BUILD STEP ---
# 1. Run qmake with explicit release and path-mapping flags.
# 2. Run make to compile the application.
# 3. Run strip on the final binary to remove debug symbols and reduce size.
RUN cd src && \
    qmake "CONFIG+=release" "QMAKE_CXXFLAGS+=-ffile-prefix-map=/app=." qdomyos-zwift.pro && \
    make -j$(nproc) && \
    strip qdomyos-zwift && \
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
COPY src/devices/antlinux/setup.sh /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/test_ant.py /qdomyos-zwift-arm64-ant/
COPY src/devices/antlinux/ant_broadcaster.py /qdomyos-zwift-arm64-ant/
RUN chmod +x /qdomyos-zwift-arm64-ant/qdomyos-zwift && \
    chmod +x /qdomyos-zwift-arm64-ant/setup.sh

# Set a no-op command (required for Docker image)
CMD ["/bin/bash"]