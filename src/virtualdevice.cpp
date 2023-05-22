#include "virtualdevice.h"
#include <QSettings>

const QString virtualdevice::DomyosBridge = QStringLiteral("DomyosBridge");
const QString virtualdevice::VirtualDeviceBaseName = QStringLiteral("QZ");
const QString virtualdevice::VirtualDeviceAlternativeNameSuffixSettingsKey = QStringLiteral("virtual_device_alt_name_suffix");
const QString virtualdevice::VirtualDeviceAlternativeNameEnabledSettingsKey = QStringLiteral("virtual_device_alt_name_enabled");

QString virtualdevice::get_VirtualDeviceName() {
    QSettings settings;

    bool usingSuffix = settings.value(VirtualDeviceAlternativeNameEnabledSettingsKey, false).toBool();

    if(!usingSuffix)
        return DomyosBridge;

    QString suffix = settings.value(virtualdevice::VirtualDeviceAlternativeNameSuffixSettingsKey, "0" ).toString();

    return VirtualDeviceBaseName+suffix;
}

#include "virtualdevice.h"
#include <QDebug>

virtualdevice::virtualdevice(QObject *parent)
    : QObject{parent}
{

}

virtualdevice::~virtualdevice() {
    qDebug() << "Deleting virtual device";
}
