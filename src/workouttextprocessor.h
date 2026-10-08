#ifndef WORKOUTTEXTPROCESSOR_H
#define WORKOUTTEXTPROCESSOR_H

#include "trainprogram.h"
#include <QString>

class WorkoutTextProcessor {
  public:
    struct Result {
        QList<trainrow> rows;
        QString title;
        QString warning;
    };

    static Result fromCanonicalJson(const QString &canonicalJson, const QString &defaultDeviceKey = QString());
    static Result fromPromptFallback(const QString &prompt, const QString &deviceKey);
    static Result generate(const QString &prompt, const QString &deviceKey);
};

#endif // WORKOUTTEXTPROCESSOR_H
