#ifndef OAUTH2_H
#define OAUTH2_H

#include <QString>
#include <QTextStream>

struct OAuth2Parameter {
    QString responseType = QStringLiteral("code");
    QString approval_prompt = QStringLiteral("force");

    inline bool isEmpty() const { return responseType.isEmpty() && approval_prompt.isEmpty(); }

    QString toString() const {
        QString msg;
        QTextStream out(&msg);
        out << QStringLiteral("OAuth2Parameter{\n") << QStringLiteral("responseType: ") << this->responseType
            << QStringLiteral("\n") << QStringLiteral("approval_prompt: ") << this->approval_prompt
            << QStringLiteral("\n");
        return msg;
    }
};

#define _STR(x) #x
#define STRINGIFY(x) _STR(x)

#endif // OAUTH2_H
