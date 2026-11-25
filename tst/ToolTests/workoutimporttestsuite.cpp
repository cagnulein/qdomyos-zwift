#include "gtest/gtest.h"

#include "trainprogram.h"
#include "workoutimport.h"
#include "fit-sdk/fit_buffer_encode.hpp"
#include "fit-sdk/fit_file_id_mesg.hpp"
#include "fit-sdk/fit_workout_mesg.hpp"
#include "fit-sdk/fit_workout_step_mesg.hpp"

#include <QDir>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>

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

// workout installed with the EXR game: PascalCase keys, meters (UnitType 0), a rest pause, a trailing comma
static const char *kExrInstalledWorkout = R"json({
    "MetaData": {"FileVersionNumber": 2, "Tags": ["AEROBIC", "ENDURANCE",]},
    "TrainingData": {
        "Title": "Kilometer Climb",
        "UnitType": 0,
        "Description": "Short sprints after every kilometer.",
        "Schedule": [
            {"Length": 1000, "FTPTarget": 1.0, "StrokesPerMin": 22},
            {"Length": 250, "FTPTarget": 1.2, "StrokesPerMin": 0}
        ],
        "Events": [
            {"_id": 1, "_timeStamp": 1000,
             "_dataTypeString": "RowingTrainingRestData, Assembly-CSharp, Version=0.0.0.0, Culture=neutral, PublicKeyToken=null",
             "DataString": "{\"_restTime\":60}"}
        ]
    }
})json";

TEST(WorkoutImport, ExrInstalledWorkoutUsesMetersAndRestPauses) {
    QString description;
    const QList<trainrow> rows = workoutimport::loadExr(QByteArray(kExrInstalledWorkout), 200.0, &description);

    ASSERT_EQ(rows.length(), 3);
    EXPECT_EQ(description, QStringLiteral("Short sprints after every kilometer."));
    EXPECT_DOUBLE_EQ(rows.at(0).distance, 1.0);
    EXPECT_EQ(rows.at(0).power, 200);
    EXPECT_EQ(rows.at(0).cadence, 22);
    // the rest pause at 1000 m becomes a 60 s step without targets
    EXPECT_DOUBLE_EQ(rows.at(1).distance, -1);
    EXPECT_EQ(rows.at(1).duration, QTime(0, 1, 0));
    EXPECT_EQ(rows.at(1).power, -1);
    EXPECT_DOUBLE_EQ(rows.at(2).distance, 0.25);
    EXPECT_EQ(rows.at(2).power, 240);
    EXPECT_EQ(rows.at(2).cadence, -1);
}

TEST(WorkoutImport, TrainProgramLoadsExrXsrFile) {
    QTemporaryFile file(QDir::tempPath() + "/qz-exr-XXXXXX.xsr");
    ASSERT_TRUE(file.open());
    file.write(kExrInstalledWorkout);
    file.close();

    trainprogram *program = trainprogram::load(file.fileName(), nullptr, QStringLiteral("XSR"));
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->rows.length(), 3);
    EXPECT_DOUBLE_EQ(program->rows.at(2).distance, 0.25);
    delete program;
}

TEST(WorkoutImport, OtherJsonIsNotAWorkout) {
    QTemporaryFile file(QDir::tempPath() + "/qz-other-XXXXXX.json");
    ASSERT_TRUE(file.open());
    file.write("{\"data\": {\"title\": \"not a workout\"}}");
    file.close();

    EXPECT_FALSE(workoutimport::isSupportedFile(file.fileName()));
}

