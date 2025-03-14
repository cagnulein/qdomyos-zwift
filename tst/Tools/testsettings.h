#ifndef TESTSETTINGS_H
#define TESTSETTINGS_H

#include <QString>
#include <QSettings>
#include <QCoreApplication>
#include "Devices/devicediscoveryinfo.h"

/**
 * @brief The TestSettings class stores a QSettings object with what is intended to be a unique
 * application and organisation name, to keep the configuration it represents separate from others
 * in the system. It also makes the stored QSettings object the default by setting the QCoreApplication's
 * organisation and application names to those of the QSettings object. The original values
 * are restored by calling the deactivate() function or on object destruction.
 */
class TestSettings
{
private:
    QString orgName=nullptr, appName=nullptr;
    bool active = false;
public:
    /**
     * @brief The underlying QSettings object.
     */
    QSettings qsettings;

    /**
     * @brief Constructor.
     * @param organisationName The organisation name to be used in combination with the application name to distinguish the QSettings content from others.
     * @param applicationName The application name to be used in combination with the organisation name to distinguish the QSettings content from others.
     */
    explicit TestSettings(const QString& organisationName, const QString& applicationName) : qsettings(organisationName, applicationName) {
        // TODO: make sure there's only ever 1 instance.
    }

    virtual ~TestSettings();

    /**
     * @brief Indicates if the default QSettings object accesses the same data as the one in this object.
     * @return
     */
    bool get_active() { return this->active; }

    /**
     * @brief Aligns the application and organisation names of the QCoreApplication with those of the test settings object.
     */
    void activate();

    /**
     * @brief Restores the original QCoreApplication application and organisation names.
     */
    void deactivate();

    /**
     * @brief Loads the QSettings from the specified object.
     * @param info The values to configure.
     * @param clear Specifies whether to clear the settings first.
     */
    void loadFrom(const DeviceDiscoveryInfo& info, bool clear=true);
};

#endif // TESTSETTINGS_H
