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
import QtQml 2.15
import QtQuick.Controls 2.15
import org.cagnulein.qdomyoszwift 1.0

Rectangle {
    id: storeItem

    property Product product: undefined

    // Modern look: the purchase is shown inside the card ("" – no purchase yet, "buying",
    // "failed", "done"): the status in place of the description, the spinner in place of the
    // price. The classic look keeps its own overlay (pendingRect)
    property string purchaseState: ""
    readonly property bool modernStatus: window.ui.modern && purchaseState !== ""

    state: "NORMAL"

    visible: product.status == Product.Registered
    // Modern look: a filled card of the theme with room around the text
    readonly property int pad: window.ui.modern ? 16 : 0
    radius: window.ui.modern ? 16 : 10
    color: window.ui.modern ? window.ui.surface : "white"

    height: titleText.contentHeight + descriptionText.contentHeight +  2 + 2 * pad
    // ![0]
    Text {
        // a plain Text does not inherit the window font: the text size while it is not 100 %.
        // Modern look: the sizes of the type scale, as the profile cards (the platform default
        // of a plain Text is smaller than the page text on Android and larger on iOS)
        Binding on font.pixelSize { when: !window.ui.modern && window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
        Binding on font.pixelSize { when: window.ui.modern; value: window.ui.fontBody; restoreMode: Binding.RestoreBindingOrValue }
        id: titleText
        text: product.title
        font.weight: window.ui.modern ? Font.DemiBold : Font.Bold
        anchors.right: priceText.left
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: storeItem.pad
        anchors.leftMargin: storeItem.pad
        anchors.rightMargin: storeItem.pad
        color: window.ui.ink("black")
    }

    Text {
        Binding on font.pixelSize { when: !window.ui.modern && window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
        Binding on font.pixelSize { when: window.ui.modern; value: window.ui.fontCaption; restoreMode: Binding.RestoreBindingOrValue }
        id: descriptionText
        text: !storeItem.modernStatus ? product.description
            : storeItem.purchaseState === "buying" ? "Purchasing..."
            : storeItem.purchaseState === "failed" ? "Purchase Failed"
            : "Purchase Succeeded"
        anchors.right: priceText.left
        anchors.left: parent.left
        anchors.top: titleText.bottom
        // Modern look: room for the status icon before the text
        anchors.leftMargin: storeItem.pad + (statusIcon.visible ? statusIcon.size + 6 : 0)
        anchors.rightMargin: storeItem.pad
        anchors.topMargin: window.ui.modern ? 4 : 0
        wrapMode: Text.WordWrap
        color: !window.ui.modern ? "black"
            : storeItem.purchaseState === "failed" && storeItem.modernStatus ? window.ui.danger
            : storeItem.purchaseState === "done" && storeItem.modernStatus ? window.ui.ok
            : window.ui.textMuted
    }

    // Modern look: the result of the purchase is told by an icon too, not by the colour alone
    // (the same icons as the login state of the services on the settings page)
    ShapeIcon {
        id: statusIcon
        visible: storeItem.modernStatus && storeItem.purchaseState !== "buying"
        name: storeItem.purchaseState === "done" ? "check_circle" : "cancel"
        color: descriptionText.color
        // 16 at the 100 % text size, growing with the caption
        size: Math.round(descriptionText.font.pixelSize * 1.2)
        x: storeItem.pad
        // centred on the first line of the status
        y: descriptionText.y + (descriptionText.contentHeight / Math.max(1, descriptionText.lineCount) - size) / 2
    }

    Text {
        Binding on font.pixelSize { when: !window.ui.modern && window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
        Binding on font.pixelSize { when: window.ui.modern; value: window.ui.fontBody; restoreMode: Binding.RestoreBindingOrValue }
        id: priceText
        text: product.price
        // Modern look: the spinner takes its place while buying
        visible: !(storeItem.modernStatus && storeItem.purchaseState === "buying")
        anchors.right: parent.right
        anchors.rightMargin: storeItem.pad
        anchors.verticalCenter: parent.verticalCenter
        color: window.ui.modern ? window.ui.accent : "black"
        font.weight: window.ui.modern ? Font.DemiBold : Font.Normal
    }

    BusyIndicator {
        id: modernSpinner
        visible: storeItem.modernStatus && storeItem.purchaseState === "buying"
        running: visible
        width: 32
        height: 32
        padding: 0
        anchors.right: parent.right
        anchors.rightMargin: storeItem.pad
        anchors.verticalCenter: parent.verticalCenter
    }

    MouseArea {
        anchors.fill: parent
        // Modern look: the card stays tappable while buying, as on the classic look: an iOS
        // "Ask to Buy" request sends no result, so a disabled card would stay stuck
        onClicked: {
            if (window.ui.modern) {
                storeItem.purchaseState = "buying";
                product.purchase();
                return;
            }
            pendingRect.visible = true;
            spinBox.visible = true;
            statusText.text = "Purchasing...";
            storeItem.state = "PURCHASING";
            product.purchase();
        }
        onPressed: {
            storeItem.state = "PRESSED";
        }
        onReleased: {
            storeItem.state = "NORMAL";
        }
    }
    // ![0]

    Rectangle {
        id: pendingRect
        anchors.fill: parent
        opacity: 0.0
        color: window.ui.modern ? window.ui.surfaceHigh : "white"
        radius: parent.radius
        Text {
            Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
            id: statusText
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: spinBox.left
            anchors.leftMargin: storeItem.pad
            verticalAlignment: Text.AlignVCenter
            color: window.ui.ink("black")
        }
        BusyIndicator {
            id: spinBox
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: height
        }

        Connections {
            target: product
            function onPurchaseSucceeded() {
                // Modern look: only the answer to a tap (a renewal can arrive on its own on iOS)
                if (storeItem.purchaseState === "buying")
                    storeItem.purchaseState = "done";
                statusText.text = "Purchase Succeeded";
                spinBox.visible = false;

            }
            function onPurchaseFailed(transaction) {
                // Modern look: a purchase the user cancelled in the store sheet is not an error –
                // the card goes back as it was. 1 is InAppTransaction::CanceledByUser: the type is
                // QML_UNAVAILABLE, so Transaction.CanceledByUser is undefined in QML
                if (storeItem.purchaseState === "buying")
                    storeItem.purchaseState = transaction && transaction.failureReason === 1
                        ? "" : "failed";
                statusText.text = "Purchase Failed";
                spinBox.visible = false;
                storeItem.state = "NORMAL";
            }
        }
    }

    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: storeItem
                color: window.ui.modern ? window.ui.surface : "white"
                border.color: "transparent"
            }
            PropertyChanges {
                target: pendingRect
                opacity: 0.0
            }
        },
        State {
            name: "PRESSED"
            PropertyChanges {
                target: storeItem
                color: window.ui.modern ? window.ui.surfaceHigh : "transparent"
                border.color: window.ui.modern ? "transparent" : window.ui.ink("white")
            }
        },
        State {
            name: "PURCHASING"
            PropertyChanges {
                target: pendingRect
                opacity: 1.0
            }
        }
    ]

    transitions: [
        Transition {
            from: "PURCHASING"
            to: "NORMAL"
            NumberAnimation { target: pendingRect; property: "opacity"; duration: 2000; easing.type: Easing.InExpo }
        }
    ]
}
