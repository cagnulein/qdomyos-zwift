# =============================================================================
# Dockerfile for QDomyos-Zwift (Linux x86-64 with ANT+)
#
# This file defines a multi-stage build to compile the application for the
# linux/amd64 platform, mirroring the environment of the linux-x86-build job.
# =============================================================================

# --- Stage 1: The Builder ---
FROM ubuntu:latest AS builder

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Add deadsnakes PPA for Python 3.11 and install all necessary build dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends software-properties-common && \
    add-apt-repository ppa:deadsnakes/ppa -y && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
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
    # Python 3.11 for ANT+ support (from deadsnakes PPA)
    python3.11 \
    python3.11-dev \
    python3.11-venv \
    libusb-1.0-0-dev \
    libudev-dev && \
    rm -rf /var/lib/apt/lists/*

# Set up the Python virtual environment for ANT+ support
RUN python3.11 -m venv /opt/ant_venv && \
    /opt/ant_venv/bin/pip install --upgrade pip --quiet && \
    /opt/ant_venv/bin/pip install openant pyusb pybind11 --quiet

# Copy the entire project source into the container
WORKDIR /app
COPY . .

# Initialize only the needed submodules (avoid full recursive fetch)
RUN git submodule update --init src/smtpclient/ && \
    git submodule update --init src/qmdnsengine/ && \
    git submodule update --init src/qthttpserver/ && \
    git submodule update --init tst/googletest/

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
    strip qdomyos-zwift && \
    mv qdomyos-zwift qdomyos-zwift-bin

# --- Stage 2: Minimal Artifact Container ---
# Using busybox for minimal image size since files are only extracted, not run here.
# Users will run the binary on their host with system-installed dependencies.
FROM busybox:latest

# Create the application directory structure
RUN mkdir -p /qdomyos-zwift-x86-64-ant

# Copy the compiled binary and wrapper script from the build context
COPY --from=builder /app/src/qdomyos-zwift-bin /qdomyos-zwift-x86-64-ant/
COPY docker/linux-ant/qdomyos-zwift-wrapper.sh /qdomyos-zwift-x86-64-ant/qdomyos-zwift
COPY src/devices/antlinux/setup.sh /qdomyos-zwift-x86-64-ant/
COPY src/devices/antlinux/test_ant.py /qdomyos-zwift-x86-64-ant/
COPY src/devices/antlinux/ant_broadcaster.py /qdomyos-zwift-x86-64-ant/
RUN chmod +x /qdomyos-zwift-x86-64-ant/qdomyos-zwift && \
    chmod +x /qdomyos-zwift-x86-64-ant/setup.sh

# Set a no-op command (required for Docker image)
CMD ["/bin/bash"]