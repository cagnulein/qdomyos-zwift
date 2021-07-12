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
        this.on_state_changed = null;
    }

    onPlayerReady(event) {
        on_player_load('youtube', this);
    }

    /*
    -1 (unstarted)
    0 (ended)
    1 (playing)
    2 (paused)
    3 (buffering)
    5 (video cued).*/
    onPlayerStateChange(event) {
        if (event.data == VIDEO_STATUS_ENDED && this.on_play_finished) { // ended
            this.on_play_finished(this);
        }
        if (this.on_state_changed)
            this.on_state_changed(this, event.data);
    }

    play_video_id(vid) {
        this.player.loadVideoById({videoId:vid});
    }

    togglePause() {
        if (this.player.getPlayerState() == VIDEO_STATUS_PLAYING)
            this.player.pauseVideo();
        else
            this.player.playVideo();
    }
}

// 3. This function creates an <iframe> (and YouTube player)
//    after the API code downloads.
function onYouTubeIframeAPIReady() {
    new YoutubePlayer(video_width, video_height);
}

dyn_module_load('https://www.youtube.com/iframe_api');
