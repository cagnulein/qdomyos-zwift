import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0

Dialog {
    id: disclaimerDialog
    modal: true
    focus: true
    closePolicy: Dialog.NoAutoClose
    
    width: Math.min(parent.width * 0.9, 600)
    height: Math.min(parent.height * 0.8, 500)
    
    anchors.centerIn: parent
    
    property bool wasShown: settings.grupetto_disclaimer_shown || false
    
    Material.theme: Material.Dark
    Material.accent: Material.Orange
    
    header: Rectangle {
        height: 60
        color: Material.color(Material.Orange)
        
        Text {
            anchors.centerIn: parent
            text: "Legal Disclaimer - Grupetto Integration"
            font.pixelSize: 18
            font.bold: true
            color: "white"
        }
    }
    
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        
        Text {
            width: parent.width
            wrapMode: Text.WordWrap
            color: "white"
            font.pixelSize: 14
            lineHeight: 1.3
            
            text: "IMPORTANT LEGAL NOTICE - THIRD-PARTY CODE DISCLAIMER\n\n" +
                  "This application incorporates code derived from the Grupetto project " +
                  "(https://github.com/spencerpayne/grupetto), which enables communication " +
                  "with Peloton fitness equipment sensors.\n\n" +
                  
                  "LIABILITY DISCLAIMER:\n\n" +
                  "1. The Grupetto-derived code is provided \"AS IS\" without any warranties " +
                  "of any kind, either expressed or implied.\n\n" +
                  
                  "2. The author of QDomyos-Zwift DISCLAIMS ALL RESPONSIBILITY AND LIABILITY " +
                  "for any damages, losses, or issues arising from the use of Grupetto-derived code, " +
                  "including but not limited to:\n" +
                  "   • Equipment damage or malfunction\n" +
                  "   • Data loss or corruption\n" +
                  "   • Personal injury\n" +
                  "   • Software crashes or instability\n" +
                  "   • Unauthorized access to device systems\n\n" +
                  
                  "3. Users assume full responsibility and risk when using features that rely " +
                  "on Grupetto-derived code for Peloton sensor integration.\n\n" +
                  
                  "4. This disclaimer does not affect the warranty or liability for other " +
                  "parts of QDomyos-Zwift not derived from Grupetto.\n\n" +
                  
                  "5. By clicking 'OK', you acknowledge that you have read, understood, " +
                  "and agree to this disclaimer.\n\n" +
                  
                  "ATTRIBUTION:\n" +
                  "Portions of this software are derived from Grupetto, developed by Spencer Payne. " +
                  "Original project: https://github.com/spencerpayne/grupetto"
        }
    }
    
    standardButtons: Dialog.Ok
    
    onAccepted: {
        settings.grupetto_disclaimer_shown = true
        close()
    }
    
    Component.onCompleted: {
        if (!wasShown) {
            open()
        }
    }
}