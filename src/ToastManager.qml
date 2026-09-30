import QtQuick 2.0
import AndroidStatusBar 1.0
import QtQuick.Window 2.12

/**
  * adapted from StackOverflow:
  * http://stackoverflow.com/questions/26879266/make-toast-in-android-by-qml
  * @brief Manager that creates Toasts dynamically
  */
ListView {
    /**
      * Public
      */

    /**
      * @brief Shows a Toast
      *
      * @param {string} text Text to show
      * @param {real} duration Duration to show in milliseconds, defaults to 3000
      */
    function show(text, duration) {
        // Always a number: a ListModel role takes its type from the first insert, so after one
        // toast with a duration an undefined one came back as 0 and the toast lasted 0.6 s
        model.insert(0, {text: text, duration: (typeof duration === "number" && duration > 0) ? duration : 0});
    }

    /**
      * Private
      */

    id: root

    z: Infinity
    spacing: 5
    anchors.fill: parent
    anchors.bottomMargin: (Qt.platform.os === "android" && AndroidStatusBar.apiLevel >= 31) ? 
                         ((Screen.orientation === Qt.PortraitOrientation || Screen.orientation === Qt.InvertedPortraitOrientation) ? 
                          AndroidStatusBar.navigationBarHeight + 10 : 10) : 10
    verticalLayoutDirection: ListView.BottomToTop

    interactive: false

    displaced: Transition {
        NumberAnimation {
            properties: "y"
            easing.type: Easing.InOutQuad
        }
    }
    
    delegate: Toast {
        Component.onCompleted: {
            if (typeof duration === "undefined" || !(duration > 0)) {
                show(text);
            }
            else {
                show(text, duration);
            }
        }
    }

    model: ListModel {id: model}
}
