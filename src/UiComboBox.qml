import QtQuick 2.15
import QtQuick.Controls 2.15

// ComboBox of the settings pages and the wizard. On Android 12+ the window runs under the
// status and navigation bars (edge to edge), and with the default 12 px margins a long list
// (App Language, tile order) was pushed under the status bar, hiding its first item.
ComboBox {
    popup.topMargin: 12 + window.getTopPadding()
    popup.bottomMargin: 12 + window.getBottomPadding()
}
