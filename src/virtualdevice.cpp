#include "virtualdevice.h"
#include <QSettings>

const QString virtualdevice::VirtualDeviceBaseName = QStringLiteral("DomyosBridge");
const QString virtualdevice::VirtualDeviceNameSuffixSettingsKey = QStringLiteral("virtual_device_name_suffix");
const QString virtualdevice::VirtualDeviceNameSuffixEnabledSettingsKey = QStringLiteral("virtual_device_name_suffix_enabled");

QString virtualdevice::get_VirtualDeviceName() {
    QSettings settings;

    bool usingSuffix = settings.value(VirtualDeviceNameSuffixEnabledSettingsKey, false).toBool();

    if(!usingSuffix)
        return VirtualDeviceBaseName;

    int suffixValue = settings.value(virtualdevice::VirtualDeviceNameSuffixSettingsKey, 0 ).toInt();

    QString suffix = QString(suffixValue);

    while(suffix.length()<4)
        suffix = "0"+suffix;

    return VirtualDeviceBaseName+"-"+suffix;
}

