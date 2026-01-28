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

#include <qmdnsengine/abstractserver.h>
#include <qmdnsengine/browser.h>
#include <qmdnsengine/cache.h>
#include <qmdnsengine/dns.h>
#include <qmdnsengine/mdns.h>
#include <qmdnsengine/message.h>
#include <qmdnsengine/query.h>
#include <qmdnsengine/record.h>

#include "browser_p.h"

using namespace QMdnsEngine;

BrowserPrivate::BrowserPrivate(Browser *browser, AbstractServer *server, const QByteArray &type, Cache *existingCache)
    : QObject(browser),
      server(server),
      type(type),
      cache(existingCache ? existingCache : new Cache(this)),
      q(browser)
{
    connect(server, &AbstractServer::messageReceived, this, &BrowserPrivate::onMessageReceived);
    connect(cache, &Cache::shouldQuery, this, &BrowserPrivate::onShouldQuery);
    connect(cache, &Cache::recordExpired, this, &BrowserPrivate::onRecordExpired);
    connect(&queryTimer, &QTimer::timeout, this, &BrowserPrivate::onQueryTimeout);
    connect(&serviceTimer, &QTimer::timeout, this, &BrowserPrivate::onServiceTimeout);

    queryTimer.setInterval(60 * 1000);
    queryTimer.setSingleShot(true);

    serviceTimer.setInterval(100);
    serviceTimer.setSingleShot(true);

    // Immediately begin browsing for services
    onQueryTimeout();
}

// TODO: multiple SRV records not supported

bool BrowserPrivate::updateService(const QByteArray &fqName)
{
    // Split the FQDN into service name and type
    int index = fqName.indexOf('.');
    QByteArray serviceName = fqName.left(index);
    QByteArray serviceType = fqName.mid(index + 1);

    // Immediately return if a PTR record does not exist
    Record ptrRecord;
    if (!cache->lookupRecord(serviceType, PTR, ptrRecord)) {
        return false;
    }

    // If a SRV record is missing, query for it (by returning true)
    Record srvRecord;
    if (!cache->lookupRecord(fqName, SRV, srvRecord)) {
        return true;
    }

    Service service;
    service.setName(serviceName);
    service.setType(serviceType);
    service.setHostname(srvRecord.target());
    service.setPort(srvRecord.port());

    // If TXT records are available for the service, add their values
    QList<Record> txtRecords;
    if (cache->lookupRecords(fqName, TXT, txtRecords)) {
        QMap<QByteArray, QByteArray> attributes;
        for (const Record &record : qAsConst(txtRecords)) {
            for (auto i = record.attributes().constBegin();
                    i != record.attributes().constEnd(); ++i) {
                attributes.insert(i.key(), i.value());
            }
        }
        service.setAttributes(attributes);
    }

    // If the service existed, this is an update; otherwise it is a new
    // addition; emit the appropriate signal
    if (!services.contains(fqName)) {
        emit q->serviceAdded(service);
    } else if(services.value(fqName) != service) {
        emit q->serviceUpdated(service);
    }

    services.insert(fqName, service);
    hostnames.insert(service.hostname());

    return false;
}

void BrowserPrivate::onMessageReceived(const Message &message)
{
    if (!message.isResponse()) {
        return;
    }

    const bool any = type == MdnsBrowseType;

    // Use a set to track all services that are updated in the message to
    // prevent unnecessary queries for SRV and TXT records
    QSet<QByteArray> updateNames;
    const auto records = message.records();
    for (const Record &record : records) {
        bool cacheRecord = false;

        switch (record.type()) {
        case PTR:
            if (any && record.name() == MdnsBrowseType) {
                ptrTargets.insert(record.target());
                serviceTimer.start();
                cacheRecord = true;
            } else if (any || record.name() == type) {
                updateNames.insert(record.target());
                cacheRecord = true;
            }
            break;
        case SRV:
        case TXT:
            if (any || record.name().endsWith("." + type)) {
                updateNames.insert(record.name());
                cacheRecord = true;
            }
            break;
        }
        if (cacheRecord) {
            cache->addRecord(record);
        }
    }

    // For each of the services marked to be updated, perform the update and
    // make a list of all missing SRV records
    QSet<QByteArray> queryNames;
    for (const QByteArray &name : qAsConst(updateNames)) {
        if (updateService(name)) {
            queryNames.insert(name);
        }
    }

    // Cache A / AAAA records after services are processed to ensure hostnames are known
    for (const Record &record : records) {
        bool cacheRecord = false;

        switch (record.type()) {
            case A:
            case AAAA:
                cacheRecord = hostnames.contains(record.name());
                break;
        }
        if (cacheRecord) {
            cache->addRecord(record);
        }
    }

    // Build and send a query for all of the SRV and TXT records
    if (queryNames.count()) {
        Message queryMessage;
        for (const QByteArray &name : qAsConst(queryNames)) {
            Query query;
            query.setName(name);
            query.setType(SRV);
            queryMessage.addQuery(query);
            query.setType(TXT);
            queryMessage.addQuery(query);
        }
        server->sendMessageToAll(queryMessage);
    }
}

void BrowserPrivate::onShouldQuery(const Record &record)
{
    // Assume that all messages in the cache are still in use (by the browser)
    // and attempt to renew them immediately

    Query query;
    query.setName(record.name());
    query.setType(record.type());
    Message message;
    message.addQuery(query);
    server->sendMessageToAll(message);
}

void BrowserPrivate::onRecordExpired(const Record &record)
{
    // If the SRV record has expired for a service, then it must be
    // removed - TXT records on the other hand, cause an update

    QByteArray serviceName;
    switch (record.type()) {
    case SRV:
        serviceName = record.name();
        break;
    case TXT:
        updateService(record.name());
        return;
    default:
        return;
    }
    Service service = services.value(serviceName);
    if (!service.name().isNull()) {
        emit q->serviceRemoved(service);
        services.remove(serviceName);
        updateHostnames();
    }
}

void BrowserPrivate::onQueryTimeout()
{
    Query query;
    query.setName(type);
    query.setType(PTR);
    Message message;
    message.addQuery(query);

    // TODO: including too many records could cause problems

    // Include PTR records for the target that are already known
    QList<Record> records;
    if (cache->lookupRecords(query.name(), PTR, records)) {
        for (const Record &record : qAsConst(records)) {
            message.addRecord(record);
        }
    }

    server->sendMessageToAll(message);
    queryTimer.start();
}

void BrowserPrivate::onServiceTimeout()
{
    if (ptrTargets.count()) {
        Message message;
        for (const QByteArray &target : qAsConst(ptrTargets)) {

            // Add a query for PTR records
            Query query;
            query.setName(target);
            query.setType(PTR);
            message.addQuery(query);

            // Include PTR records for the target that are already known
            QList<Record> records;
            if (cache->lookupRecords(target, PTR, records)) {
                for (const Record &record : qAsConst(records)) {
                    message.addRecord(record);
                }
            }
        }

        server->sendMessageToAll(message);
        ptrTargets.clear();
    }
}

void BrowserPrivate::updateHostnames()
{
    hostnames.clear();

    for (const auto& service : services) {
        hostnames.insert(service.hostname());
    }
}

Browser::Browser(AbstractServer *server, const QByteArray &type, Cache *cache, QObject *parent)
    : QObject(parent),
      d(new BrowserPrivate(this, server, type, cache))
{
}
