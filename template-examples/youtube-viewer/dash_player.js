class DashPlayer {
    constructor(video_width, video_height) {
        let $vid = $(
            `<video style="margin-left: auto; margin-right: auto; display: block;" height="${video_height}" width="${video_width}" controls="true" preload="auto">
        `);
        this.player = dashjs.MediaPlayer().create();
        this.player.updateSettings({ 'debug': { 'logLevel': dashjs.Debug.LOG_LEVEL_NONE }});
        this.player.on(dashjs.MediaPlayer.events['PLAYBACK_PLAYING'], this.onPlayerStateChange.bind(this));
        this.player.on(dashjs.MediaPlayer.events['PLAYBACK_ERROR'], this.onPlayerStateChange.bind(this));
        this.player.on(dashjs.MediaPlayer.events['PLAYBACK_PAUSED'], this.onPlayerStateChange.bind(this));
        this.player.on(dashjs.MediaPlayer.events['PLAYBACK_ENDED'], this.onPlayerStateChange.bind(this));
        this.player.on(dashjs.MediaPlayer.events['PLAYBACK_WAITING'], this.onPlayerStateChange.bind(this));
        this.on_play_finished = null;
        this.on_state_changed = null;
        this.first_load = true;
        this.state = VIDEO_STATUS_UNSTARTED;
        $('#player').append($vid);
    }

    /*
    -1 (unstarted)
    0 (ended)
    1 (playing)
    2 (paused)
    3 (buffering)
    5 (video cued).*/
    onPlayerStateChange(event) {
        if (event.type == 'canPlay') {
            this.player.off(dashjs.MediaPlayer.events['CAN_PLAY'], this.onPlayerStateChange.bind(this));
            this.player.play();
        }
        else {
            if (event.type == 'playbackEnded' && this.on_play_finished) { // ended
                this.on_play_finished(this);
            }
            if (this.on_state_changed) {
                if (event.type == 'playbackEnded')
                    this.state = VIDEO_STATUS_ENDED;
                else if (event.type == 'playbackPaused')
                    this.state = VIDEO_STATUS_PAUSED;
                else if (event.type == 'playbackError')
                    this.state = VIDEO_STATUS_BUFFERING;
                else if (event.type == 'playbackWaiting')
                    this.state = VIDEO_STATUS_BUFFERING;
                else if (event.type == 'playbackPlaying')
                    this.state = VIDEO_STATUS_PLAYING;
                this.on_state_changed(this, this.state);
            }
        }
    }

    play_video(url) {
        if (this.first_load) {
            this.player.on(dashjs.MediaPlayer.events['CAN_PLAY'], this.onPlayerStateChange.bind(this));
            this.player.initialize(document.querySelector('video'), url, true);
            this.first_load = false;
        }
        else {
            this.player.attachSource(url);
            this.player.play();
        }
    }

    togglePause() {
        if (this.state == VIDEO_STATUS_PLAYING)
            this.player.pause();
        else
            this.player.play();
    }
}

// dyn_module_load('./dash_sost.js', function() {

// });

dyn_module_load('https://cdn.dashjs.org/latest/dash.all.debug.js', function() {
    on_player_load('dash', new DashPlayer(video_width, video_height));
});

