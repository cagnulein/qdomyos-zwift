# QDomyos-Zwift operation guide


# Usage

The QDomyos-Zwift can be started in two modes : QML or NativeQT.  
The main difference is the configuration management : you can change settings within the application with QML, where you need to specify settings at startup in NativeQT.

**Note:** Android and IOS are always running in QML mode.

On MacOS and Linux, you start QDomyos-Zwift in NativeQT mode (where settings are defined in commandline switches).

You can start the application in QML mode with the command-line switch -qml.

## Configuration in QML mode 
Please refer to this article for more information under [QML Operations](https://robertoviola.cloud/qdomyos-zwift-guide/) with several useful information.


## Configuration in NativeQT mode

This is the list of settings available in the application. These settings need to be appended to the binary command line.  
*Example :* `sudo ./qdomyos-zwift -no-gui` for disabling any graphical interface.

| **Option**              		| **Type** | **Default** | **Function**                                                                 |
|:------------------------------|:---------|:------------|:-----------------------------------------------------------------------------|
| -no-gui                 		| Boolean  | False       | Disable GUI                                                                  |
| -qml                    		| Boolean  | True        | Enables the QML interface                                                    |
| -noqml                    	| Boolean  | False       | Enables the NativeQT interface                                               |
| -miles                  		| Boolean  | False       | Switches to Imperial Units System                                             |
| -no-console             		| Boolean  | False       | Not in use                                                                   |
| -test-resistance        		| Boolean  | False       |                                                                              |
| -no-log                 		| Boolean  | False       | Disable Logging                                                              |
| -no-write-resistance    		| Boolean  | False       | Disable resistance instructions from QZ to your fitness equipment            |
| -no-heart-service       		| Boolean  | False       | Do not simulate external HR monitor, use only FTMS                           |
| -heart-service          		| Boolean  | True        | Simulate HR service (required for applications not reading FTMS)             |
| -only-virtualbike       		| Boolean  | False       |                                                                              |
| -only-virtualtreadmill  		| Boolean  | False       |                                                                              |
| -no-reconnection        		| Boolean  | False       | QZ will not try to reconnect your fitness equipment if enabled              |
| -bluetooth-relaxed      		| Boolean  | False       | In case of deconnections from QZ to your fitness equipment                  |
| -bike-cadence-sensor    		| Boolean  | False       |                                                                              |
| -bike-power-sensor      		| Boolean  | False       |                                                                              |
| -battery-service        		| Boolean  | False       |                                                                              |
| -service-changed        		| Boolean  | False       |                                                                              |
| -bike-wheel-revs        		| Boolean  | False       |                                                                              |
| -run-cadence-sensor     		| Boolean  | False       |                                                                              |
| -nordictrack-10-treadmill  	| Boolean  | False       | Enable NordicTrack compatibility mode                                        |
| -train                  		| String   |             | Force training program                                                       |
| -name                   		| String   |             | Force bluetooth device name (if QZ struggles to find your fitness equipment) |
| -poll-device-time       		| Int      | 200 (ms)    | Frequency to refresh information from QZ to Fitness equipment               |
| -bike-resistance-gain   		| Int      |             | Adjust resistance from the fitness application                               |
| -bike-resistance-offset 		| Int      |             | Set another resistance point than default                                    |



