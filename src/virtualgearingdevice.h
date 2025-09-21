#ifndef VIRTUALGEARINGDEVICE_H
#define VIRTUALGEARINGDEVICE_H

#include <QObject>

class VirtualGearingDevice : public QObject
{
    Q_OBJECT

public:
    explicit VirtualGearingDevice(QObject *parent = nullptr);
    static VirtualGearingDevice* instance();

public slots:
    bool isAccessibilityServiceEnabled();
    void openAccessibilitySettings();
    void simulateShiftUp();
    void simulateShiftDown();
    void simulateTouch(int x, int y);
    bool isServiceRunning();

private:
    static VirtualGearingDevice* m_instance;
};

#endif // VIRTUALGEARINGDEVICE_H