/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Nathan Osman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef QMDNSENGINE_PROVIDER_P_H
#define QMDNSENGINE_PROVIDER_P_H

#include <QObject>
#include <QTimer>

#include <qmdnsengine/record.h>
#include <qmdnsengine/service.h>

namespace QMdnsEngine {

class AbstractServer;
class Hostname;
class Message;
class Prober;

class ProviderPrivate : public QObject {
    Q_OBJECT

  public:
    ProviderPrivate(QObject *parent, AbstractServer *server, Hostname *hostname);
    virtual ~ProviderPrivate();

    void announce();
    void confirm();
    void farewell();
    void publish();

    // Send the goodbye exactly once, however we are being torn down.
    void sayGoodbye();

    AbstractServer *server;
    Hostname *hostname;
    Prober *prober;

    Service service;
    bool initialized;
    bool confirmed;
    bool saidGoodbye;

    // RFC 6762 8.3 asks for repeated announcements; a single one is easily lost
    // on Wi-Fi and the service then stays invisible until the next query.
    QTimer announceTimer;
    int announcesRemaining;
    int announceDelayMs;

    Record browsePtrRecord;
    Record ptrRecord;
    Record srvRecord;
    Record txtRecord;
    Record ARecord;

    Record browsePtrProposed;
    Record ptrProposed;
    Record srvProposed;
    Record txtProposed;

  private Q_SLOTS:

    void onMessageReceived(const Message &message);
    void onHostnameChanged(const QByteArray &hostname);
    void onAnnounceTimeout();
};

} // namespace QMdnsEngine

#endif // QMDNSENGINE_PROVIDER_P_H
