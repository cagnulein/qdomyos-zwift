#ifndef VIRTUALGEARINGDEVICE_H
#define VIRTUALGEARINGDEVICE_H

#include <QObject>
#include <QQmlEngine>

class VirtualGearingDevice : public QObject
{
    Q_OBJECT

public:
    explicit VirtualGearingDevice(QObject *parent = nullptr);
    static VirtualGearingDevice* instance();
    static void registerQmlType();

public slots:
    bool isAccessibilityServiceEnabled();
    void openAccessibilitySettings();
    void simulateShiftUp();
    void simulateShiftDown();
    void simulateTouch(int x, int y);
    bool isServiceRunning();
    QString getLastTouchCoordinates();
    QString getShiftUpCoordinates();
    QString getShiftDownCoordinates();

private:
    static VirtualGearingDevice* m_instance;
};

#endif // VIRTUALGEARINGDEVICE_H