namespace {
QByteArray makeFitWorkout(const QList<fit::WorkoutStepMesg> &steps, FIT_FILE fileType = FIT_FILE_WORKOUT,
                          FIT_SPORT sport = FIT_SPORT_RUNNING) {
    fit::BufferEncode encode;
    encode.Open();
    fit::FileIdMesg id;
    id.SetType(fileType);
    encode.Write(id);
    fit::WorkoutMesg workout;
    workout.SetSport(sport);
    workout.SetNumValidSteps(FIT_UINT16(steps.size()));
    encode.Write(workout);
    for (const auto &step : steps)
        encode.Write(step);
    const std::string result = encode.Close();
    return QByteArray(result.data(), int(result.size()));
}

fit::WorkoutStepMesg timedFitStep(int index, int seconds, FIT_WKT_STEP_TARGET target,
                                FIT_UINT32 low = 0, FIT_UINT32 high = 0) {
    fit::WorkoutStepMesg step;
    step.SetMessageIndex(FIT_MESSAGE_INDEX(index));
    step.SetDurationType(FIT_WKT_STEP_DURATION_TIME);
    step.SetDurationValue(FIT_UINT32(seconds * 1000));
    step.SetTargetType(target);
    step.SetTargetValue(0);
    step.SetCustomTargetValueLow(low);
    step.SetCustomTargetValueHigh(high);
    return step;
}

fit::WorkoutStepMesg fitRepeat(int index, int first, int count) {
    fit::WorkoutStepMesg step;
    step.SetMessageIndex(FIT_MESSAGE_INDEX(index));
    step.SetDurationType(FIT_WKT_STEP_DURATION_REPEAT_UNTIL_STEPS_CMPLT);
    step.SetDurationValue(FIT_UINT32(first));
    step.SetTargetType(FIT_WKT_STEP_TARGET_OPEN);
    step.SetTargetValue(FIT_UINT32(count));
    return step;
}
} // namespace

TEST(WorkoutImport, FitNorwegianFourByFourHeartRate) {
    // Synthetic FIT exercising the common 4 x 4 structure; not a downloaded FitFileForge file.
    const auto bytes = makeFitWorkout({
        timedFitStep(0, 600, FIT_WKT_STEP_TARGET_OPEN),
        timedFitStep(1, 240, FIT_WKT_STEP_TARGET_HEART_RATE, 85, 95),
        timedFitStep(2, 180, FIT_WKT_STEP_TARGET_HEART_RATE, 60, 70),
        fitRepeat(3, 1, 4),
        timedFitStep(4, 300, FIT_WKT_STEP_TARGET_OPEN)
    });
    const auto rows = workoutimport::loadFit(bytes, 250, 250, 200);
    ASSERT_EQ(rows.size(), 10);
    EXPECT_EQ(rows.first().duration, QTime(0, 10, 0));
    for (int i = 0; i < 4; ++i) {
        const auto &hard = rows.at(1 + 2 * i);
        const auto &easy = rows.at(2 + 2 * i);
        EXPECT_EQ(hard.duration, QTime(0, 4, 0));
        EXPECT_EQ(hard.HRmin, 170);
        EXPECT_EQ(hard.HRmax, 190);
        EXPECT_EQ(easy.duration, QTime(0, 3, 0));
        EXPECT_EQ(easy.HRmin, 120);
        EXPECT_EQ(easy.HRmax, 140);
    }
    EXPECT_EQ(rows.last().duration, QTime(0, 5, 0));
}

TEST(WorkoutImport, FitPowerSpeedCadenceAndAbsoluteHeartRate) {
    const auto rows = workoutimport::loadFit(makeFitWorkout({
        timedFitStep(0, 60, FIT_WKT_STEP_TARGET_POWER, 80, 100),
        timedFitStep(1, 60, FIT_WKT_STEP_TARGET_POWER, 1200, 1250),
        timedFitStep(2, 60, FIT_WKT_STEP_TARGET_SPEED, 2500, 3500),
        timedFitStep(3, 60, FIT_WKT_STEP_TARGET_CADENCE, 80, 100),
        timedFitStep(4, 60, FIT_WKT_STEP_TARGET_HEART_RATE, 250, 270)
    }), 250, 250, 200);
    ASSERT_EQ(rows.size(), 5);
    EXPECT_EQ(rows.at(0).power, 225);
    EXPECT_EQ(rows.at(1).power, 225);
    EXPECT_NEAR(rows.at(2).speed, 10.8, 1e-6);
    EXPECT_TRUE(rows.at(2).forcespeed);
    EXPECT_EQ(rows.at(3).cadence, 90);
    EXPECT_EQ(rows.at(4).HRmin, 150);
    EXPECT_EQ(rows.at(4).HRmax, 170);
}

