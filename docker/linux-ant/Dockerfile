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

# Instead of compiling qthttpserver, we copy the pre-built headers,
# exactly like the original linux-x86-build CI job.
RUN cp qHttpServerBin/5.15.2/headers/* src/qthttpserver/src/3rdparty/http-parser/

# Configure the project for an ANT+ build
RUN echo 'include(devices/antlinux/antlinux.pri)' >> src/qdomyos-zwift.pro && \
    echo '/opt/ant_venv/bin/python3' > src/.ant_venv_path

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
RUN mkdir -p /qdomyos-zwift-x86-64-ant

# Copy the compiled binary and wrapper script from the build context
COPY --from=builder /app/src/qdomyos-zwift-bin /qdomyos-zwift-x86-64-ant/
COPY docker/linux-ant/qdomyos-zwift-wrapper.sh /qdomyos-zwift-x86-64-ant/qdomyos-zwift
COPY src/devices/antlinux/setup-dashboard.sh /qdomyos-zwift-x86-64-ant/
COPY src/devices/antlinux/bt_provider.py /qdomyos-zwift-x86-64-ant/
COPY src/devices/antlinux/test_ant.py /qdomyos-zwift-x86-64-ant/
COPY src/devices/antlinux/ant_broadcaster.py /qdomyos-zwift-x86-64-ant/
COPY src/devices/antlinux/README.md /qdomyos-zwift-x86-64-ant/
COPY --from=builder /app/src/devices.ini /qdomyos-zwift-x86-64-ant/devices.ini
RUN chmod +x /qdomyos-zwift-x86-64-ant/qdomyos-zwift && \
    chmod +x /qdomyos-zwift-x86-64-ant/setup-dashboard.sh || true

# Set a no-op command (required for Docker image)
CMD ["/bin/bash"]