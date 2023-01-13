# Applications supported

This list is not exhaustive ! 
Open a discussion to tell us if a missing application is supported.

## BKool
Everything is working out of the box (read FTMS data).


## Fulgaz

### HRM Support
The application do not read the FTMS value. It is required to start the application with `-heart-service` or `bike_heartrate_service: true` in settings.
The HRM captor will not be shown by default, you need to go in the `Application Settings > Advanced > Disable Bluetooth Filter`.

### Resistance management
Fulgaz is known to be very severe by default in resistance adjustment. It is advised to adjust [application settings](https://tempocyclist.com/2020/04/29/fulgaz-resistance-too-hard/).
The resistance is automatically adjusted with the slope. However, you can override it using 

### Additional notes 
You can have ae true 4k video stream while you ride ("extreme quality" setting) however it requires about 10 gb per hour.


## Zwift
### HRM support
The application do not read the FTMS value. It is required to start the application with `-heart-service` or `bike_heartrate_service: true` in settings.

### Resistance management
You can adjust resistance using arrows [up and down](img/21_zwift-resistance-buttons.jpg) rom the riding screen.

