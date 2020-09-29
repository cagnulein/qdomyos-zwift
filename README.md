# qdomyos-zwift
Zwift bridge for Domyos treadmills

<a href="https://www.buymeacoffee.com/cagnulein" target="_blank"><img src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png" alt="Buy Me A Coffee" style="height: 41px !important;width: 174px !important;box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;-webkit-box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;" ></a>

![First Success](docs/first_success.jpg)

### Installation

$ git clone https://github.com/cagnulein/qdomyos-zwift.git

$ sudo apt upgrade && sudo apt update # this is very important on raspberry pi: you need the bluetooth firmware updated!

$ sudo apt install libqt5bluetooth5

$ sudo hciconfig hci0 leadv 0

$ sudo ./qdomyos-zwift

### Tested on

Raspberry PI 0W and Domyos Intense Run

### Reference

https://github.com/ProH4Ck/treadmill-bridge
https://www.livestrong.com/article/422012-what-is-10-degrees-in-incline-on-a-treadmill/

### Blog

https://robertoviola.cloud
