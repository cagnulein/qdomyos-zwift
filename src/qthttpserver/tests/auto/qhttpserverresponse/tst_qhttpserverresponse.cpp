/****************************************************************************
**
** Copyright (C) 2019 Tasuku Suzuki <tasuku.suzuki@qbc.io>
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

#include <QtHttpServer/qhttpserverresponse.h>

#include <private/qhttpserverliterals_p.h>

#include <QtCore/qfile.h>
#include <QtTest/qtest.h>

QT_BEGIN_NAMESPACE

class tst_QHttpServerResponse : public QObject
{
    Q_OBJECT

private slots:
    void mimeTypeDetection_data();
    void mimeTypeDetection();
    void mimeTypeDetectionFromFile_data();
    void mimeTypeDetectionFromFile();
    void headers();
};

void tst_QHttpServerResponse::mimeTypeDetection_data()
{
    QTest::addColumn<QString>("content");
    QTest::addColumn<QByteArray>("mimeType");

    QTest::addRow("application/x-zerosize")
        << QFINDTESTDATA("data/empty")
        << QByteArrayLiteral("application/x-zerosize");

    QTest::addRow("text/plain")
        << QFINDTESTDATA("data/text.plain")
        << QByteArrayLiteral("text/plain");

    QTest::addRow("text/html")
        << QFINDTESTDATA("data/text.html")
        << QHttpServerLiterals::contentTypeTextHtml();

    QTest::addRow("image/png")
        << QFINDTESTDATA("data/image.png")
        << QByteArrayLiteral("image/png");

    QTest::addRow("image/jpeg")
             << QFINDTESTDATA("data/image.jpeg")
             << QByteArrayLiteral("image/jpeg");

    QTest::addRow("image/svg+xml")
             << QFINDTESTDATA("data/image.svg")
             << QByteArrayLiteral("image/svg+xml");
}

void tst_QHttpServerResponse::mimeTypeDetection()
{
    QFETCH(QString, content);
    QFETCH(QByteArray, mimeType);

    QFile file(content);
    file.open(QFile::ReadOnly);
    QHttpServerResponse response(file.readAll());
    file.close();

    QCOMPARE(response.mimeType(), mimeType);
}

void tst_QHttpServerResponse::mimeTypeDetectionFromFile_data()
{
    QTest::addColumn<QString>("content");
    QTest::addColumn<QByteArray>("mimeType");

    QTest::addRow("application/x-zerosize")
            << QFINDTESTDATA("data/empty")
            << QByteArrayLiteral("application/x-zerosize");

    QTest::addRow("text/plain")
            << QFINDTESTDATA("data/text.plain")
            << QByteArrayLiteral("text/plain");

    QTest::addRow("text/html")
            << QFINDTESTDATA("data/text.html")
            << QHttpServerLiterals::contentTypeTextHtml();

    QTest::addRow("image/png")
            << QFINDTESTDATA("data/image.png")
            << QByteArrayLiteral("image/png");

    QTest::addRow("image/jpeg")
            << QFINDTESTDATA("data/image.jpeg")
            << QByteArrayLiteral("image/jpeg");

    QTest::addRow("image/svg+xml")
            << QFINDTESTDATA("data/image.svg")
            << QByteArrayLiteral("image/svg+xml");

    QTest::addRow("application/json")
            << QFINDTESTDATA("data/application.json")
            << QByteArrayLiteral("application/json");
}

void tst_QHttpServerResponse::mimeTypeDetectionFromFile()
{
    QFETCH(QString, content);
    QFETCH(QByteArray, mimeType);

    QCOMPARE(QHttpServerResponse::fromFile(content).mimeType(), mimeType);
}

void tst_QHttpServerResponse::headers()
{
    QHttpServerResponse resp("");

    const QByteArray test1 = QByteArrayLiteral("test1");
    const QByteArray test2 = QByteArrayLiteral("test2");
    const QByteArray zero = QByteArrayLiteral("application/x-zerosize");
    const auto &contentTypeHeader = QHttpServerLiterals::contentTypeHeader();
    const auto &contentLengthHeader = QHttpServerLiterals::contentLengthHeader();

    QVERIFY(!resp.hasHeader(contentLengthHeader));
    QVERIFY(resp.hasHeader(contentTypeHeader, zero));
    QVERIFY(!resp.hasHeader(contentTypeHeader, test1));
    QVERIFY(!resp.hasHeader(contentTypeHeader, test2));

    resp.addHeader(contentTypeHeader, test1);
    resp.addHeader(contentLengthHeader, test2);
    QVERIFY(resp.hasHeader(contentLengthHeader, test2));
    QVERIFY(resp.hasHeader(contentTypeHeader, zero));
    QVERIFY(resp.hasHeader(contentTypeHeader, test1));
    QVERIFY(!resp.hasHeader(contentTypeHeader, test2));

    const auto &typeHeaders = resp.headers(contentTypeHeader);
    QCOMPARE(typeHeaders.size(), 2);
    QVERIFY(typeHeaders.contains(zero));
    QVERIFY(typeHeaders.contains(test1));

    const auto &lengthHeaders = resp.headers(contentLengthHeader);
    QCOMPARE(lengthHeaders.size(), 1);
    QVERIFY(lengthHeaders.contains(test2));

    resp.setHeader(contentTypeHeader, test2);

    QVERIFY(resp.hasHeader(contentLengthHeader, test2));
    QVERIFY(!resp.hasHeader(contentTypeHeader, zero));
    QVERIFY(!resp.hasHeader(contentTypeHeader, test1));
    QVERIFY(resp.hasHeader(contentTypeHeader, test2));

    resp.clearHeader(contentTypeHeader);

    QVERIFY(resp.hasHeader(contentLengthHeader, test2));

    resp.clearHeader(contentLengthHeader);

    QVERIFY(!resp.hasHeader(contentLengthHeader));
    QVERIFY(!resp.hasHeader(contentTypeHeader));

    resp.addHeaders({ {contentTypeHeader, zero}, {contentLengthHeader, test1} });

    QVERIFY(resp.hasHeader(contentTypeHeader, zero));
    QVERIFY(resp.hasHeader(contentLengthHeader, test1));

    resp.clearHeaders();

    QVERIFY(!resp.hasHeader(contentLengthHeader));
    QVERIFY(!resp.hasHeader(contentTypeHeader));

    const QList<QPair<QByteArray, QByteArray>> headers = {
      {contentTypeHeader, zero}, {contentLengthHeader, test2}
    };

    resp.addHeaders(headers);

    QVERIFY(resp.hasHeader(contentTypeHeader, zero));
    QVERIFY(resp.hasHeader(contentLengthHeader, test2));

    resp.clearHeaders();

    QVERIFY(!resp.hasHeader(contentLengthHeader));
    QVERIFY(!resp.hasHeader(contentTypeHeader));
}

QT_END_NAMESPACE

QTEST_MAIN(tst_QHttpServerResponse)

#include "tst_qhttpserverresponse.moc"
