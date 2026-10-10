#ifndef WORKOUTIMPORT_H
#define WORKOUTIMPORT_H
#include "trainprogram.h"

// Training programs from other apps:
// - .mrc / .erg course files (TrainerRoad, Golden Cheetah, CompuTrainer): MINUTES PERCENT or MINUTES WATTS points,
//   a change of value at the same time is a step, a change over time is a ramp (split in 1 second rows like .zwo);
// - EXR rowing workouts: .json exported from the EXR web profile and .xsr installed with the game. Schedule of
//   length (seconds, or meters when unitType is 0), FTPTarget, strokesPerMin, plus rest pauses from events.
class workoutimport {

  public:
    // extension: "MRC", "ERG", "JSON" or "XSR"; callers that pass the last three characters of the file name
    // ("SON") work too. JSON/XSR files are accepted only if they contain an EXR schedule.
    static bool isSupportedFile(const QString &filename, const QString &extension = QString());
    static QList<trainrow> load(const QString &filename, const QString &extension = QString(),
                                QString *description = nullptr);

    // percentByDefault: true for .mrc, false for .erg, used when the header has no MINUTES PERCENT/WATTS line.
    // ftp: watts for 100%. Both return an empty list when the input has no usable steps.
    static QList<trainrow> loadErgMrc(const QByteArray &input, bool percentByDefault, double ftp,
                                      QString *description = nullptr);
    static QList<trainrow> loadExr(const QByteArray &input, double ftp, QString *description = nullptr);

  private:
    static QString fileKind(const QString &filename, const QString &extension);
};

#endif // WORKOUTIMPORT_H
