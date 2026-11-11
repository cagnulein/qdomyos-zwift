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
    id: swagBagPage
    // Modern look: side margins like the other pages, a readable text size instead of the
    // poster-size one, the product as a card and a pill button
    readonly property int modernMargin: Math.max(16, window.contentSideMargin)

    // the intro and the terms grow with the text size: scroll them above the button
    // when they no longer fit (at 100 % they fit and nothing moves)
    Flickable {
        id: swagBagFlick
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: restoreButton.top
        anchors.leftMargin: window.ui.modern ? swagBagPage.modernMargin : 0
        anchors.rightMargin: window.ui.modern ? swagBagPage.modernMargin : 0
        contentHeight: appleDescription.y + appleDescription.height
        interactive: contentHeight > height
        clip: interactive

        Text {
            padding: 5
            id: description
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            color: window.ui.ink("white")
            // already large: it grows at half the rate, so it stays above the body text
            // without filling the screen (about 30 pt at 170 %)
            font.pointSize: window.ui.modern ? 15 * window.textScale : 22 * (1 + (window.textScale - 1) / 2)
            topPadding: window.ui.modern ? 16 : 5
            lineHeight: window.ui.modern ? 1.15 : 1
            wrapMode: TextArea.Wrap
            text: qsTr("Hi! Do you know that QZ is just an Open Source Indie App?<br><br>No Big Companies are running this!<br>The \"Swag Bag\" is a way to support the ongoing development, maintenance and support of QZ Fitness!")
        }
        Column {
            //anchors.top: description.bottom + 10
            anchors.top: description.bottom
            anchors.topMargin: window.ui.modern ? 16 : 0
            //anchors.bottom: restoreButton.top
            anchors.right: parent.right
            anchors.left: parent.left
            id: itemSwagBag

            SwagBagItem {
                product: productUnlockVowels
                width: parent.width
            }
        }
        Text {
            anchors {
                top: itemSwagBag.bottom
                horizontalCenter: parent.horizontalCenter
            }
            padding: 5
            id: appleDescription
            width: parent.width
            topPadding: window.ui.modern ? 16 : 5
            // the default link blue is hard to read on the dark modern page ("blue" is the default)
            linkColor: window.ui.modern ? window.ui.accent : "blue"
            color: window.ui.modern ? window.ui.textMuted : "white"
            font.pointSize: (window.ui.modern ? 11 : 8) * window.textScale
            wrapMode: TextArea.Wrap
            text: qsTr("<html><style type='text/css'></style>Swag bag feature:<br>• an auto-renewable subscription<br>• 1 month ($1.99)<br>• Your subscription will be charged to your iTunes account at confirmation of purchase and will automatically renew (at the duration selected) unless auto-renew is turned off at least 24 hours before the end of the current period.<br>• Current subscription may not be cancelled during the active subscription period; however, you can manage your subscription and/or turn off auto-renewal by visiting your iTunes Account Settings after purchase.<br>• Privacy policy: <a href='https://robertoviola.cloud/privacy-policy-qdomyos-zwift/'>https://robertoviola.cloud/privacy-policy-qdomyos-zwift/</a><br>• Licensed Application end user license agreement: <a href='https://www.apple.com/legal/internet-services/itunes/dev/stdeula/'>https://www.apple.com/legal/internet-services/itunes/dev/stdeula/</a><br></html>")
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }

    UiButton {
        id: restoreButton
        wrapsText: true
        anchors.bottom: parent.bottom
        anchors.bottomMargin: window.ui.modern ? 8 : 0
        anchors.horizontalCenter: parent.horizontalCenter
        // Modern look: the full width between the page margins, as Other folders on the list pages
        width: window.ui.modern ? parent.width - 2 * swagBagPage.modernMargin : parent.width * .5
        text: qsTr("Restore Purchases")
        onClicked: {
            console.log("restoring...");
            toast.show(qsTr("Restoring..."));
            iapStore.restorePurchases();
        }
    }
}
