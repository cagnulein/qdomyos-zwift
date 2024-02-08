
#include <gtest/gtest.h>
#include <QApplication>
#include <QTimer>


int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};

    QTimer::singleShot(500, [&]()
    {
        ::testing::InitGoogleTest(&argc, argv);
        auto testResult = RUN_ALL_TESTS();
        app.exit(testResult);
    });

    return app.exec();
}
