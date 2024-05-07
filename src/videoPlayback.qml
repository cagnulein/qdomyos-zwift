import QtMultimedia 5.15
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.15
import QtQuick 2.15

Rectangle {
    anchors.fill: parent

    Timer {
        id: pauseTimer
        interval: 1000; running: true; repeat: true
        onTriggered: { (rootItem.currentSpeed > 0 ?
                            videoPlayback.play() :
                            videoPlayback.pause()) }
    }

    MediaPlayer {
           id: videoPlayback
           source: rootItem.videoPath
           autoPlay: false
           playbackRate: rootItem.videoRate

           onError: {
               if (videoPlayback.NoError !== error) {
                   console.log("[qmlvideo] VideoItem.onError error " + error + " errorString " + errorString)
               }
           }

       }

    VideoOutput {
             id:videoPlayer
             anchors.fill: parent
             source: videoPlayback

             Component.onCompleted: {
                 console.log("mediaPlayer onCompleted: " + rootItem.videoPath)
                 videoPlayback.seek(rootItem.videoPosition)
                 videoPlayback.play()
             }
         }
}
