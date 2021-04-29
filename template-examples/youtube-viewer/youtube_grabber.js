class YoutubeGrabber {
    constructor(up) {
        this.on_grab = null;
        this.configure(up);
    }
    configure(up) {

    }
    extract_id(el) {
        let match;
        if ((match = /[&?]list=([^&]{16,})/i.exec(el)))
            return match[1];
        else if ((match = /(?:youtube\.com\/(?:[^\/]+\/.+\/|(?:v|e(?:mbed)?)\/|.*[?&]v=)|youtu\.be\/)([^"&?\/\s]{11})/gi.exec(el)))
            return match[1];
        else
            return el;
    }
    add_params(sp) {
    }
    get_settings_form(){
        return null;
    }
    form2params(main_id) {
        
    }
    grab(value) {
        value = this.extract_id(value);
        if (value && value.length == 11) {
            yt_loadVideoData(value, false).then(
                function(video) {
                    console.log('Video ' + value +' is ' + JSON.stringify(video.video));
                    //this.on_grab(this.videos);
                    video.video.meta.uid = video.video.videoID;
                    video.video.meta.link = 'https://www.youtube.com/watch?v=' + video.video.uid;
                    if (this.on_grab)
                        this.on_grab([video.video.meta], value, 'youtube');
                }.bind(this)
            ).catch(function(error) {
                console.error('Error for video ' + value +' is '+ error);
                if (this.on_grab)
                    this.on_grab([], value, 'youtube');
            }.bind(this));
        }
        else if (value && value.match(/^([A-Z]{2}[a-zA-Z0-9_-]{16,})$/)) {
            yt_loadPlaylistData(value, false).then(
                function(playlist) {
                    console.log('playlist ' + value +' is ' + JSON.stringify(playlist));
                    playlist.videos.forEach(function (video) {
                        video.uid = video.videoID;
                        video.link = 'https://www.youtube.com/watch?v=' + video.uid;
                    });
                    if (this.on_grab)
                        this.on_grab(playlist.videos, value, 'youtube');
                }.bind(this)
            ).catch(function(error) {
                console.error('Error for playlist ' + value +' is '+ error);
                if (this.on_grab)
                    this.on_grab([], value, 'youtube');
            }.bind(this));
        }
        else {
            setTimeout(function() {
                console.error(value + ' is not a valid playlist or video id');
                if (this.on_grab)
                    this.on_grab([], value, 'youtube');
            }.bind(this), 0);
        }
    }
}

let youtube_playlist_onload2 = function() {
    on_grabber_load('youtube', new YoutubeGrabber(urlParams));
};

let youtube_playlist_onload1 = function() {
    dyn_module_load('./senereal_yt_sost.js', youtube_playlist_onload2);
};
fetcher_fetch('https://raw.githubusercontent.com/Seneral/FlagPlayer/master/page/page.js').then(
    function(response) {
        return response.text();
    }
). then( function(script) {
    dyn_module_load('//' + script.replace('ct_init();', '') + '\nyoutube_playlist_onload1();\n');
}).catch(
    function(error) {
        console.log('Error is ' + error);
    }
);
