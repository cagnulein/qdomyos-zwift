class VideojsPlayer {
    constructor(video_width, video_height) {
        let $vid = $(
            `
            <video-js id="videojs-vid-id" class="vjs-default-skin" controls preload="auto" style="margin-left: auto; margin-right: auto; display: block;" height="${video_height}" width="${video_width}">
        `);
        $('#player').append($vid);
        this.player = videojs('videojs-vid-id');
        this.player.on('ended', this.onPlayerStateChange.bind(this));
        this.player.on('pause', this.onPlayerStateChange.bind(this));
        this.player.on('play', this.onPlayerStateChange.bind(this));
        this.player.on('playing', this.onPlayerStateChange.bind(this));
        this.player.on('stalled', this.onPlayerStateChange.bind(this));
        this.player.on('waiting', this.onPlayerStateChange.bind(this));
        this.player.on('error', this.onPlayerStateChange.bind(this));
        this.player.on('abort', this.onPlayerStateChange.bind(this));
        this.player.ready(this.onPlayerReady.bind(this));
        this.on_play_finished = null;
        this.on_state_changed = null;
        this.state = VIDEO_STATUS_UNSTARTED;
    }

    onPlayerReady() {
        this.player.play();
    }

    /*
    -1 (unstarted)
    0 (ended)
    1 (playing)
    2 (paused)
    3 (buffering)
    5 (video cued).*/
    onPlayerStateChange(event) {
        if (event.type == 'ended' && this.on_play_finished) { // ended
            this.on_play_finished(this);
        }
        if (this.on_state_changed) {
            if (event.type == 'ended')
                this.state = VIDEO_STATUS_ENDED;
            else if (event.type == 'error' || event.type == 'abort')
                this.state = VIDEO_STATUS_UNSTARTED;
            else if (event.type == 'waiting')
                this.state = VIDEO_STATUS_BUFFERING;
            else if (!this.player.paused())
                this.state = VIDEO_STATUS_PLAYING;
            else
                this.state = VIDEO_STATUS_PAUSED;
            this.on_state_changed(this, this.state);
            console.log('New state event ' + event.type);
        }
    }

    play_video(url, typeV) {
        this.player.src({type: typeV || 'application/x-mpegURL', src: url});
        this.player.play();
    }

    togglePause() {
        if (!this.player.paused())
            this.player.pause();
        else
            this.player.play();
    }
}

// dyn_module_load('./dash_sost.js', function() {

// });

dyn_module_load('https://vjs.zencdn.net/7.11.4/video-js.css', function() {
    dyn_module_load('https://vjs.zencdn.net/7.11.4/video.min.js', function() {
        dyn_module_load('https://cdn.jsdelivr.net/npm/@videojs/http-streaming@2.8.2/dist/videojs-http-streaming.min.js', function() {
            on_player_load('videojs', new VideojsPlayer(video_width, video_height));
        });
    });
}, 'css');

