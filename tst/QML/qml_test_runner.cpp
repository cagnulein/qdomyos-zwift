#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QQmlContext>
#include <QCoreApplication>
#include <QDir>

class TestSetup : public QObject {
    Q_OBJECT
public:
    TestSetup() {}

public slots:
    void qmlEngineAvailable(QQmlEngine *engine) {
        // Get test data directory from environment variable or use default
        QString testDataDir = qgetenv("QML_TEST_DATA_DIR");

        if (testDataDir.isEmpty()) {
            // Use path relative to executable location
            testDataDir = QCoreApplication::applicationDirPath() + "/test-data/training";
        }

        // Convert to file URL
        QUrl testDataUrl = QUrl::fromLocalFile(QDir(testDataDir).absolutePath());

        // Make test data directory available to QML tests
        engine->rootContext()->setContextProperty("testDataDirUrl", testDataUrl.toString());
        engine->rootContext()->setContextProperty("testDataDir", testDataDir);
    }
};

// This macro sets up the test runner for Qt Quick Tests
QUICK_TEST_MAIN_WITH_SETUP(TrainingProgramsListTests, TestSetup)

#include "qml_test_runner.moc"
