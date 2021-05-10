let yt_player = null;
let yt_player_ready = false;

class YoutubePlayer {
    constructor(video_width, video_height) {
        this.player = new YT.Player('player', {
            height: '' + video_height,
            width: '' + video_width,
            videoId: 'M7lc1UVf-VE',
            events: {
                'onReady': this.onPlayerReady.bind(this),
                'onStateChange': this.onPlayerStateChange.bind(this)
            }
        });
        this.on_play_finished = null;
    }

    onPlayerReady(event) {
        on_player_load("youtube", this);
    }

    // 5. The API calls this function when the player's state changes.
    //    The function indicates that when playing a video (state=1),
    //    the player should play for six seconds and then stop.
    onPlayerStateChange(event) {
        if (event.data == 0 && this.on_play_finished) { // ended
            this.on_play_finished(this);
        }
    }

    play_video_id(vid) {
        this.player.loadVideoById({videoId:vid});
    }
}

// 3. This function creates an <iframe> (and YouTube player)
//    after the API code downloads.
function onYouTubeIframeAPIReady() {
    new YoutubePlayer(video_width, video_height);
}

dyn_module_load("https://www.youtube.com/iframe_api");
