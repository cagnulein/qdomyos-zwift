#include <gtest/gtest.h>
#include <QApplication>
#include <QTimer>

// https://forum.qt.io/topic/84229/is-there-a-canonical-way-to-set-up-qapplication-and-google-test-together/2

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};

    QTimer::singleShot(0, [&]()
    {
        ::testing::InitGoogleTest(&argc, argv);
        auto testResult = RUN_ALL_TESTS();
        app.exit(testResult);
    });

    return app.exec();
}
