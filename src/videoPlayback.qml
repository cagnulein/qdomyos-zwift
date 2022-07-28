import QtMultimedia 5.15
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.15
import QtQuick 2.15

Rectangle {
    anchors.fill: parent
    MediaPlayer {
        id: videoPlayback
        source: rootItem.videoPath
        autoPlay: false
        playbackRate: rootItem.videoRate

        Component.onCompleted: {
            videoPlayback.seek(rootItem.videoPosition)
            videoPlayback.play()
        }
    }
    VideoOutput {
         id:videoPlayer
         anchors.fill: parent
         source: videoPlayback
     }
}
