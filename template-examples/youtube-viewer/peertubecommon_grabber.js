class PeertubeGrabber {
    constructor(host, name, up) {
        this.on_grab = null;
        this.configure(up);
        this.host = host;
        this.name = name;
    }
    configure(up) {

    }
    escapeRegExp(string) {
        return string.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'); // $& means the whole matched string
    }
    extract_id(el) {
        let match;
        if ((match = new RegExp(this.escapeRegExp(this.host)+'/w/([a-zA-Z0-9_\\-]+)', 'i').exec(el)))
            return match[1];
        else
            return el;
    }
    get_settings_form(){
        return null;
    }
    form2params(up) {
        this.configure(up);        
    }
    grab(value) {
        value = this.extract_id(value);
        if (value && value.length) {
            fetcher_fetch('https://' + this.host + '/api/v1/videos/' + value).then(
                function(response) {
                    return response.json();
                }
            ). then( function(json_obj) {
                let pls;
                if (json_obj && (pls = json_obj.streamingPlaylists) && pls.length && (pls = pls[0]) && pls.playlistUrl) {
                    let obj = {
                        img: 'https://' + this.host + json_obj.thumbnailPath,
                        uid: value,
                        link: pls.playlistUrl,
                        dur: json_obj.duration,
                        title: json_obj.name
                    };
                    if (this.on_grab)
                        this.on_grab([obj], value, this.name);
                }
                else if (this.on_grab)
                    this.on_grab([], 'Unexpected object ' + JSON.stringify(json_obj), this.name);
                
            }.bind(this)).catch(
                function(error) {
                    setTimeout(function() {
                        console.error('Error is ' + error);
                        if (this.on_grab)
                            this.on_grab([], '' + error, this.name);
                    }.bind(this), 0);
                }
            );
        }
        else {
            setTimeout(function() {
                console.error(value + ' is not a valid playlist or video id');
                if (this.on_grab)
                    this.on_grab([], value, this.name);
            }.bind(this), 0);
        }
    }
}
