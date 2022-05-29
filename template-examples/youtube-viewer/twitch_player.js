class TwitchPlayer {
    constructor(video_width, video_height) {
        let $vid = $(
            
            `<div id="twitch-video"></div>
        `);
        let options = {
            width: video_width,
            height: video_height,
            autoplay: false,
            muted: false,
            channel: 'twitch',
            // only needed if your site is also embedded on embed.example.com and othersite.example.com
            parent: [window.location.host]
        };
        $('#player').append($vid);
        this.on_play_finished = null;
        this.on_state_changed = null;
        this.vid = TWITCH_VIDEO_ID_PRE;
        this.state = VIDEO_STATUS_UNSTARTED;
        this.embed = this.player = null;
        this.embed = new Twitch.Embed('twitch-video', options);
        this.player = this.embed.getPlayer();
        this.embed.addEventListener(Twitch.Embed.VIDEO_READY, function() {
            this.connect_embed_events();
            on_player_load('twitch', this);
        }.bind(this));
    }

    connect_embed_events() {
        let f = function(ev) {
            return function(x) {
                this.onPlayerStateChange({type: ev});
            }.bind(this);
        }.bind(this);
        let addev = function(ev) {
            this.embed.addEventListener(ev, f(ev));
        }.bind(this);
        addev(Twitch.Embed.PLAY);
        addev(Twitch.Player.ENDED);
        addev(Twitch.Player.PAUSE);
        addev(Twitch.Player.PLAYING);
        addev(Twitch.Player.PLAYBACK_BLOCKED);
        addev(Twitch.Player.OFFLINE);
    }

    /*
    -1 (unstarted)
    0 (ended)
    1 (playing)
    2 (paused)
    3 (buffering)
    5 (video cued).*/
    onPlayerStateChange(event) {
        if (((event.type == Twitch.Player.OFFLINE && !this.vid.startsWith(TWITCH_VIDEO_ID_PRE)) ||  event.type == Twitch.Player.ENDED) && this.on_play_finished) { // ended
            this.on_play_finished(this);
        }
        if (this.on_state_changed) {
            if (event.type == Twitch.Player.OFFLINE ||  event.type == Twitch.Player.ENDED)
                this.state = VIDEO_STATUS_ENDED;
            else if (event.type == Twitch.Player.PLAYBACK_BLOCKED || event.type == Twitch.Player.PAUSE)
                this.state = VIDEO_STATUS_PAUSED;
            else if (event.type == Twitch.Player.PLAY)
                this.state = VIDEO_STATUS_BUFFERING;
            else if (event.type == Twitch.Player.PLAYING || event.type == Twitch.Embed.PLAY)
                this.state = VIDEO_STATUS_PLAYING;
            this.on_state_changed(this, this.state);
        }
    }

    play_video_id(vid) {
        this.vid = vid;
        this.onPlayerStateChange({type: VIDEO_STATUS_PAUSED});
        if (vid.startsWith(TWITCH_VIDEO_ID_PRE)) {
            this.player.setChannel(null);
            this.player.setVideo(vid.substring(TWITCH_VIDEO_ID_PRE.length),5);
        } else if (/^[0-9]{7,}$/i.exec(vid)) {
            this.vid = TWITCH_VIDEO_ID_PRE + vid;
            this.player.setChannel(null);
            this.player.setVideo(vid, 5);
        } else {
            this.player.setVideo(null,5);
            this.player.setChannel(vid);
        }
        this.player.play();
    }

    togglePause() {
        if (this.state != VIDEO_STATUS_PLAYING)
            this.player.play();
        else
            this.player.pause();
    }
}

// dyn_module_load('./dash_sost.js', function() {

// });

dyn_module_load('https://embed.twitch.tv/embed/v1.js', function() {
    new TwitchPlayer(video_width, video_height);
});
