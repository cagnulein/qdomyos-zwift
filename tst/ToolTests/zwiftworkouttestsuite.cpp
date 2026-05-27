#include "zwiftworkouttestsuite.h"

#include "trainprogram.h"
#include "zwiftworkout.h"

#include <QDir>
#include <QTemporaryFile>

static const char *kRunningWorkoutXml = R"xml(<workout_file>
    <author>RestOrTrain AI</author>
    <name>QZ Stress Test (9m)</name>
    <description>9-minute manual verification workout</description>
    <sportType>running</sportType>
    <workout>
        <SteadyState Duration="60" Power="0.5" Incline="0.0"/>
        <SteadyState Duration="60" Power="0.8" Incline="0.0"/>
        <SteadyState Duration="60" Power="0.6" Incline="0.0"/>
        <SteadyState Duration="60" Power="0.6" Incline="2.0"/>
        <SteadyState Duration="60" Power="0.6" Incline="5.0"/>
        <SteadyState Duration="60" Power="0.6" Incline="1.0"/>
        <SteadyState Duration="60" Power="0.7" Incline="3.0"/>
        <SteadyState Duration="60" Power="0.5" Incline="6.0"/>
        <SteadyState Duration="60" Power="0.4" Incline="0.0"/>
    </workout>
</workout_file>)xml";

void ZwiftWorkoutTestSuite::test_xmlWorkoutFileParsesPercentIncline() {
    const QList<trainrow> rows = zwiftworkout::load(QByteArray(kRunningWorkoutXml));

    ASSERT_EQ(rows.length(), 9);
    EXPECT_DOUBLE_EQ(rows.at(0).inclination, 0.0);
    EXPECT_DOUBLE_EQ(rows.at(3).inclination, 2.0);
    EXPECT_DOUBLE_EQ(rows.at(4).inclination, 5.0);
    EXPECT_DOUBLE_EQ(rows.at(5).inclination, 1.0);
    EXPECT_DOUBLE_EQ(rows.at(7).inclination, 6.0);
}

void ZwiftWorkoutTestSuite::test_xmlWorkoutFileParsesNegativeIncline() {
    static const char *kNegativeInclineWorkoutXml = R"xml(<workout_file>
        <sportType>running</sportType>
        <workout>
            <SteadyState Duration="60" Power="0.6" Incline="-0.05"/>
            <SteadyState Duration="60" Power="0.6" Incline="-2.0"/>
        </workout>
    </workout_file>)xml";

    const QList<trainrow> rows = zwiftworkout::load(QByteArray(kNegativeInclineWorkoutXml));

    ASSERT_EQ(rows.length(), 2);
    EXPECT_DOUBLE_EQ(rows.at(0).inclination, -5.0);
    EXPECT_DOUBLE_EQ(rows.at(1).inclination, -2.0);
}

void ZwiftWorkoutTestSuite::test_xmlExtensionWorkoutFileUsesZwiftParser() {
    QTemporaryFile file(QDir::tempPath() + "/qz-zwo-like-XXXXXX.xml");
    ASSERT_TRUE(file.open());
    file.write(kRunningWorkoutXml);
    file.close();

    trainprogram *program = trainprogram::load(file.fileName(), nullptr, QStringLiteral("XML"));
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->rows.length(), 9);
    EXPECT_DOUBLE_EQ(program->rows.at(3).inclination, 2.0);
    EXPECT_DOUBLE_EQ(program->rows.at(7).inclination, 6.0);
    delete program;
}
