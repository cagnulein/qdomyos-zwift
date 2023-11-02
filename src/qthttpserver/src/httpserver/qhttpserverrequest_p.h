/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtHttpServer module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QHTTPSERVERREQUEST_P_H
#define QHTTPSERVERREQUEST_P_H

#include <QtHttpServer/qhttpserverrequest.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qpair.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
#include <QtNetwork/qhostaddress.h>

#include "../3rdparty/http-parser/http_parser.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QHttpServer. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

QT_BEGIN_NAMESPACE

class QHttpServerRequestPrivate : public QSharedData
{
public:
    QHttpServerRequestPrivate(const QHostAddress &remoteAddress);

    quint16 port = 0;
    enum class State {
        NotStarted,
        OnMessageBegin,
        OnUrl,
        OnStatus,
        OnHeaders,
        OnHeadersComplete,
        OnBody,
        OnMessageComplete,
        OnChunkHeader,
        OnChunkComplete
    } state = State::NotStarted;
    QByteArray body;

    QUrl url;

    http_parser httpParser;

    QByteArray header(const QByteArray &key) const;
    bool parse(QIODevice *socket);

    QByteArray lastHeader;
    QMap<uint, QPair<QByteArray, QByteArray>> headers;
    const uint headersSeed = uint(qGlobalQHashSeed());
    uint headerHash(const QByteArray &key) const;

    void clear();
    QHostAddress remoteAddress;
    bool handling{false};

private:
    static http_parser_settings httpParserSettings;
    static bool parseUrl(const char *at, size_t length, bool connect, QUrl *url);

    static QHttpServerRequestPrivate *instance(http_parser *httpParser);

    static int onMessageBegin(http_parser *httpParser);
    static int onUrl(http_parser *httpParser, const char *at, size_t length);
    static int onStatus(http_parser *httpParser, const char *at, size_t length);
    static int onHeaderField(http_parser *httpParser, const char *at, size_t length);
    static int onHeaderValue(http_parser *httpParser, const char *at, size_t length);
    static int onHeadersComplete(http_parser *httpParser);
    static int onBody(http_parser *httpParser, const char *at, size_t length);
    static int onMessageComplete(http_parser *httpParser);
    static int onChunkHeader(http_parser *httpParser);
    static int onChunkComplete(http_parser *httpParser);
};

QT_END_NAMESPACE

#endif // QHTTPSERVERREQUEST_P_H
