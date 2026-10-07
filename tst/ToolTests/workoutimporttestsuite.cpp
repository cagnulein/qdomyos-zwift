#include "gtest/gtest.h"

#include "trainprogram.h"
#include "workoutimport.h"

#include <QDir>
#include <QTemporaryFile>

static const char *kMrcWorkout = "[COURSE HEADER]\r\n"
                                 "VERSION = 2\r\n"
                                 "UNITS = ENGLISH\r\n"
                                 "DESCRIPTION = Steps & a ramp\r\n"
                                 "FILE NAME = test.mrc\r\n"
                                 "MINUTES PERCENT\r\n"
                                 "[END COURSE HEADER]\r\n"
                                 "[COURSE DATA]\r\n"
                                 "0.00\t50\r\n"
                                 "1.00\t50\r\n"
                                 "1.00\t100\r\n"
                                 "2.00\t100\r\n"
                                 "2.00\t60\r\n"
                                 "3.00\t80\r\n"
                                 "[END COURSE DATA]\r\n"
                                 "[COURSE TEXT]\r\n"
                                 "70\tGo hard\t10\r\n"
                                 "\t\r\n"
                                 "70\tStand up\t5\r\n"
                                 "150\tKeep spinning\t10\r\n"
                                 "[END COURSE TEXT]\r\n";

static const char *kExrWorkout = R"json({
  "metaData": {"fileVersionNumber": 2, "guid": "124b5131-baad-4449-85af-2926f592194c"},
  "data": {
    "category": "Custom Workouts",
    "title": "HIIT Row",
    "unitType": 1,
    "description": "My Custom training",
    "schedule": [
      {"length": 600, "FTPTarget": 0.5584951043128967, "strokesPerMin": 20},
      {"length": 240, "FTPTarget": 1.2205406427383423, "strokesPerMin": 30},
      {"length": 180, "FTPTarget": -1, "strokesPerMin": 0}
    ],
    "events": []
  },
  "editorData": {"eventLinks": []},
  "uuid": "ecccecd6-6829-48aa-bc11-77d0160e5137"
})json";

TEST(WorkoutImport, MrcStepsAndRampUsePercentOfFtp) {
    QString description;
    const QList<trainrow> rows = workoutimport::loadErgMrc(QByteArray(kMrcWorkout), true, 200.0, &description);

    // two steps + a 60 s ramp split in 1 s rows
    ASSERT_EQ(rows.length(), 62);
    EXPECT_EQ(description, QStringLiteral("Steps & a ramp"));
    EXPECT_EQ(rows.at(0).duration, QTime(0, 1, 0));
    EXPECT_EQ(rows.at(0).power, 100);
    EXPECT_EQ(rows.at(1).duration, QTime(0, 1, 0));
    EXPECT_EQ(rows.at(1).power, 200);
    EXPECT_EQ(rows.at(2).duration, QTime(0, 0, 1));
    EXPECT_EQ(rows.at(2).power, 120);
    EXPECT_EQ(rows.at(2).rampDuration, QTime(0, 1, 0));
    EXPECT_EQ(rows.at(32).power, 140);
    EXPECT_EQ(rows.at(61).power, 159);
    EXPECT_EQ(rows.at(61).rampElapsed, QTime(0, 0, 59));

    ASSERT_EQ(rows.at(1).textEvents.length(), 1);
    EXPECT_EQ(rows.at(1).textEvents.at(0).timeoffset, 10u);
    // a blank line in [COURSE TEXT] is skipped, two messages for the same second are joined
    EXPECT_EQ(rows.at(1).textEvents.at(0).message, QStringLiteral("Go hard\nStand up"));
    // 150 s is the 31st second of the ramp
    ASSERT_EQ(rows.at(32).textEvents.length(), 1);
    EXPECT_EQ(rows.at(32).textEvents.at(0).timeoffset, 0u);
}

TEST(WorkoutImport, ErgKeepsAbsoluteWatts) {
    static const char *kErgWorkout = "[COURSE HEADER]\n"
                                     "VERSION = 2\n"
                                     "UNITS = ENGLISH\n"
                                     "FTP = 250\n"
                                     "MINUTES WATTS\n"
                                     "[END COURSE HEADER]\n"
                                     "[COURSE DATA]\n"
                                     "0.00 151\n"
                                     "0.50 151\n"
                                     "0.50 333\n"
                                     "1.50 333\n"
                                     "[END COURSE DATA]\n";

    const QList<trainrow> rows = workoutimport::loadErgMrc(QByteArray(kErgWorkout), false, 287.0);

    ASSERT_EQ(rows.length(), 2);
    EXPECT_EQ(rows.at(0).duration, QTime(0, 0, 30));
    EXPECT_EQ(rows.at(0).power, 151);
    EXPECT_EQ(rows.at(1).duration, QTime(0, 1, 0));
    EXPECT_EQ(rows.at(1).power, 333);
}

TEST(WorkoutImport, MrcWithoutCourseDataIsEmpty) {
    EXPECT_TRUE(workoutimport::loadErgMrc(QByteArray("[COURSE HEADER]\nMINUTES PERCENT\n[END COURSE HEADER]\n"), true,
                                          200.0)
                    .isEmpty());
}

TEST(WorkoutImport, ExrScheduleUsesFtpTargetAndStrokeRate) {
    QString description;
    const QList<trainrow> rows = workoutimport::loadExr(QByteArray(kExrWorkout), 200.0, &description);

    ASSERT_EQ(rows.length(), 3);
    EXPECT_EQ(description, QStringLiteral("My Custom training"));
    EXPECT_EQ(rows.at(0).duration, QTime(0, 10, 0));
    EXPECT_EQ(rows.at(0).power, 112);
    EXPECT_EQ(rows.at(0).cadence, 20);
    EXPECT_EQ(rows.at(1).duration, QTime(0, 4, 0));
    EXPECT_EQ(rows.at(1).power, 244);
    EXPECT_EQ(rows.at(1).cadence, 30);
    // FTPTarget -1: free rowing without targets
    EXPECT_EQ(rows.at(2).duration, QTime(0, 3, 0));
    EXPECT_EQ(rows.at(2).power, -1);
    EXPECT_EQ(rows.at(2).cadence, -1);
}

TEST(WorkoutImport, TrainProgramLoadsExrJsonByContent) {
    QTemporaryFile file(QDir::tempPath() + "/qz-exr-XXXXXX.json");
    ASSERT_TRUE(file.open());
    file.write(kExrWorkout);
    file.close();

    // homeform passes the last three characters of the file name as the extension
    EXPECT_TRUE(workoutimport::isSupportedFile(file.fileName(), QStringLiteral("SON")));
    trainprogram *program = trainprogram::load(file.fileName(), nullptr, QStringLiteral("SON"));
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->rows.length(), 3);
    EXPECT_EQ(program->rows.at(1).cadence, 30);
    delete program;
}

TEST(WorkoutImport, OtherJsonIsNotAWorkout) {
    QTemporaryFile file(QDir::tempPath() + "/qz-other-XXXXXX.json");
    ASSERT_TRUE(file.open());
    file.write("{\"data\": {\"title\": \"not a workout\"}}");
    file.close();

    EXPECT_FALSE(workoutimport::isSupportedFile(file.fileName()));
}
