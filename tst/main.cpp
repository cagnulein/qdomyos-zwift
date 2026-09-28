#include <gtest/gtest.h>
#include <QApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QResource>
#include <QScopedPointer>
#include <QTimer>
#include "Devices/devicetestdataindex.h"

// https://forum.qt.io/topic/84229/is-there-a-canonical-way-to-set-up-qapplication-and-google-test-together/2

static int runSettingsLabelLayoutProbe(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);

    Q_INIT_RESOURCE(qml);

    QQmlEngine engine;
    QQmlComponent component(&engine, QUrl(QStringLiteral("qrc:/IndicatorOnlySwitch.qml")));
    if (component.isError())
        return 10;

    QScopedPointer<QObject> object(component.create());
    if (!object)
        return 11;

    object->setProperty("width", 280.0);
    object->setProperty(
        "text",
        QStringLiteral("Nutze Apple Watch Cadence für gefälschte Laufbandgeschwindigkeitsberechnung mit Donaudampfschifffahrtsgesellschaftskapitän"));

    // Let Component.onCompleted / Qt.callLater apply the wrapping settings and
    // let the Material IconLabel recalculate its implicit size.
    for (int i = 0; i < 3; ++i)
        app.processEvents();

    QObject *contentItem = object->property("contentItem").value<QObject *>();
    QObject *label = object->findChild<QObject *>(QStringLiteral("label"), Qt::FindChildrenRecursively);
    if (!contentItem || !label)
        return 12;

    const int lineCount = label->property("lineCount").toInt();
    if (lineCount != 2)
        return 13;

    if (!label->property("truncated").toBool())
        return 14;

    const qreal implicitHeight = object->property("implicitHeight").toReal();
    object->setProperty("height", implicitHeight);
    app.processEvents();

    const qreal contentWidth = contentItem->property("width").toReal();
    const qreal contentHeight = contentItem->property("height").toReal();
    const qreal labelX = label->property("x").toReal();
    const qreal labelY = label->property("y").toReal();
    const qreal labelWidth = label->property("width").toReal();
    const qreal labelHeight = label->property("height").toReal();

    constexpr qreal epsilon = 0.5;
    if (labelX < -epsilon || labelY < -epsilon ||
        labelX + labelWidth > contentWidth + epsilon ||
        labelY + labelHeight > contentHeight + epsilon)
        return 15;

    const qreal contentImplicitHeight = contentItem->property("implicitHeight").toReal();
    const qreal topPadding = object->property("topPadding").toReal();
    const qreal bottomPadding = object->property("bottomPadding").toReal();
    if (implicitHeight + epsilon < contentImplicitHeight + topPadding + bottomPadding)
        return 16;

    return 0;
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == QStringLiteral("--settings-label-layout-probe"))
            return runSettingsLabelLayoutProbe(argc, argv);
    }

    QCoreApplication app{argc, argv};

    DeviceTestDataIndex::Initialize();

    QTimer::singleShot(0, [&]()
                       {
                           ::testing::InitGoogleTest(&argc, argv);
                           auto testResult = RUN_ALL_TESTS();
                           app.exit(testResult);
                       });

    return app.exec();
}
