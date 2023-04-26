class TwitchGrabber {
    constructor(urlParams) {
        this.on_grab = null;
        this.time_last_refresh = 0;
        this.configure(urlParams);
    }
    configure(urlParams) {
        this.token = urlParams.get('twitch-token');
        this.access = urlParams.get('twitch-access');
        this.ajax_settings = {
            type        : 'GET',
            url         : null,
            headers : {
                'Client-Id': TWITCH_CLIENT_ID,
                'Authorization': 'Bearer ' + this.access
            },
            dataType    : 'json',
            contentType : 'application/json',
            retries     : -1
        };
        this.set_form_values();
    }

    set_form_values() {
    }
    get_settings_form(call_on_change, playlists){
        for (let [pn, p] of Object.entries(playlists)) {
            let urlp = new URLSearchParams(p);
            if (urlp.get('grabber') == 'twitch') {
                this.configure(urlp);
                return null;
            }
        }
        this.set_form_values();
        return null;
    }
    form2params(sp) {
        sp.append('twitch-token', this.token);
        sp.append('twitch-access', this.access);
    }

    identify_query(pls_name) {
        let match;
        if ((match = /(channel|vod)\s*=\s*([^\s*]+)/i.exec(pls_name))) {
            let out = {};
            out[match[1]] = match[2];
            return out;
        }
        else
            return null;
    }

    grab(pls_name) {
        let query_type = this.identify_query(pls_name);
        if (!query_type) {
            setTimeout(function() {
                console.error(pls_name + ' is not a valid playlist or video id');
                if (this.on_grab)
                    this.on_grab([], pls_name, 'twitch');
            }.bind(this), 0);
        }
        else if (!this.access || !this.token) {
            if (new Date().getTime() - this.time_last_refresh > 15000) {
                let secure_code = generate_rand_string(32);
                let url = `https://id.twitch.tv/oauth2/authorize?response_type=token+id_token&client_id=${TWITCH_CLIENT_ID}&redirect_uri=${window.location.protocol}//${window.location.host}/${get_template_name()}/twitch-id.htm&scope=viewing_activity_read+openid&state=${secure_code}`;
                window.open(url);
                window.addEventListener('id-window-closed', function(ev) {
                    let event = ev.detail;
                    if (event.error)
                        this.on_fail(event.error);
                    else if (secure_code == event.state) {
                        this.time_last_refresh = new Date().getTime();
                        this.access = event.access;
                        this.token = event.token;
                        this.ajax_settings.headers.Authorization = 'Bearer ' + this.access;
                        this.grab(pls_name);
                    }
                    else {
                        this.on_fail('Secure code error: ' + event.state + ' != ' + secure_code);
                    }
                }.bind(this));
            }
            else
                this.on_grab([], this.ajax_settings.raw_query, 'twitch');
        }
        else  {
            this.ajax_settings.raw_query = pls_name;
            this.ajax_settings.query = query_type.channel || query_type.vod;
            this.ajax_settings.query_type = query_type;
            this.ajax_settings.url = 'https://api.twitch.tv/helix/users';
            this.ajax_settings.data = {login: this.ajax_settings.query};
            $.ajax(this.ajax_settings).fail(this.on_fail.bind(this)).done(this.on_curl_users.bind(this));
        }
    }

    on_fail(jqXHR, textStatus, errorThrown) {
        console.error('fail downloading / parsing ts = ' + textStatus + '; err' + errorThrown);
        if (jqXHR.status == 401 || jqXHR.status == 403) {
            this.access = null;
            this.token = null;
            this.grab(this.ajax_settings.raw_query);
        }
        else if (this.on_grab) {
            this.on_grab([], this.ajax_settings.raw_query, 'twitch');
        }
    }

    on_curl_users(json_obj) {
        if (json_obj.error || !json_obj.data || !(json_obj = json_obj.data[0])) {
            this.access = null;
            console.error(JSON.stringify(json_obj));
            this.grab(this.ajax_settings.raw_query);
        }
        if (this.ajax_settings.query_type.vod) {
            this.ajax_settings.url = 'https://api.twitch.tv/helix/videos';
            this.ajax_settings.data = {user_id: json_obj.id};
            $.ajax(this.ajax_settings).fail(this.on_fail.bind(this)).done(this.on_curl_videos.bind(this));
        }
        else {
            let obj = {
                img: json_obj.profile_image_url,
                uid: this.ajax_settings.query,
                link: json_obj.id,
                dur: 0,
                title: json_obj.display_name
            };

            if (this.on_grab)
                this.on_grab([obj], this.ajax_settings.raw_query, 'twitch');
        }
    }

    on_curl_videos(json_obj) {
        json_obj.data.reverse();
        for (let d of json_obj.data) {
            d.link = TWITCH_VIDEO_ID_PRE + d.id;
            d.uid = TWITCH_VIDEO_ID_PRE + d.id;
            d.img = d.thumbnail_url;
            let match = /^(?:([0-9]+)d)?(?:([0-9]+)h)?(?:([0-9]+)m)?(?:([0-9]+)s)?$/.exec(d.duration);
            let dur = 0;
            if (match) {
                if (match[1])
                    dur += parseInt(match[1]) * 86400;
                if (match[2])
                    dur += parseInt(match[2]) * 3600;
                if (match[3])
                    dur += parseInt(match[3]) * 60;
                if (match[4])
                    dur += parseInt(match[4]);
            }
            d.dur = dur;
        }
        if (this.on_grab) {
            this.on_grab(json_obj.data, this.ajax_settings.raw_query, 'twitch');
        }
    }
}

on_grabber_load('twitch', new TwitchGrabber(urlParams));
