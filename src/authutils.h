#ifndef AUTHUTILS_H
#define AUTHUTILS_H

#include <QJsonObject>
#include <QString>
#include <QUrl>
#include <QUrlQuery>

inline QString sanitizedOAuthCallbackUrl(const QUrl &url) {
    if (!url.isValid()) {
        return QStringLiteral("(invalid)");
    }

    QUrl sanitized(url);
    if (url.hasQuery()) {
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("code"), QStringLiteral("XXXX"));
        query.addQueryItem(QStringLiteral("state"), QStringLiteral("XXXX"));
        sanitized.setQuery(query);
    }

    return sanitized.toString();
}

inline QString sanitizedOAuthCallbackUrl(const QString &callbackUrl) {
    const QUrl url(callbackUrl);
    if (!url.isValid()) {
        return QStringLiteral("(invalid)");
    }

    return sanitizedOAuthCallbackUrl(url);
}


// Strava returns id_str alongside id for 64-bit athlete identifiers.
// Qt 5 stores JSON numbers as doubles, so never use a potentially rounded
// numeric ID as a profile credential key.
inline QString stravaAthleteIdFromJson(const QJsonObject &athlete) {
    const QJsonValue stringId = athlete.value(QStringLiteral("id_str"));
    const QJsonValue legacyId = athlete.value(QStringLiteral("id"));
    if (!stringId.isUndefined() || legacyId.isString()) {
        const QJsonValue value = stringId.isUndefined() ? legacyId : stringId;
        if (!value.isString())
            return QString();
        const QString id = value.toString();
        if (id.isEmpty())
            return QString();
        for (const QChar ch : id) {
            if (ch < QLatin1Char('0') || ch > QLatin1Char('9'))
                return QString();
        }
        bool ok = false;
        const quint64 number = id.toULongLong(&ok);
        return ok && number != 0 ? QString::number(number) : QString();
    }

    // Legacy responses may omit id_str. Keep existing accounts working, but
    // reject numbers outside the range where every integer is exactly
    // representable in a double (including 2^53, which may be a rounded ID).
    if (!legacyId.isDouble())
        return QString();
    const double number = legacyId.toDouble();
    constexpr double maxSafeInteger = 9007199254740991.0;
    if (!(number > 0.0 && number <= maxSafeInteger))
        return QString();
    const quint64 integer = static_cast<quint64>(number);
    return static_cast<double>(integer) == number ? QString::number(integer) : QString();
}

#endif