TEST(WorkoutImport, FitDistanceAndLap) {
    auto distance = timedFitStep(0, 60, FIT_WKT_STEP_TARGET_OPEN);
    distance.SetDurationType(FIT_WKT_STEP_DURATION_DISTANCE);
    distance.SetDurationDistance(1000);
    auto lap = timedFitStep(1, 60, FIT_WKT_STEP_TARGET_OPEN);
    lap.SetDurationType(FIT_WKT_STEP_DURATION_OPEN);
    const auto rows = workoutimport::loadFit(makeFitWorkout({distance, lap}), 250, 250, 200);
    ASSERT_EQ(rows.size(), 2);
    EXPECT_DOUBLE_EQ(rows.first().distance, 1.0);
    EXPECT_TRUE(rows.last().waitForLap);
}

TEST(WorkoutImport, FitRejectsActivityTruncationAndInvalidRepeats) {
    const auto step = timedFitStep(0, 60, FIT_WKT_STEP_TARGET_OPEN);
    EXPECT_TRUE(workoutimport::loadFit(makeFitWorkout({step}, FIT_FILE_ACTIVITY), 250, 250, 200).isEmpty());
    const auto valid = makeFitWorkout({step});
    EXPECT_TRUE(workoutimport::loadFit(valid.left(valid.size() - 1), 250, 250, 200).isEmpty());
    EXPECT_TRUE(workoutimport::loadFit(makeFitWorkout({step, fitRepeat(1, 1, 4)}), 250, 250, 200).isEmpty());
    EXPECT_TRUE(workoutimport::loadFit(makeFitWorkout({step, fitRepeat(1, 0, 10001)}), 250, 250, 200).isEmpty());
    EXPECT_TRUE(workoutimport::loadFit(makeFitWorkout({step, timedFitStep(1, 60, FIT_WKT_STEP_TARGET_GRADE)}),
                                       250, 250, 200).isEmpty());
}

TEST(WorkoutImport, FitFileDispatch) {
    QTemporaryFile file(QDir::tempPath() + QStringLiteral("/qz-workout-XXXXXX.fit"));
    ASSERT_TRUE(file.open());
    const auto bytes = makeFitWorkout({timedFitStep(0, 60, FIT_WKT_STEP_TARGET_OPEN)});
    ASSERT_EQ(file.write(bytes), bytes.size());
    file.flush();
    EXPECT_TRUE(workoutimport::isSupportedFile(file.fileName()));
    const auto rows = workoutimport::load(file.fileName(), QStringLiteral("FIT"));
    ASSERT_EQ(rows.size(), 1);
    EXPECT_EQ(rows.first().duration, QTime(0, 1, 0));
}


