#ifndef TEMPLATEMANAGERS_H
#define TEMPLATEMANAGERS_H

#include "templateinfosenderbuilder.h"
#include "bluetoothdevice.h"

class templatemanagers {
private:
    TemplateInfoSenderBuilder *userTemplateManager = nullptr;
    TemplateInfoSenderBuilder *innerTemplateManager = nullptr;
public:
    static void writeSettings(bool enabled);

    /**
     * @brief Static method to start the managers on the specified device, if the managers object is not null.
     * @param managers
     * @param device
     */
    static void start(templatemanagers * managers, bluetoothdevice * device);

    /**
     * @brief Static method to stop the managers if not null.
     * @param managers
     */
    static void stop(templatemanagers * managers);

    explicit templatemanagers(QObject * parent);
    virtual ~templatemanagers();

    TemplateInfoSenderBuilder *getUserTemplateManager() const;
    TemplateInfoSenderBuilder *getInnerTemplateManager() const;

    void start(bluetoothdevice * device);
    void stop();


};

#endif // TEMPLATEMANAGERS_H
