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

#include "iosinapppurchasetransaction.h"
#include "iosinapppurchasebackend.h"

#import <StoreKit/StoreKit.h>

QT_BEGIN_NAMESPACE

IosInAppPurchaseTransaction::IosInAppPurchaseTransaction(SKPaymentTransaction *transaction,
                                           const TransactionStatus status,
                                           InAppProduct *product,
                                           IosInAppPurchaseBackend *backend)
    : InAppTransaction(status, product, backend)
    , m_nativeTransaction(transaction)
    , m_failureReason(NoFailure)
{
    if (status == PurchaseFailed) {
        m_failureReason = ErrorOccurred;
        switch (m_nativeTransaction.error.code) {
        case SKErrorClientInvalid:
            m_errorString = QStringLiteral("Client Invalid");
            break;
        case SKErrorPaymentCancelled:
            m_errorString = QStringLiteral("Payment Cancelled");
            m_failureReason = CanceledByUser;
            break;
        case SKErrorPaymentInvalid:
            m_errorString = QStringLiteral("Payment Invalid");
            break;
        case SKErrorPaymentNotAllowed:
            m_errorString = QStringLiteral("Payment Not Allowed");
            break;
#if defined(Q_OS_IOS) || defined(Q_OS_TVOS)
        case SKErrorStoreProductNotAvailable:
            m_errorString = QStringLiteral("Store Product Not Available");
            break;
#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(90300) || QT_TVOS_PLATFORM_SDK_EQUAL_OR_ABOVE(90200)
        case SKErrorCloudServicePermissionDenied:
            m_errorString = QStringLiteral("Cloud Service Permission Denied");
            break;
        case SKErrorCloudServiceNetworkConnectionFailed:
            m_errorString = QStringLiteral("Cloud Service Network Connection Failed");
            break;
#endif
                                               // rdar://35589806
#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(100300) // || QT_TVOS_PLATFORM_SDK_EQUAL_OR_ABOVE(100200)
        case SKErrorCloudServiceRevoked:
            m_errorString = QStringLiteral("Cloud Service Revoked");
            break;
#endif
#endif
        case SKErrorUnknown:
        default:
            m_errorString = QString::fromNSString([m_nativeTransaction.error localizedDescription]);
        }
    }
}

void IosInAppPurchaseTransaction::finalize()
{
    [[SKPaymentQueue defaultQueue] finishTransaction:m_nativeTransaction];
}

QString IosInAppPurchaseTransaction::orderId() const
{
    return QString::fromNSString(m_nativeTransaction.transactionIdentifier);
}

InAppTransaction::FailureReason IosInAppPurchaseTransaction::failureReason() const
{
    return m_failureReason;
}

QString IosInAppPurchaseTransaction::errorString() const
{
    return m_errorString;
}

QDateTime IosInAppPurchaseTransaction::timestamp() const
{
    return QDateTime::fromNSDate(m_nativeTransaction.transactionDate);
}

QT_END_NAMESPACE

#include "moc_iosinapppurchasetransaction.cpp"
