# =============================================================================
# Dockerfile for QDomyos-Zwift (Linux x86-64 with ANT+)
#
# This file defines a multi-stage build to compile the application for the
# linux/amd64 platform, mirroring the environment of the linux-x86-build job.
# =============================================================================

# --- Stage 1: The Builder ---
# Use Ubuntu 22.04 LTS for maximum compatibility with Qt5 development packages.
FROM ubuntu:22.04 AS builder

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install all necessary build dependencies.
# This list is now corrected to include all required Qt5 development modules.
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    git \
    cmake \
    xvfb \
    # Core Qt5 development packages
    qtbase5-dev \
    qtbase5-private-dev \
    qtchooser \
    qt5-qmake \
    qtbase5-dev-tools \
    qtquickcontrols2-5-dev \
    libqt5bluetooth5 \
    libqt5widgets5 \
    libqt5positioning5 \
    libqt5xml5 \
    qtconnectivity5-dev \
    qtpositioning5-dev \
    libqt5charts5-dev \
    libqt5charts5 \
    libqt5networkauth5-dev \
    libqt5websockets5-dev \
    qtmultimedia5-dev \
    qtlocation5-dev \
    libqt5texttospeech5-dev \
    libxcb-randr0-dev \
    libxcb-xtest0-dev \
    libxcb-xinerama0-dev \
    libxcb-shape0-dev \
    libxcb-xkb-dev \
    libqt5sql5-sqlite \
    libqt5sql5-mysql \
    libqt5sql5-psql \
    # ANT+ specific dependencies
    python3-dev \
    python3-venv \
    python3-pip \
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

# Instead of compiling qthttpserver, we copy the pre-built headers,
# exactly like the original linux-x86-build CI job.
RUN cp qHttpServerBin/5.15.2/headers/* src/qthttpserver/src/3rdparty/http-parser/

# Configure the project for an ANT+ build
RUN echo 'include(devices/antlinux/antlinux.pri)' >> src/qdomyos-zwift.pro && \
    echo '/opt/ant_venv/bin/python3' > src/.ant_venv_path

# Run qmake and compile the main project
RUN cd src && qmake qdomyos-zwift.pro && make -j$(nproc)

# --- Stage 2: The Final Artifact ---
# Use a minimal image for the final stage
FROM scratch

# Copy the compiled binary from the builder stage
COPY --from=builder /app/src/qdomyos-zwift /qdomyos-zwift-x86-64-ant