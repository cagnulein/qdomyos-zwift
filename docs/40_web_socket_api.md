# QDomyos-Zwift WebSocket API Installation & Operation guide

# Installation
## About

The QDomyos-Zwift WebSocket API can be installed from source on Linux, Raspberry Pi (4, 3, zero W), macOS, Android and IOS.

However, this guide will only focus on the Linux (Debian 11) Installation and Raspberry Pi cause there are the most useful case in headless control.

If you already install the Web Socket, feel free to [skip to the Usage section](#usage).

## Requirement

To Install QDomyos-Zwift with WebSocket API you will need Qt 5.12.2+ and the following modules :
- Qt Bluetooth
- Qt Widgets
- Qt Positioning
- Qt XML
- Qt Charts
- Qt Network
- Qt Network Authorization
- Qt WebSockets
- Qt Assistant

Unfortunately under Debian 11 (or Raspbian 11) the Qt 5 packages are not recent enough for compilation however this guide will explain how to manually compile the latest version of Qt (5.12.12)

If you already had Qt 5.12.2 or more, feel free to [skip to Install Qt Httpserver](#install-qt-httpserver).

## Install Qt 5.12.2

*If you compile for a Raspberry Pi Zero, it's* ***faster and easy*** *to do all the Raspberry Pi task on a Raspberry Pi 4 and after copy compiled binary files toe the Raspberry Pi Zero*

For more info on the steps [please refer to the source](#source)

Before do anything. Make sure all your packages are updated :

```bash
apt update && apt upgrade # this is very important on raspberry pi: you need the bluetooth firmware updated!
```

After download last version of Qt Source and extract them :
```bash
wget https://download.qt.io/official_releases/qt/5.12/5.12.12/single/qt-everywhere-src-5.12.12.tar.xz
```

If you compile for a Raspberry Pi you will need the Raspberry Pi Qt Configuration for raspberry pi and install it in the source :

```bash
git clone https://github.com/oniongarlic/qt-raspberrypi-configuration.git
cd qt-raspberrypi-configuration && make install DESTDIR=../qt-everywhere-src-5.12.12
```

Install the bare minimum required development packages for building Qt 5 with apt :
```bash
apt install build-essential libfontconfig1-dev libdbus-1-dev libfreetype6-dev libicu-dev libinput-dev libxkbcommon-dev libsqlite3-dev libssl-dev libpng-dev libjpeg-dev libglib2.0-dev libraspberrypi-dev
```

*For raspberry Pi install `libraspberrypi-dev` package* :
```bash
apt install libraspberrypi-dev
```


Now install all required development packages for building all Qt 5 modules:
```bash
apt install bluez libgbm-dev
apt install libudev-dev libinput-dev libts-dev libxcb-xinerama0-dev libxcb-xinerama0 gdbserver
apt install libegl1-mesa libegl1-mesa-dev libgles2-mesa libgles2-mesa-dev
apt install wiringpi libnfc-bin libnfc-dev fonts-texgyre libts-dev
apt install libbluetooth-dev bluez-tools gstreamer1.0-plugins* libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libopenal-data libopenal1 libopenal-dev pulseaudio
apt install libgstreamer*-dev
apt install gstreamer*-dev
apt install libasound2-dev libavcodec-dev libavformat-dev libswscale-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev gstreamer-tools libgstreamer-plugins-*
apt install qtdeclarative5-dev
apt install libvlc-dev
```

On Raspbian Stretch/Buster/Bullseye the OpenGL library files have been renamed so that they wouldn't conflict with Mesa installed ones. Unfortunately Qt configure script is still looking for the old names.
So ***on your target Raspberry Pi*** you need to symlink those file to make sure Qt run correctly.
```bash
ln -s /usr/lib/arm-linux-gnueabihf/libGLESv2.so /usr/lib/libbrcmGLESv2.so
ln -s /usr/lib/arm-linux-gnueabihf/libEGL.so /usr/lib/libbrcmEGL.so
```

Now all dependency are installed. It's time to create build folder and compiled.
```bash
mkdir build
cd build

# For Raspberry Pi Zero or 3
PKG_CONFIG_LIBDIR=/usr/lib/arm-linux-gnueabihf/pkgconfig:/usr/share/pkgconfig ../qt-everywhere-src-5.12.12/configure -platform linux-rpi-g++ -v -opengl es2 -eglfs -no-gtk -opensource -confirm-license -release -reduce-exports -force-pkg-config -nomake examples -no-compile-examples -skip qtwayland -skip qtwebengine -no-feature-geoservices_mapboxgl -qt-pcre -no-pch -ssl -evdev -system-freetype -fontconfig -glib -prefix /opt/Qt/5.12.12 -qpa eglfs
CFLAGS="-march=armv6zk -mtune=arm1176jzf-s -mfpu=vfp" make -j3 # Remove -j3 if you compiled directly on Raspberry Pi Zero

# For Raspberry Pi 4
PKG_CONFIG_LIBDIR=/usr/lib/arm-linux-gnueabihf/pkgconfig:/usr/share/pkgconfig ../qt-everywhere-src-5.12.12/configure -platform linux-rpi4-v3d-g++ -v -opengl es2 -eglfs -no-gtk -opensource -confirm-license -release -reduce-exports -force-pkg-config -nomake examples -no-compile-examples -skip qtwayland -skip qtwebengine -no-feature-geoservices_mapboxgl -qt-pcre -no-pch -ssl -evdev -system-freetype -fontconfig -glib -prefix /opt/Qt/5.12.12 -qpa eglfs
CFLAGS="-march=armv8-a -mtune=cortex-a72 -mfpu=crypto-neon-fp-armv8" make -j3

# For Debian 11 x64 (Not tested)
../qt-everywhere-src-5.12.12/configure -v -opengl es2 -eglfs -no-gtk -opensource -confirm-license -release -reduce-exports -force-pkg-config -nomake examples -no-compile-examples -skip qtwayland -skip qtwebengine -no-feature-geoservices_mapboxgl -qt-pcre -no-pch -ssl -evdev -system-freetype -fontconfig -glib -prefix /opt/Qt/5.12.12 -qpa eglfs
make
```

Finally, if you cross compiled you can transfer the build folder to other machine and then just run as root in the build folder :
```bash
make install
```

# Install Qt Httpserver

Like explain in PR #252, to make work the Http Server you will need to manually compile `qthttpserver` module.

For that just run following commands in your home directory :
```bash
cd ~
git clone https://github.com/qt-labs/qthttpserver
cd ~/qthttpserver/src/3rdparty/http-parser
wget https://raw.githubusercontent.com/nodejs/http-parser/main/http_parser.h
wget https://raw.githubusercontent.com/nodejs/http-parser/main/http_parser.c
cd ~/qthttpserver/src
qmake # Please note if you compiled Qt you need to specify /opt/Qt/5.12.12/bin/qmake
make
# Wait...
sudo make install
```

***You have successfully installed Qt Httpserver***

# Install QDomyos-Zwift

If you already compile QDomyos-Zwift and you just compiled a new version of Qt.
Please delete the whole QDomyos-Zwift folder and restart from scratch to prevent linking issues.

```bash
cd ~
git clone https://github.com/cagnulein/qdomyos-zwift.git
cd ~/qdomyos-zwift
git submodule update --init ~/qdomyos-zwift/src/smtpclient/
cd ~/qdomyos-zwift/src
qmake # Please note if you compiled Qt you need to specify /opt/Qt/5.12.12/bin/qmake
make -j4 # Remove -j4 if you compiled on Raspberry Pi Zero
```

Now installed you need to compile like say in PR #252 and issue #572 template/debug in the same directory of source file of QDomyos-Zwift.
```bash
cp -r ~/qdomyos-zwift/src/templates/debug ~/qdomyos-zwift/src/.
cp -r ~/qdomyos-zwift/src/templates/debug/* ~/qdomyos-zwift/src/.
```

Last if you can't run QML version (probably because you don't had a X11 Server.) you need to manually edit the configuration file in `/root/.config/Roberto Viola/qDomyos-Zwift.conf` and add :
```
template_inner_QZWS_enabled=true
template_inner_QZWS_folders=:/inner_templates//chartjs
template_inner_QZWS_ips=192.168.1.42
template_inner_QZWS_port=34107
template_inner_QZWS_type=WebServer
```

In this config file we open an HTTP Server on port 34107 with bind to 192.168.1.42 but feel free to change these values.

Finally, ***do not move `qdomyos-zwift` from src folder*** and run it as Root

# Usage

The way that [WebSocket](https://developer.mozilla.org/docs/Web/API/WebSockets_API) work in QDomyos-Zwift is by sending commands and listen events.

## Workout Event

The workout Event is the default message send almost every second by QDomyos-Zwift to inform you which state is your equipment.

Here what is look like :
```json
{
  "BIKE_TYPE": 2,
  "ELLIPTICAL_TYPE": 4,
  "ROWING_TYPE": 3,
  "TREADMILL_TYPE": 1,
  "UNKNOWN_TYPE": 0,
  "deviceId": "0B:54:49:D1:BC:DA",
  "deviceName": "Domyos-TC-0314",
  "deviceRSSI": 0,
  "deviceType": 1,
  "deviceConnected": false,
  "devicePaused": false,
  "elapsed_s": 0,
  "elapsed_m": 0,
  "elapsed_h": 0,
  "pace_s": 0,
  "pace_m": 0,
  "pace_h": 0,
  "moving_s": 0,
  "moving_m": 0,
  "moving_h": 0,
  "speed": 0,
  "speed_avg": 0,
  "calories": 0,
  "distance": 0,
  "heart": 0,
  "heart_avg": 0,
  "heart_max": 0,
  "jouls": 0,
  "elevation": 0,
  "difficult": 1,
  "watts": 0,
  "watts_avg": 0,
  "watts_max": 0,
  "kgwatts": 0,
  "kgwatts_avg": 0,
  "kgwatts_max": 0,
  "workoutName": "",
  "workoutStartDate": "",
  "instructorName": "",
  "latitude": null,
  "longitude": null,
  "nickName": "N/A",
  "inclination": 0,
  "inclination_avg": 0
}
```

## Commands

To send commands you will need to send a socket message in JSON format like : 
```json
{
  "msg": "pause"
}
```

which `msg` is always the name of the command. Command also return on WebSocket message like to acknowledge command :
```json
{
  "msg": "R_pause"
}
```

Here is a list of the most "useful" commands

### Start
#### Description :
Allows you to start the bike / treadmill (Reset Timer if bike / treadmill is stopped)

#### Send :
```json
{
  "msg": "start"
}
```
#### Response :
```json
{
  "msg": "R_start"
}
```

### Pause
#### Description :
Allows you to stop (pause) the bike / treadmill without reset timer.

#### Send :
```json
{
  "msg": "pause"
}
```
#### Response :
```json
{
  "msg": "R_pause"
}
```

### Stop
#### Description :
Allows you to stop the bike / treadmill and reset timer.

#### Send :
```json
{
  "msg": "stop"
}
```
#### Response :
```json
{
  "msg": "R_stop"
}
```

### SetSpeed
#### Description :
Allows you to control the treadmill speed.

#### Send :
```json
{
  "msg": "setspeed",
  "content": {
    "value": 8.0
  }
}
```
#### Response :
```json
{
  "msg": "R_setspeed",
  "content": {
    "value": 8.0
  }
}
```

### SetResistance
#### Description :
Allows you to control the resistance bike or the treadmill incline.

#### Send :
```json
{
  "msg": "setresistance",
  "content": {
    "value": 8.0
  }
}
```
#### Response :
```json
{
  "msg": "R_setresistance",
  "content": {
    "value": 8.0
  }
}
```

### SetFanSpeed
#### Description :
Allows you to control the fan bike / treadmill speed.

#### Send :
```json
{
  "msg": "setfanspeed",
  "content": {
    "value": 8.0
  }
}
```
#### Response :
```json
{
  "msg": "R_setfanspeed",
  "content": {
    "value": 8.0
  }
}
```

# Source
How compile Qt 5.12.10 on Raspberry Pi : https://www.tal.org/tutorials/building-qt-512-raspberry-pi

How cross compile Qt 5.12.5 on Raspberry Pi (in French) : https://wiki.logre.eu/index.php/Cross-compilation_Qt_5.12.5_pour_Raspberry_Pi

Issue [REQ] Add to qdomyos an API for remote access to treadmill #572

PR "Templated" connections and Web server #252
