#include "dircondiscovery.h"
#include <QDebug>
#include <QHostAddress>

DirconDiscovery::DirconDiscovery(QObject *parent)
    : QObject(parent),
      mdnsServer(nullptr),
      mdnsBrowser(nullptr),
      mdnsCache(nullptr),
      mdnsResolver(nullptr) {
    qDebug() << "DirconDiscovery: initialized";
}

DirconDiscovery::~DirconDiscovery() {
    stopDiscovery();
}

void DirconDiscovery::startDiscovery() {
    qDebug() << "DirconDiscovery: starting mDNS discovery for Wahoo DirCon devices";

    // Initialize mDNS components if not already done
    if (!mdnsServer) {
        mdnsServer = new QMdnsEngine::Server(this);
    }

    if (!mdnsCache) {
        mdnsCache = new QMdnsEngine::Cache(this);
    }

    if (!mdnsBrowser) {
        mdnsBrowser = new QMdnsEngine::Browser(mdnsServer, "_wahoo-fitness-tnp._tcp.local.", mdnsCache);
        connect(mdnsBrowser, &QMdnsEngine::Browser::serviceAdded,
                this, &DirconDiscovery::onServiceAdded);
        connect(mdnsBrowser, &QMdnsEngine::Browser::serviceUpdated,
                this, &DirconDiscovery::onServiceUpdated);
        connect(mdnsBrowser, &QMdnsEngine::Browser::serviceRemoved,
                this, &DirconDiscovery::onServiceRemoved);
    }

    qDebug() << "DirconDiscovery: mDNS browser started for _wahoo-fitness-tnp._tcp.local.";
}

void DirconDiscovery::stopDiscovery() {
    qDebug() << "DirconDiscovery: stopping mDNS discovery";

    // Clean up mDNS components
    if (mdnsResolver) {
        mdnsResolver->deleteLater();
        mdnsResolver = nullptr;
    }

    if (mdnsBrowser) {
        mdnsBrowser->deleteLater();
        mdnsBrowser = nullptr;
    }

    if (mdnsCache) {
        mdnsCache->deleteLater();
        mdnsCache = nullptr;
    }

    if (mdnsServer) {
        mdnsServer->deleteLater();
        mdnsServer = nullptr;
    }

    devices.clear();
}

void DirconDiscovery::onServiceAdded(const QMdnsEngine::Service &service) {
    qDebug() << "DirconDiscovery: service added:" << service.name() << service.hostname() << service.port();

    // Save the service and create resolver to get IP address
    currentService = service;

    if (mdnsResolver) {
        mdnsResolver->deleteLater();
    }

    mdnsResolver = new QMdnsEngine::Resolver(mdnsServer, service.hostname(), mdnsCache, this);
    connect(mdnsResolver, &QMdnsEngine::Resolver::resolved,
            this, &DirconDiscovery::onResolved);
}

void DirconDiscovery::onServiceUpdated(const QMdnsEngine::Service &service) {
    qDebug() << "DirconDiscovery: service updated:" << service.name();
    // Handle updates similarly to additions
    onServiceAdded(service);
}

void DirconDiscovery::onServiceRemoved(const QMdnsEngine::Service &service) {
    qDebug() << "DirconDiscovery: service removed:" << service.name();

    // Find and remove device from list
    QString deviceName = QString::fromUtf8(service.name());
    for (int i = 0; i < devices.size(); ++i) {
        if (devices[i].name == deviceName) {
            QString displayName = devices[i].displayName;
            devices.removeAt(i);
            emit deviceRemoved(displayName);
            qDebug() << "DirconDiscovery: removed device" << displayName;
            break;
        }
    }
}

void DirconDiscovery::onResolved(const QHostAddress &address) {
    qDebug() << "DirconDiscovery: resolved address" << address.toString();

    // Extract device information from currentService
    QString name = QString::fromUtf8(currentService.name());
    quint16 port = currentService.port();

    // Check if device already exists
    bool exists = false;
    for (const DirconDeviceInfo &dev : devices) {
        if (dev.name == name) {
            exists = true;
            break;
        }
    }

    if (exists) {
        qDebug() << "DirconDiscovery: device" << name << "already in list, skipping";
        return;
    }

    // Create new device info
    DirconDeviceInfo deviceInfo;
    deviceInfo.name = name;
    deviceInfo.address = address.toString();
    deviceInfo.port = port;

    // Extract TXT record attributes from Service
    QHash<QByteArray, QByteArray> attributes = currentService.attributes();
    if (attributes.contains("mac-address")) {
        deviceInfo.macAddress = QString::fromUtf8(attributes.value("mac-address"));
    }
    if (attributes.contains("serial-number")) {
        deviceInfo.serialNumber = QString::fromUtf8(attributes.value("serial-number"));
    }
    if (attributes.contains("ble-service-uuids")) {
        deviceInfo.bleServiceUuids = QString::fromUtf8(attributes.value("ble-service-uuids"));
    }

    // Determine device type (treadmill or bike)
    deviceInfo.isTreadmill = isTreadmillDevice(name);

    // Create display name
    deviceInfo.displayName = createDisplayName(name);

    // Add to list
    devices.append(deviceInfo);

    qDebug() << "DirconDiscovery: discovered device"
             << "name:" << deviceInfo.name
             << "displayName:" << deviceInfo.displayName
             << "address:" << deviceInfo.address
             << "port:" << deviceInfo.port
             << "isTreadmill:" << deviceInfo.isTreadmill
             << "mac:" << deviceInfo.macAddress
             << "serial:" << deviceInfo.serialNumber
             << "uuids:" << deviceInfo.bleServiceUuids;

    // Emit signal
    emit deviceDiscovered(deviceInfo.displayName);
}

DirconDeviceInfo DirconDiscovery::findDeviceByName(const QString &displayName) const {
    for (const DirconDeviceInfo &dev : devices) {
        if (dev.displayName == displayName) {
            return dev;
        }
    }
    return DirconDeviceInfo(); // Return empty struct if not found
}

bool DirconDiscovery::isTreadmillDevice(const QString &name) const {
    // Check if name contains "tread" (case insensitive)
    return name.toLower().contains("tread");
}

QString DirconDiscovery::createDisplayName(const QString &baseName) const {
    // Format: "Device Name (DirCon)"
    return baseName + QStringLiteral(" (DirCon)");
}
