/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef INAPPTRANSACTION_H
#define INAPPTRANSACTION_H

#include <QDateTime>
#include <QQmlEngine>
#include <QtCore/qobject.h>
#include <QtCore/qsharedpointer.h>
#include <QtQml/qqml.h>

#include "inappproduct.h"

QT_BEGIN_NAMESPACE

class InAppProduct;
class InAppTransactionPrivate;
class InAppTransaction : public QObject {
    Q_OBJECT
    Q_PROPERTY(TransactionStatus status READ status CONSTANT)
    Q_PROPERTY(InAppProduct *product READ product CONSTANT)
    Q_PROPERTY(QString orderId READ orderId CONSTANT)
    Q_PROPERTY(FailureReason failureReason READ failureReason CONSTANT)
    Q_PROPERTY(QString errorString READ errorString CONSTANT)
    Q_PROPERTY(QDateTime timestamp READ timestamp CONSTANT)
    QML_NAMED_ELEMENT(Transaction)
    QML_UNAVAILABLE

  public:
    enum TransactionStatus { Unknown, PurchaseApproved, PurchaseFailed, PurchaseRestored };
    Q_ENUM(TransactionStatus);

    enum FailureReason { NoFailure, CanceledByUser, ErrorOccurred };
    Q_ENUM(FailureReason);

    ~InAppTransaction();

    InAppProduct *product() const;

    virtual QString orderId() const;
    virtual FailureReason failureReason() const;
    virtual QString errorString() const;
    virtual QDateTime timestamp() const;

    Q_INVOKABLE virtual void finalize() = 0;
    Q_INVOKABLE virtual QString platformProperty(const QString &propertyName) const;

    TransactionStatus status() const;

  protected:
    explicit InAppTransaction(TransactionStatus status, InAppProduct *product, QObject *parent = nullptr);

  private:
    Q_DISABLE_COPY(InAppTransaction)
    QSharedPointer<InAppTransactionPrivate> d;
};

#endif // INAPPTRANSACTION_H
