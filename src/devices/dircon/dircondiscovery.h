#ifndef DIRCONDISCOVERY_H
#define DIRCONDISCOVERY_H

#include <QObject>
#include <QString>
#include <QList>
#include "qmdnsengine/browser.h"
#include "qmdnsengine/cache.h"
#include "qmdnsengine/resolver.h"
#include "qmdnsengine/server.h"

/**
 * @brief Structure to hold discovered DirCon device information
 */
struct DirconDeviceInfo {
    QString name;           // Device name (e.g., "KICKR-ABC123")
    QString displayName;    // Display name with (DirCon) suffix
    QString address;        // IP address
    quint16 port;          // TCP port
    QString macAddress;    // MAC address from TXT record
    QString serialNumber;  // Serial number from TXT record
    QString bleServiceUuids; // BLE service UUIDs from TXT record
    bool isTreadmill;      // true if treadmill, false if bike

    DirconDeviceInfo() : port(0), isTreadmill(false) {}

    bool operator==(const DirconDeviceInfo &other) const {
        return name == other.name;
    }
};

/**
 * @brief Class to discover Wahoo DirCon devices via mDNS
 *
 * This class uses QMdnsEngine to discover devices advertising
 * the "_wahoo-fitness-tnp._tcp.local." service type on the local network.
 */
class DirconDiscovery : public QObject {
    Q_OBJECT

public:
    explicit DirconDiscovery(QObject *parent = nullptr);
    ~DirconDiscovery();

    /**
     * @brief Start mDNS discovery for DirCon devices
     */
    void startDiscovery();

    /**
     * @brief Stop mDNS discovery
     */
    void stopDiscovery();

    /**
     * @brief Get list of discovered devices
     * @return List of DirconDeviceInfo structures
     */
    QList<DirconDeviceInfo> discoveredDevices() const { return devices; }

    /**
     * @brief Find device info by display name
     * @param displayName The display name to search for
     * @return Device info if found, null DirconDeviceInfo otherwise
     */
    DirconDeviceInfo findDeviceByName(const QString &displayName) const;

signals:
    /**
     * @brief Emitted when a new DirCon device is discovered
     * @param displayName Display name of the device (e.g., "KICKR ABC123 (DirCon)")
     */
    void deviceDiscovered(const QString &displayName);

    /**
     * @brief Emitted when a DirCon device is removed from the network
     * @param displayName Display name of the device
     */
    void deviceRemoved(const QString &displayName);

private slots:
    void onServiceAdded(const QByteArray &name);
    void onServiceUpdated(const QByteArray &name);
    void onServiceRemoved(const QByteArray &name);
    void onResolved(const QHostAddress &address, quint16 port);

private:
    QMdnsEngine::Server *mdnsServer;
    QMdnsEngine::Browser *mdnsBrowser;
    QMdnsEngine::Cache *mdnsCache;
    QMdnsEngine::Resolver *mdnsResolver;

    QList<DirconDeviceInfo> devices;

    /**
     * @brief Determine if device is a treadmill based on name
     * @param name Device name
     * @return true if name contains "tread" (case insensitive), false otherwise
     */
    bool isTreadmillDevice(const QString &name) const;

    /**
     * @brief Create display name with (DirCon) suffix
     * @param baseName Base device name
     * @return Display name with " (DirCon)" appended
     */
    QString createDisplayName(const QString &baseName) const;
};

#endif // DIRCONDISCOVERY_H
