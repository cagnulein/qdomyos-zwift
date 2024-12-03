FROM ubuntu:latest

# Install essential dependencies
ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && apt upgrade -y \
    && apt install -y \
        git \
        qtquickcontrols2-5-dev \
        libqt5bluetooth5 \
        libqt5widgets5 \
        libqt5positioning5 \
        libqt5xml5 \
        qtconnectivity5-dev \
        qtbase5-private-dev \
        qtpositioning5-dev \
        libqt5charts5-dev \
        libqt5charts5 \
        qt5-assistant \
        libqt5networkauth5-dev \
        libqt5websockets5-dev \
        qml-module* \
        libqt5texttospeech5-dev \
        libqt5texttospeech5 \
        libqt5location5-plugins \
        qtlocation5-dev \
        qtmultimedia5-dev \
        libqt5multimediawidgets5 \
        libqt5multimedia5-plugins \
        libqt5multimedia5 \
        g++ \
        make \
        bluez \
        dbus \
        xfce4 \
        tightvncserver \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get purge --auto-remove \
    && apt-get clean

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

# Configure VNC
RUN mkdir -p /root/.vnc \
    && echo "password" | vncpasswd -f > /root/.vnc/passwd \
    && chmod 600 /root/.vnc/passwd

# Start VNC and the application
CMD tightvncserver && ./qdomyos-zwift
