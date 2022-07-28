import QtMultimedia 5.15
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.15
import QtQuick 2.15

Rectangle {
    anchors.fill: parent
    Video {
        id: video
        anchors.fill: parent
        source: rootItem.videoPath
        autoPlay: false
        playbackRate: rootItem.videoRate

        Component.onCompleted: {
            video.seek(rootItem.videoPosition)
            video.play()
        }

    }
}
