#include "virtualdevice.h"
#include "qzsettings.h"
#include <QSettings>
#include <QDebug>

const QString virtualdevice::DomyosBridge = QStringLiteral("DomyosBridge");
const QString virtualdevice::VirtualDeviceBaseName = QStringLiteral("QZ");

QString virtualdevice::get_VirtualDeviceName() {
    QSettings settings;

    bool usingSuffix = settings.value(QZSettings::virtual_device_alt_name_enabled, QZSettings::default_virtual_device_alt_name_enabled).toBool();

    if(!usingSuffix)
        return DomyosBridge;

    QString suffix = settings.value(QZSettings::virtual_device_alt_name_suffix, QZSettings::default_virtual_device_alt_name_suffix).toString();

    return VirtualDeviceBaseName+suffix;
}


virtualdevice::virtualdevice(QObject *parent)
    : QObject{parent}
{

}

virtualdevice::~virtualdevice() {
    qDebug() << "Deleting virtual device";
}
