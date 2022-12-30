#ifndef TESTSETTINGS_H
#define TESTSETTINGS_H

#include <QString>
#include <QSettings>
#include <QCoreApplication>

class TestSettings
{
private:
    QString orgName=nullptr, appName=nullptr;
    bool active = false;
public:
    QSettings qsettings;

    TestSettings(const QString& organisationName, const QString& applicationName) : qsettings(organisationName, applicationName) {
        // TODO: make sure there's only ever 1 instance.
    }

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

    virtual ~TestSettings();

};

#endif // TESTSETTINGS_H
