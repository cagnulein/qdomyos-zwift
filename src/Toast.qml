import QtQuick 2.0
import QtQml 2.15

/**
 * adapted from StackOverflow:
 * http://stackoverflow.com/questions/26879266/make-toast-in-android-by-qml
 */

/**
  * @brief An Android-like timed message text in a box that self-destroys when finished if desired
  */
Rectangle {

    /**
      * Public
      */

    /**
      * @brief Shows this Toast
      *
      * @param {string} text Text to show
      * @param {real} duration Duration to show in milliseconds, defaults to 3000
      */
    function show(text, duration) {
        message.text = text;
        if (typeof duration !== "undefined") { // checks if parameter was passed
            time = Math.max(duration, 2 * fadeTime);
        }
        else {
            time = defaultTime;
        }
        animation.start();
    }

    /**
      * Private
      */

    id: root

    readonly property real defaultTime: 3000
    property real time: defaultTime
    readonly property real fadeTime: 300

    property real margin: 10

    anchors {
        left: (parent != null) ? parent.left : undefined
        right: (parent != null) ? parent.right : undefined
        margins: margin
    }

    height: message.height + margin
    radius: window.ui.modern ? 14 : margin

    opacity: 0
    // Modern look: inverse of the theme, dark on a light page and light on a dark one
    color: window.ui.modern ? window.ui.inverseSurface : "#222222"

    Text {
        // a plain Text does not inherit the window font: the text size while it is not 100 %;
        // modern look: Body (16) of the type scale
        Binding on font.pixelSize { when: window.ui.modern || window.scaledFontPx > 0; value: window.ui.modern ? window.ui.fontBody : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
        id: message
        color: window.ui.modern ? window.ui.inverseText : "white"
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: margin / 2
        }
    }

    SequentialAnimation on opacity {
        id: animation
        running: false


        NumberAnimation {
            to: .9
            duration: fadeTime
        }

        PauseAnimation {
            duration: time - 2 * fadeTime
        }

        NumberAnimation {
            to: 0
            duration: fadeTime
        }

        onRunningChanged: {
            if (!running) {
                // the toast is a delegate of ToastManager: drop it from the list once faded
                root.ListView.view.model.remove(index);
            }
        }
    }
}
