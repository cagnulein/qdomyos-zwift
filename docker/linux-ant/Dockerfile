# =============================================================================
# Dockerfile for QDomyos-Zwift (Linux x86-64 with ANT+)
#
# This file defines a multi-stage build to compile the application for the
# linux/amd64 platform, mirroring the environment of the linux-x86-build job.
# =============================================================================

# --- Stage 1: The Builder ---
FROM debian:bookworm AS builder

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install all necessary build dependencies.
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

# Instead of compiling qthttpserver, we copy the pre-built headers,
# exactly like the original linux-x86-build CI job.
RUN cp qHttpServerBin/5.15.2/headers/* src/qthttpserver/src/3rdparty/http-parser/

# Configure the project for an ANT+ build
RUN echo 'include(devices/antlinux/antlinux.pri)' >> src/qdomyos-zwift.pro && \
    echo '/opt/ant_venv/bin/python3' > src/.ant_venv_path

# --- OPTIMIZED BUILD STEP ---
# 1. Run qmake with explicit release and path-mapping flags.
# 2. Run make to compile the application.
# 3. Run strip on the final binary to remove debug symbols and reduce size.
RUN cd src && \
    qmake "CONFIG+=release" "QMAKE_CXXFLAGS+=-ffile-prefix-map=/app=." qdomyos-zwift.pro && \
    make -j$(nproc) && \
    strip qdomyos-zwift

# --- Stage 2: The Final Artifact ---
# Use a valid, non-empty base image to solve the "no command specified" error.
FROM debian:bookworm

# Copy the compiled binary from the builder stage
COPY --from=builder /app/src/qdomyos-zwift /qdomyos-zwift-x86-64-ant