TEST(WorkoutImport, FitFileForgeNorwegianFourByFourOriginal) {
    // Byte-for-byte Norwegian_4_4.fit supplied by the maintainer, not re-encoded
    // by the SDK. Source: https://www.fitfileforge.com/workouts/library/norwegian-4x4
    // Base64 keeps the binary fixture self-contained in the existing test suite.
    const QByteArray bytes = QByteArray::fromBase64(
        "DgLeUlICAAAuRklUG+dAAAAAAAQAAQIBAoQCAoQEBIYABf8AAQB3v2lFQQAADAABAAECAQFCAAAaAAQE"
        "AQIIEQcGAoQRogcCAU5vcndlZ2lhbiA0IMOXIDQABQA0IMOXIDQgbWluIGF0IDkw4oCTOTUlIG1heCBI"
        "UiwgMyBtaW4gZWFzeSBqb2cgwrcgSW5zcGlyZWQgYnkgSmFuIEhlbGdlcnVkLiDCtyBGaXQgRmlsZSBG"
        "b3JnZSBXb3Jrb3V0IExpYnJhcnk6IHd3dy5maXRmaWxlZm9yZ2UuY29tL3dvcmtvdXRzL2xpYnJhcnkv"
        "bm9yd2VnaWFuLTR4NABDAAAbAAj+AoQABQcHAQIBAQICBIYDAQIEBIYIBQcDAABFYXN5AAIAoLsNAAIA"
        "AAAARWFzeQBEAAAbAAr+AoQAFQcHAQIBAQICBIYDAQIEBIYFBIYGBIYIFQcEAQBBYm91dCAzS+KAkzVL"
        "IGVmZm9ydAAFAICpAwABAAAAAFoAAABfAAAAQWJvdXQgM0vigJM1SyBlZmZvcnQARQAAGwAI/gKEADEH"
        "BwECAQECAgSGAwECBASGCDEHBQIARWFzeSBqb2c7IGFib3V0IDcwJSBvZiBtYXggaGVhcnQgcmF0ZSBh"
        "cyBhIGd1aWRlAAQAIL8CAAIAAAAARWFzeSBqb2c7IGFib3V0IDcwJSBvZiBtYXggaGVhcnQgcmF0ZSBh"
        "cyBhIGd1aWRlAEYAABsABf4ChAcBAgEBAgIEhgQEhgYDAAEGAQAAAAQAAAADBABFYXN5AAMAwCcJAAIA"
        "AAAARWFzeQChTA==");
    ASSERT_EQ(bytes.size(), 610);

    QString description;
    // Fixed profile values keep this test independent of the user's settings.
    BLUETOOTH_TYPE fitDeviceType = UNKNOWN;
    const auto rows = workoutimport::loadFit(bytes, 250, 250, 200, &description, &fitDeviceType);
    ASSERT_EQ(rows.size(), 10);
    EXPECT_EQ(fitDeviceType, TREADMILL);
    EXPECT_EQ(description, QString::fromUtf8("Norwegian 4 \xC3\x97 4"));

    auto checkFreeStep = [](const trainrow &row, int seconds, const QString &message) {
        EXPECT_EQ(row.duration, QTime(0, 0).addSecs(seconds));
        EXPECT_EQ(row.HRmin, -1);
        EXPECT_EQ(row.HRmax, -1);
        EXPECT_EQ(row.zoneHR, -1);
        EXPECT_EQ(row.power, -1);
        EXPECT_EQ(row.speed, -1);
        EXPECT_FALSE(row.forcespeed);
        EXPECT_FALSE(row.waitForLap);
        ASSERT_EQ(row.textEvents.size(), 1);
        EXPECT_EQ(row.textEvents.first().timeoffset, 0u);
        EXPECT_EQ(row.textEvents.first().message, message);
    };

    checkFreeStep(rows.first(), 900, QStringLiteral("Easy"));
    for (int i = 0; i < 4; ++i) {
        SCOPED_TRACE(i);
        const auto &hard = rows.at(1 + 2 * i);
        EXPECT_EQ(hard.duration, QTime(0, 4));
        EXPECT_EQ(hard.HRmin, 180);
        EXPECT_EQ(hard.HRmax, 190);
        EXPECT_EQ(hard.zoneHR, -1);
        EXPECT_EQ(hard.power, -1);
        EXPECT_EQ(hard.speed, -1);
        EXPECT_FALSE(hard.waitForLap);
        ASSERT_EQ(hard.textEvents.size(), 1);
        EXPECT_EQ(hard.textEvents.first().timeoffset, 0u);
        EXPECT_EQ(hard.textEvents.first().message, QString::fromUtf8("About 3K\xE2\x80\x93" "5K effort"));

        // The 70% HR recovery guidance is text, not a FIT heart-rate target.
        checkFreeStep(rows.at(2 + 2 * i), 180,
                      QStringLiteral("Easy jog; about 70% of max heart rate as a guide"));
    }
    checkFreeStep(rows.last(), 600, QStringLiteral("Easy"));

    int totalSeconds = 0;
    for (const auto &row : rows)
        totalSeconds += QTime(0, 0).secsTo(row.duration);
    EXPECT_EQ(totalSeconds, 53 * 60);

    // Verify the same original bytes also work through the file-based dispatcher.
    QTemporaryFile file(QDir::tempPath() + QStringLiteral("/Norwegian_4_4-XXXXXX.fit"));
    ASSERT_TRUE(file.open());
    ASSERT_EQ(file.write(bytes), bytes.size());
    ASSERT_TRUE(file.flush());
    file.close();
    EXPECT_TRUE(workoutimport::isSupportedFile(file.fileName(), QStringLiteral("FIT")));
    QString fileDescription;
    const auto imported = workoutimport::load(file.fileName(), QStringLiteral("FIT"), &fileDescription);
    ASSERT_EQ(imported.size(), rows.size());
    EXPECT_EQ(fileDescription, description);
    for (int i = 0; i < rows.size(); ++i)
        EXPECT_EQ(imported.at(i).duration, rows.at(i).duration);
}

