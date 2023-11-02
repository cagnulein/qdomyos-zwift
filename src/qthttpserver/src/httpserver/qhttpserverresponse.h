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

#ifndef QHTTPSERVERRESPONSE_H
#define QHTTPSERVERRESPONSE_H

#include <QtHttpServer/qhttpserverresponder.h>

#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class QHttpServerResponsePrivate;
class Q_HTTPSERVER_EXPORT QHttpServerResponse
{
    Q_DECLARE_PRIVATE(QHttpServerResponse)

public:
    using StatusCode = QHttpServerResponder::StatusCode;

    QHttpServerResponse() = delete;
    QHttpServerResponse(const QHttpServerResponse &other) = delete;
    QHttpServerResponse& operator=(const QHttpServerResponse &other) = delete;

    QHttpServerResponse(QHttpServerResponse &&other) noexcept;
    QHttpServerResponse& operator=(QHttpServerResponse &&other) noexcept;

    QHttpServerResponse(const StatusCode statusCode);

    QHttpServerResponse(const char *data);

    QHttpServerResponse(const QString &data);

    explicit QHttpServerResponse(const QByteArray &data);
    explicit QHttpServerResponse(QByteArray &&data);

    QHttpServerResponse(const QJsonObject &data);
    QHttpServerResponse(const QJsonArray &data);

    QHttpServerResponse(const QByteArray &mimeType,
                        const QByteArray &data,
                        const StatusCode status = StatusCode::Ok);
    QHttpServerResponse(QByteArray &&mimeType,
                        const QByteArray &data,
                        const StatusCode status = StatusCode::Ok);
    QHttpServerResponse(const QByteArray &mimeType,
                        QByteArray &&data,
                        const StatusCode status = StatusCode::Ok);
    QHttpServerResponse(QByteArray &&mimeType,
                        QByteArray &&data,
                        const StatusCode status = StatusCode::Ok);

    virtual ~QHttpServerResponse();
    static QHttpServerResponse fromFile(const QString &fileName);

    QByteArray data() const;

    QByteArray mimeType() const;

    StatusCode statusCode() const;

    void addHeader(QByteArray &&name, QByteArray &&value);
    void addHeader(QByteArray &&name, const QByteArray &value);
    void addHeader(const QByteArray &name, QByteArray &&value);
    void addHeader(const QByteArray &name, const QByteArray &value);

    void addHeaders(QHttpServerResponder::HeaderList headers);

    template<typename Container>
    void addHeaders(const Container &headerList)
    {
        for (const auto &header : headerList)
            addHeader(header.first, header.second);
    }

    void clearHeader(const QByteArray &name);
    void clearHeaders();

    void setHeader(QByteArray &&name, QByteArray &&value);
    void setHeader(QByteArray &&name, const QByteArray &value);
    void setHeader(const QByteArray &name, QByteArray &&value);
    void setHeader(const QByteArray &name, const QByteArray &value);

    void setHeaders(QHttpServerResponder::HeaderList headers);

    bool hasHeader(const QByteArray &name) const;
    bool hasHeader(const QByteArray &name, const QByteArray &value) const;

    QVector<QByteArray> headers(const QByteArray &name) const;

    virtual void write(QHttpServerResponder &&responder) const;

protected:
    QHttpServerResponse(QHttpServerResponsePrivate *d);

    QScopedPointer<QHttpServerResponsePrivate> d_ptr;
};

QT_END_NAMESPACE

#endif   // QHTTPSERVERRESPONSE_H
