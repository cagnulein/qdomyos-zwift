# Supported applications

QDomyos-Zwift should support every application able to read a [FTMS](specs/FTMS_v1.0.pdf) bluetooth signal.

This list is not exhaustive. Please report any application known to be working with QDomyos-Zwift !

|Application|Sport|Platform|Speed|RPM|Power|HRM |Resistance Control|
|-----------|-----|--------|------------|---|-----|-----|----------------|
|[Zwift](21_applications_detail.md#zwift)|![bike](img/20_bike.png) ![run](img/20_treadmill.png)|![IOS](img/20_apple.png) ![Android](img/20_android.png) ![PC](img/20_windows.png)| Yes|Yes|Yes| Yes, no FTMS support | Yes |
|[BKool](21_applications_detail.md#bkool) |![bike](img/20_bike.png)|![IOS](img/20_apple.png) ![Android](img/20_android.png) ![PC](img/20_windows.png)|Yes|Yes|Yes|Yes| Yes |
|[Fulgaz](21_applications_detail.md#fulgaz)|![bike](img/20_bike.png)|![IOS](img/20_apple.png) ![Android](img/20_android.png) ![PC](img/20_windows.png)|Yes|Yes|Yes|Yes, no FTMS support (see note)|Yes (see note) |

# Supported devices
Check the full list https://github.com/cagnulein/qdomyos-zwift/wiki/Equipment-Compatibility

This list is not exhaustive. 
Try the qdomyos app with your fitness appliance and report how it is going.
If it's not working, you can [ask for your device to be supported](#ask-for-device-support)

# Ask for device support

You can ask for supporting a device by opening an issue and following these steps.
We need to "spy" the bluetooth activity from your fitness device and it's application, in order to guess how they communicate.

An android device is required for this operation.

## Android device 

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
11. A notification will appear at the top of the device. Click on it, share, email it to yourself. If it doesn't appear you need to use ADB to pull the file from the phone itself
12. You'll get a zip file with the entire report. In the FS/Data/Log/bt directory of the zipfile is the file you want.
13. attach the log file in a new issue with a short description of the steps you did in the app when you used it

## Android Device (oppo based OS : oppo, ColorOS, RealMe, ...)
1. Dial *#800# on the phone app. A special menu should appear.
2. Go to Bluetooth and press "Start capture" (green button)
3. open your machine app and play with it collecting inclination and speed
4. Go back to the special menu by dialing *#800# on the phone app. Stop the bluetooth capture.
5. A new debug directory is stored onto `/oppo_log/` with a time stamp.
6. Fetch the CFA file stored in the `btsnoop_hci` folder.