TEST(WorkoutImport, FitSelectsSportSpecificFtp) {
    const QList<fit::WorkoutStepMesg> steps{
        timedFitStep(0, 60, FIT_WKT_STEP_TARGET_POWER, 80, 100),
        timedFitStep(1, 60, FIT_WKT_STEP_TARGET_POWER, 1200, 1250)
    };
    BLUETOOTH_TYPE type = UNKNOWN;
    auto rows = workoutimport::loadFit(makeFitWorkout(steps, FIT_FILE_WORKOUT, FIT_SPORT_RUNNING),
                                       200, 300, 190, nullptr, &type);
    ASSERT_EQ(rows.size(), 2);
    EXPECT_EQ(type, TREADMILL);
    EXPECT_EQ(rows.at(0).power, 270);
    EXPECT_EQ(rows.at(1).power, 225);
    rows = workoutimport::loadFit(makeFitWorkout(steps, FIT_FILE_WORKOUT, FIT_SPORT_CYCLING),
                                  200, 300, 190, nullptr, &type);
    ASSERT_EQ(rows.size(), 2);
    EXPECT_EQ(type, BIKE);
    EXPECT_EQ(rows.at(0).power, 180);
    EXPECT_EQ(rows.at(1).power, 225);
}

TEST(WorkoutImport, FitNotesAndNamesPreservedOncePerInterval) {
    auto both = timedFitStep(0, 60, FIT_WKT_STEP_TARGET_OPEN);
    both.SetWktStepName(L"Warm up");
    both.SetNotes(L"Keep it easy");
    auto duplicate = timedFitStep(1, 60, FIT_WKT_STEP_TARGET_OPEN);
    duplicate.SetWktStepName(L"Recovery");
    duplicate.SetNotes(L"Recovery");
    auto notesOnly = timedFitStep(2, 60, FIT_WKT_STEP_TARGET_OPEN);
    notesOnly.SetNotes(L"Cool down");
    const auto rows = workoutimport::loadFit(makeFitWorkout({both, duplicate, notesOnly, fitRepeat(3, 0, 2)}),
                                             200, 300, 190);
    ASSERT_EQ(rows.size(), 6);
    for (int i = 0; i < 6; ++i) {
        ASSERT_EQ(rows.at(i).textEvents.size(), 1);
        EXPECT_EQ(rows.at(i).textEvents.first().timeoffset, 0u);
        const QString expected[] = {QStringLiteral("Warm up\nKeep it easy"),
                                    QStringLiteral("Recovery"), QStringLiteral("Cool down")};
        EXPECT_EQ(rows.at(i).textEvents.first().message, expected[i % 3]);
    }
}

