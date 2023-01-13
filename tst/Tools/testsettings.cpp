#include "testsettings.h"

void TestSettings::activate() {
    if(this->active) return;

    this->orgName = QCoreApplication::organizationName();
    this->appName = QCoreApplication::applicationName();

    QCoreApplication::setApplicationName(this->qsettings.applicationName());
    QCoreApplication::setOrganizationName(this->qsettings.organizationName());

    this->active = true;
}

void TestSettings::deactivate() {
    if(!this->active) return;

    QCoreApplication::setApplicationName(this->appName);
    QCoreApplication::setOrganizationName(this->orgName);

    this->active = false;
}

void TestSettings::loadFrom(const DeviceDiscoveryInfo &info, bool clear){ info.setValues(this->qsettings, clear);}

TestSettings::~TestSettings() {
    this->deactivate();
}
