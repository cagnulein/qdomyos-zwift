import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.0
import QtQuick.Layouts 1.12

Dialog {
    id: genericDeviceDialog

    property var deviceNames: []
    property var deviceAddresses: []
    property var deviceServiceTypes: []
    property int selectedDeviceIndex: 0

    title: "Unknown Bluetooth Device Found"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel

    width: Math.min(parent.width * 0.9, 500)
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)

    // Get current selected device info
    function getSelectedDeviceName() {
        return deviceNames[selectedDeviceIndex] || "";
    }

    function getSelectedDeviceAddress() {
        return deviceAddresses[selectedDeviceIndex] || "";
    }

    function getSelectedServiceType() {
        return deviceServiceTypes[selectedDeviceIndex] || "";
    }

    function getServiceDisplayName() {
        var serviceType = getSelectedServiceType();
        if (serviceType === "power") {
            return "Cycling Power Service";
        } else if (serviceType === "ftms") {
            return "Fitness Machine Service (FTMS)";
        }
        return "Unknown";
    }

    function isFTMSDevice() {
        return getSelectedServiceType() === "ftms";
    }

    contentItem: ColumnLayout {
        spacing: 15

        Label {
            text: "QZ found a Bluetooth device that is not recognized:"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.bold: true
        }

        // Device selection
        RowLayout {
            Layout.fillWidth: true
            visible: deviceNames.length > 1

            Label {
                text: "Select Device:"
            }

            ComboBox {
                id: deviceComboBox
                Layout.fillWidth: true
                model: deviceNames
                currentIndex: selectedDeviceIndex
                onCurrentIndexChanged: {
                    selectedDeviceIndex = currentIndex;
                }
            }
        }

        // Show selected device name if only one device
        Label {
            text: "Device: " + getSelectedDeviceName()
            visible: deviceNames.length === 1
            font.bold: true
        }

        Label {
            text: "Service Type: " + getServiceDisplayName()
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#cccccc"
        }

        // Device type selection (only for FTMS)
        Label {
            text: "Please select the device type:"
            visible: isFTMSDevice()
            font.bold: true
        }

        ColumnLayout {
            spacing: 8
            visible: isFTMSDevice()
            Layout.leftMargin: 20

            RadioButton {
                id: bikeRadio
                text: "Bike"
                checked: true
            }

            RadioButton {
                id: treadmillRadio
                text: "Treadmill"
            }

            RadioButton {
                id: ellipticalRadio
                text: "Elliptical"
            }

            RadioButton {
                id: rowerRadio
                text: "Rower"
            }
        }

        // Info for Power Sensor
        Label {
            text: "This device will be configured as a Power Sensor Bike."
            visible: !isFTMSDevice()
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.italic: true
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#cccccc"
        }

        Label {
            text: "Note: If you click Cancel, this dialog will not appear again until you restart the app."
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: 12
            color: "#666666"
        }
    }

    onAccepted: {
        // Determine device type selection
        var deviceType = "";
        if (isFTMSDevice()) {
            if (bikeRadio.checked) deviceType = "bike";
            else if (treadmillRadio.checked) deviceType = "treadmill";
            else if (ellipticalRadio.checked) deviceType = "elliptical";
            else if (rowerRadio.checked) deviceType = "rower";
        }

        // Show confirmation dialog asking if user wants to report to team
        reportConfirmDialog.deviceName = getSelectedDeviceName();
        reportConfirmDialog.deviceAddress = getSelectedDeviceAddress();
        reportConfirmDialog.serviceType = getSelectedServiceType();
        reportConfirmDialog.deviceType = deviceType;
        reportConfirmDialog.open();
    }

    onRejected: {
        // User clicked Cancel - restart discovery without showing dialog again
        rootItem.bluetoothManager.genericDeviceDialogShownThisSession = true;
        rootItem.bluetoothManager.restart();
    }

    // Report confirmation dialog
    MessageDialog {
        id: reportConfirmDialog

        property string deviceName: ""
        property string deviceAddress: ""
        property string serviceType: ""
        property string deviceType: ""

        title: "Report Device to Team"
        text: "Would you like to report this device to the QZ team?"
        informativeText: "This will help us add native support for your device in future releases.\n\n" +
                         "An email will be opened with device information (no personal data or MAC address will be included)."
        buttons: MessageDialog.Yes | MessageDialog.No

        onYesClicked: {
            // Confirm with report to team
            rootItem.bluetoothManager.confirmGenericDevice(deviceName, deviceType, true, deviceAddress, serviceType);
        }

        onNoClicked: {
            // Confirm without report
            rootItem.bluetoothManager.confirmGenericDevice(deviceName, deviceType, false, deviceAddress, serviceType);
        }
    }
}
