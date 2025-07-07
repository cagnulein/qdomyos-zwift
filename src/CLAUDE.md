when you add a setting remember:
- you have to add always as the last settings declared in the settings.qml
- if you have to add a setting also on another qml file, you need also to declare it there always putting as the last one
- in the qzsettings.cpp there is a allsettingscount that must be updated if you add a setting
