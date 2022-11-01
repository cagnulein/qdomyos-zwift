#include <QSettings>
#include <QString>
#include "homeform.h"

#include "templatemanagers.h"

void templatemanagers::writeSettings(bool enabled) {
    QSettings settings;
    QString innerId = QStringLiteral("inner");
    QString sKey = QStringLiteral("template_") + innerId + QStringLiteral("_" TEMPLATE_PRIVATE_WEBSERVER_ID "_");
    settings.setValue(sKey + QStringLiteral("enabled"), enabled);
    settings.setValue(sKey + QStringLiteral("type"), TEMPLATE_TYPE_WEBSERVER);
    settings.setValue(sKey + QStringLiteral("port"), 0);
}

void templatemanagers::start(templatemanagers *managers, bluetoothdevice *device) {
    if(managers) managers->start(device);
}

void templatemanagers::stop(templatemanagers *managers) {
    if(managers) managers->stop();
}

templatemanagers::templatemanagers(QObject *parent) {
    if(!parent)
        std::invalid_argument("Parent must be specified.");

    QString innerId = QStringLiteral("inner");
    QString path = homeform::getWritableAppDir() + QStringLiteral("QZTemplates");
    this->userTemplateManager = TemplateInfoSenderBuilder::getInstance(QStringLiteral("user"), QStringList({path, QStringLiteral(":/templates/")}), parent);
    this->innerTemplateManager = TemplateInfoSenderBuilder::getInstance(innerId, QStringList({QStringLiteral(":/inner_templates/")}),parent);
}

void templatemanagers::start(bluetoothdevice *device){
    this->userTemplateManager->start(device);
    this->innerTemplateManager->start(device);
}

void templatemanagers::stop() {
    this->userTemplateManager->stop();
    this->innerTemplateManager->stop();
}

templatemanagers::~templatemanagers() {

    // Since the managers are owned by a QObject, let it delete them.
    /*
    if(this->userTemplateManager)
        delete this->userTemplateManager;
    if(this->innerTemplateManager)
        delete this->innerTemplateManager;
    this->userTemplateManager = nullptr;
    this->innerTemplateManager = nullptr;
*/
}

TemplateInfoSenderBuilder *templatemanagers::getUserTemplateManager() const { return this->userTemplateManager; }

TemplateInfoSenderBuilder *templatemanagers::getInnerTemplateManager() const { return this->innerTemplateManager; }

