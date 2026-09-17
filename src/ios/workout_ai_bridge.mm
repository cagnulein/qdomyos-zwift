#ifndef IO_UNDER_QT
#import <Foundation/Foundation.h>
#import "workout_ai_bridge.h"

#include "../homeform.h"
#include "../trainprogram.h"
#include "../workouttextprocessor.h"
#include <QDir>
#include <QMetaObject>
#include <QRegularExpression>

namespace {
QString sanitizeWorkoutName(const QString &input) {
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) {
        trimmed = QStringLiteral("AI_Workout");
    }
    trimmed.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_\\- ]")), QStringLiteral("_"));
    trimmed.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral("_"));
    return trimmed;
}

NSString *pendingRequestPath() {
    NSArray<NSString *> *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documents = [paths firstObject];
    return [documents stringByAppendingPathComponent:@"pending_workout_ai.json"];
}

bool saveAndMaybeStartWorkoutJson(const QString &canonicalJson, bool autoStart, QString *errorOut) {
    WorkoutTextProcessor::Result result = WorkoutTextProcessor::fromCanonicalJson(canonicalJson);
    if (result.rows.isEmpty()) {
        if (errorOut) {
            *errorOut = result.warning.isEmpty() ? QStringLiteral("Unable to parse canonical workout JSON")
                                                 : result.warning;
        }
        return false;
    }

    const QString workoutName = sanitizeWorkoutName(result.title);
    const QString trainingDir = homeform::getWritableAppDir() + QStringLiteral("training/");
    QDir dir(trainingDir);
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        if (errorOut) {
            *errorOut = QStringLiteral("Unable to create training directory");
        }
        return false;
    }

    const QString filePath = trainingDir + workoutName + QStringLiteral(".xml");
    if (!trainprogram::saveXML(filePath, result.rows)) {
        if (errorOut) {
            *errorOut = QStringLiteral("Unable to save generated workout");
        }
        return false;
    }

    if (homeform::singleton()) {
        QMetaObject::invokeMethod(homeform::singleton(), "startTrainingProgramFromFile", Qt::QueuedConnection,
                                  Q_ARG(QString, filePath));
        if (autoStart) {
            QMetaObject::invokeMethod(homeform::singleton(), "trainprogram_autostart_requested",
                                      Qt::QueuedConnection);
        }
    }

    return true;
}

bool queueOrProcessCanonicalWorkout(NSString *canonicalJson, bool autoStart, NSString **errorOut) {
    if (!canonicalJson || canonicalJson.length == 0) {
        if (errorOut) {
            *errorOut = @"Canonical workout JSON is empty";
        }
        return false;
    }

    QString errorText;
    if (homeform::singleton() && saveAndMaybeStartWorkoutJson(QString::fromUtf8(canonicalJson.UTF8String), autoStart,
                                                              &errorText)) {
        [[NSFileManager defaultManager] removeItemAtPath:pendingRequestPath() error:nil];
        return true;
    }

    NSDictionary *payload = @{
        @"canonicalJson": canonicalJson,
        @"autoStart": [NSNumber numberWithBool:autoStart]
    };
    NSError *writeError = nil;
    NSData *data = [NSJSONSerialization dataWithJSONObject:payload options:0 error:&writeError];
    if (!data || writeError) {
        if (errorOut) {
            *errorOut = writeError.localizedDescription ?: @"Unable to serialize pending workout request";
        }
        return false;
    }
    if (![data writeToFile:pendingRequestPath() options:NSDataWritingAtomic error:&writeError]) {
        if (errorOut) {
            *errorOut = writeError.localizedDescription ?: @"Unable to persist pending workout request";
        }
        return false;
    }
    return true;
}
} // namespace

static char *dupNSString(NSString *value) {
    if (!value) {
        return nullptr;
    }
    const char *utf8 = value.UTF8String;
    if (!utf8) {
        return nullptr;
    }
    size_t length = strlen(utf8);
    char *buffer = static_cast<char *>(malloc(length + 1));
    if (!buffer) {
        return nullptr;
    }
    memcpy(buffer, utf8, length + 1);
    return buffer;
}

bool ios_workout_ai_queue_canonical_workout(const char *canonicalJson, bool autoStart, char **outError) {
    @autoreleasepool {
        NSString *canonicalWorkout = [NSString stringWithUTF8String:canonicalJson ?: ""];
        NSString *error = nil;
        BOOL ok = queueOrProcessCanonicalWorkout(canonicalWorkout, autoStart, &error);
        if (outError) {
            *outError = dupNSString(error);
        }
        return ok;
    }
}

void ios_workout_ai_consume_pending_request_if_needed(void) {
    @autoreleasepool {
        NSError *readError = nil;
        NSData *data = [NSData dataWithContentsOfFile:pendingRequestPath() options:0 error:&readError];
        if (!data || readError || !homeform::singleton()) {
            return;
        }

        NSDictionary *payload = [NSJSONSerialization JSONObjectWithData:data options:0 error:&readError];
        if (![payload isKindOfClass:[NSDictionary class]] || readError) {
            return;
        }

        NSString *canonicalJson = payload[@"canonicalJson"];
        BOOL autoStart = [payload[@"autoStart"] boolValue];
        QString errorText;
        if (saveAndMaybeStartWorkoutJson(QString::fromUtf8(canonicalJson.UTF8String), autoStart, &errorText)) {
            [[NSFileManager defaultManager] removeItemAtPath:pendingRequestPath() error:nil];
        }
    }
}

void ios_workout_ai_free_string(char *value) {
    if (value) {
        free(value);
    }
}

#endif
