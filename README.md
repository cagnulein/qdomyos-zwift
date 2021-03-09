# qdomyos-zwift
Zwift bridge for Treadmills and Bike!

[<img src="docs/google_play.png">](https://play.google.com/store/apps/details?id=org.cagnulen.qdomyoszwift&fbclid=IwAR3CVoYb0scvGf7gb0Y20VFh5Na5fDWwe7VACk-2c45Tm0x5s8sXpIGhGyw)
[<img src="docs/app_store.png">](https://apps.apple.com/app/id1543684531?fbclid=IwAR10H6y3mEgwkTlGJON3e8voYOh2wt3kLFOpFzoIXaYZ_N0y0pDvKxHMUaM)
<a href="https://www.buymeacoffee.com/cagnulein" target="_blank"><img src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png" alt="Buy Me A Coffee" style="height: 41px !important;width: 174px !important;box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;-webkit-box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;" ></a>

![UI](docs/treadmill-bridge-schema.png)

[![Video](https://img.youtube.com/vi/GgG3dMhmo2Y/0.jpg)](https://www.youtube.com/watch?v=GgG3dMhmo2Y)

![UI](docs/ui.png)
![UI](docs/realtime-chart.png)

UI on Linux

![UI](docs/ui-mac.png)

UI on MacOS

### Features

1. Domyos compatible
2. Toorx TRX Route Key compatible
3. Echelon Connect Sport compatible
4. Zwift compatible
5. Create, load and save train programs
6. Measure distance, elevation gain and watts
7. Gpx import (with difficulty slider)
8. Realtime Charts

![First Success](docs/first_success.jpg)

### Installation from source

$ sudo apt update && sudo apt upgrade # this is very important on raspberry pi: you need the bluetooth firmware updated!

$ sudo apt install git libqt5bluetooth5 libqt5widgets5 libqt5positioning5 libqt5xml5 qtconnectivity5-dev qtpositioning5-dev libqt5charts5-dev libqt5charts5 qt5-default libqt5networkauth5-dev

$ git clone https://github.com/cagnulein/qdomyos-zwift.git

$ cd src

$ qmake

$ make -j4

$ sudo ./qdomyos-zwift

### MacOs installation

You will need to (at a minimum) to install the xcode Command Line Tools (CLI) thanks to @richardwait
https://developer.apple.com/download/more/?=xcode

Download and install http://download.qt.io/official_releases/qt/5.12/5.12.9/qt-opensource-mac-x64-5.12.9.dmg and simply run the qdomyos-zwift relase for MacOs

### Tested on

- Raspberry PI 0W and Domyos Intense Run

- MacBook Air 2011 and Domyos Intense Run

- Raspberry 3b+ and Domyos T900C

- Raspberry 3b+ and Toorx TRX Route Key

- Android Pixel 2 and Echelon Connect Sport


### Your machine is not compatible?

Open an issue and follow these steps!

1. first of all you need an android device (phone or tablet)
2. you need to become developer on your phone https://wccftech.com/how-to/how-to-enable-developer-options-on-android-10-tutorial/
3. Go to Settings
4. Go into developer options
5. Enable the option Enable Bluetooth HCI snoop log
6. restart your phone
7. open your machine app and play with it collecting inclination and speed
8. Disable the option Enable Bluetooth HCI snoop log
9. in Developer Options: Bug report->Full report
10. wait a random amount of time (10-20 seconds)
11. A notification will appear at the top of the device. Click on it, share, email it to yourself
12. You'll get a zip file with the entire report. In the FS/Data/Log/bt directory of the zipfile is the file you want.
13. attach the log file in a new issue with a short description of the steps you did in the app when you used it

### No gui version

run as

$ sudo ./qdomyos-zwift -no-gui

### Reference

https://github.com/ProH4Ck/treadmill-bridge

https://www.livestrong.com/article/422012-what-is-10-degrees-in-incline-on-a-treadmill/

### Blog

https://robertoviola.cloud
