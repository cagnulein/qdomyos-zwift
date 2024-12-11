# Define build image
FROM ubuntu:latest AS build

# Install essential build dependencies
ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && apt upgrade -y \
    && apt install --no-install-recommends -y \
        git \
        ca-certificates \
        qtquickcontrols2-5-dev \
        qtconnectivity5-dev \
        qtbase5-private-dev \
        qtpositioning5-dev \
        libqt5charts5-dev \
        libqt5networkauth5-dev \
        libqt5websockets5-dev \
        qml-module* \
        libqt5texttospeech5-dev \
        qtlocation5-dev \
        qtmultimedia5-dev \
        g++ \
        make \
        wget \
        unzip \
    && rm -rf /var/lib/apt/lists/*


# Define runtime image        
FROM ubuntu:latest AS runtime

# Install essential runtime dependencies
ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && apt upgrade -y \
    && apt install --no-install-recommends -y \
        libqt5bluetooth5 \
        libqt5widgets5 \
        libqt5positioning5 \
        libqt5xml5 \
        libqt5charts5 \
        qt5-assistant \
        libqt5networkauth5 \
        libqt5websockets5 \
        qml-module* \
        libqt5texttospeech5 \
        libqt5location5-plugins \
        libqt5multimediawidgets5 \
        libqt5multimedia5-plugins \
        libqt5multimedia5 \
        qml-module-qtquick-controls2 \
        libqt5location5 \
        bluez \
        dbus \
        tigervnc-standalone-server \
        tigervnc-tools \
        libgl1-mesa-dri \
        xfonts-base \
        x11-xserver-utils \
        tigervnc-common \
        net-tools \
    && rm -rf /var/lib/apt/lists/*


# Stage 1: Build
FROM build AS builder

# Clone the project and build it
WORKDIR /usr/local/src
RUN git clone --recursive https://github.com/cagnulein/qdomyos-zwift.git
WORKDIR /usr/local/src/qdomyos-zwift
RUN git submodule update --init src/smtpclient/ \
    && git submodule update --init src/qmdnsengine/ \
    && git submodule update --init tst/googletest/
WORKDIR /usr/local/src/qdomyos-zwift/src
RUN qmake qdomyos-zwift.pro \
    && make -j4


# Stage 2: Runtime
FROM runtime

# Copy the built binary to /usr/local/bin
COPY --from=builder /usr/local/src/qdomyos-zwift/src/qdomyos-zwift /usr/local/bin/qdomyos-zwift

# VNC configuration
RUN mkdir -p ~/.vnc && \
    echo "securepassword" | vncpasswd -f > ~/.vnc/passwd && \
    chmod 600 ~/.vnc/passwd

# .Xauthority configuration
RUN touch /root/.Xauthority
ENV DISPLAY=:99

# Start VNC server with authentication
CMD vncserver :99 -depth 24 -localhost no -xstartup qdomyos-zwift && \
    sleep infinity

