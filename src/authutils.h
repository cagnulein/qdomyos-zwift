#ifndef AUTHUTILS_H
#define AUTHUTILS_H

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

#endif
