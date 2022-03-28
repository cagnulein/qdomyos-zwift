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

import QtQuick 2.7
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import org.cagnulein.qdomyoszwift 1.0

Item {

    Text {
        id: description
        text: qsTr("Hi! Do you know that QZ is just an Open Souce Indie App?<br>No Big Companies are running this!<br><br>The \"Swag Bag\" is a way to support the ongoing<br>development, maintenance and support of QZ Fitness!<br><br>Thanks to Rungap App to give me the idea of the name!")
    }

    Column {
        anchors.top: description.bottom
        anchors.bottom: restoreButton.top
        anchors.margins: topLevel.globalMargin
        anchors.right: parent.right
        anchors.left: parent.left
        spacing: topLevel.globalMargin

        SwagBagItem {
            product: productUnlockVowels
            width: parent.width
        }
    }

    Button {
        id: restoreButton
        anchors.bottom: parent.bottom
        anchors.margins: topLevel.globalMargin
        anchors.horizontalCenter: parent.horizontalCenter
        height: topLevel.buttonHeight
        width: parent.width * .5
        text: "Restore Purchases"
        onClicked: {
            console.log("restoring...");
            iapStore.restorePurchases();
        }
    }

    Product {
        id: productUnlockVowels
        type: Product.Unlockable
        store: iapStore
        identifier: "org.cagnulein.qdomyoszwift.swagbag"

        onPurchaseSucceeded: {
            console.log(identifier + " purchase successful");
            applicationData.vowelsUnlocked = true;
            transaction.finalize();
            pageStack.pop();
        }

        onPurchaseFailed: {
            console.log(identifier + " purchase failed");
            console.log("reason: "
                        + transaction.failureReason === Transaction.CanceledByUser ? "Canceled" : transaction.errorString);
            transaction.finalize();
        }

        onPurchaseRestored: {
            console.log(identifier + " purchase restored");
            applicationData.vowelsUnlocked = true;
            console.log("timestamp: " + transaction.timestamp);
            transaction.finalize();
            pageStack.pop();
        }
    }
}