TEST(WorkoutImport, FitDeviceCompatibility) {
    EXPECT_TRUE(workoutimport::isFitDeviceCompatible(TREADMILL, TREADMILL));
    EXPECT_TRUE(workoutimport::isFitDeviceCompatible(BIKE, BIKE));
    EXPECT_TRUE(workoutimport::isFitDeviceCompatible(ROWING, ROWING));
    EXPECT_FALSE(workoutimport::isFitDeviceCompatible(BIKE, TREADMILL));
    EXPECT_FALSE(workoutimport::isFitDeviceCompatible(TREADMILL, BIKE));
    EXPECT_FALSE(workoutimport::isFitDeviceCompatible(ROWING, TREADMILL));
    EXPECT_TRUE(workoutimport::isFitDeviceCompatible(UNKNOWN, BIKE));
    EXPECT_TRUE(workoutimport::isFitDeviceCompatible(BIKE, UNKNOWN));
    const auto bytes = makeFitWorkout({timedFitStep(0, 60, FIT_WKT_STEP_TARGET_OPEN)},
                                       FIT_FILE_WORKOUT, FIT_SPORT_SWIMMING);
    EXPECT_TRUE(workoutimport::loadFit(bytes, 200, 300, 190).isEmpty());
}

TEST(WorkoutImport, FitLibraryKeepsOriginalWithoutXml) {
    QTemporaryDir external;
    QTemporaryDir training;
    ASSERT_TRUE(external.isValid());
    ASSERT_TRUE(training.isValid());
    const QString sourcePath = QDir(external.path()).filePath(QStringLiteral("Workout.fit"));
    const auto bytes = makeFitWorkout({timedFitStep(0, 60, FIT_WKT_STEP_TARGET_HEART_RATE, 80, 90)});
    QFile source(sourcePath);
    ASSERT_TRUE(source.open(QIODevice::WriteOnly));
    ASSERT_EQ(source.write(bytes), bytes.size());
    source.close();
    const QString saved = workoutimport::keepFitInTrainingFolder(sourcePath, training.path());
    ASSERT_EQ(saved, QDir(training.path()).filePath(QStringLiteral("Workout.fit")));
    QFile savedFile(saved);
    ASSERT_TRUE(savedFile.open(QIODevice::ReadOnly));
    EXPECT_EQ(savedFile.readAll(), bytes);
    savedFile.close();
    EXPECT_EQ(workoutimport::keepFitInTrainingFolder(sourcePath, training.path()), saved);
    EXPECT_EQ(workoutimport::keepFitInTrainingFolder(saved, training.path()), QFileInfo(saved).canonicalFilePath());
    EXPECT_TRUE(QDir(training.path()).entryList(QStringList() << QStringLiteral("*.xml"), QDir::Files).isEmpty());
    const auto rows = workoutimport::load(saved, QStringLiteral("FIT"));
    ASSERT_EQ(rows.size(), 1);
    EXPECT_EQ(rows.first().duration, QTime(0, 1));

    ASSERT_TRUE(source.open(QIODevice::WriteOnly | QIODevice::Truncate));
    const auto changed = makeFitWorkout({timedFitStep(0, 120, FIT_WKT_STEP_TARGET_OPEN)});
    ASSERT_EQ(source.write(changed), changed.size());
    source.close();
    const QString second = workoutimport::keepFitInTrainingFolder(sourcePath, training.path());
    EXPECT_EQ(second, QDir(training.path()).filePath(QStringLiteral("Workout_2.fit")));
    ASSERT_TRUE(savedFile.open(QIODevice::ReadOnly));
    EXPECT_EQ(savedFile.readAll(), bytes);
}
