let playlist_player = null;
let playlist_grabbers = null;
let playlist_pars = null;
let playlist_load_idx = 0;
let playlist_grabbers_obj = {};
let video_manager_obj = null;
let playlist_raw_list = [];
let playlist_saves = [];
let playlist_map = {};
let current_item = null;
let playlist_arr = [];

function init_video_params_from_url() {
    playlist_player = urlParams.has('player')?urlParams.get('player'):'youtube';
    playlist_grabbers = urlParams.getAll('grabber');
    playlist_pars = urlParams.getAll('par');
}

function parse_list(json_list) {
    try {
        playlist_map = {};
        current_item = null;
        clear_playlist();
        if (json_list) {
            playlist_arr = json_list;
            json_list.forEach(function(item, index) {
                playlist_map[item.uid] = index;
                add_video_to_button(item);
            });
            set_playlist_button_enabled(true);
        }
    }
    catch (err) {
        console.log('Error parsing playlist ' + err.message);
    }
}

function process_on_grab(obj_list) {
    playlist_raw_list.push(...obj_list);
    playlist_load_grabber_module(playlist_load_idx + 1);
}

function on_grabber_load(grabber_name, grabber_obj) {
    console.log(grabber_name + ' loaded');
    playlist_grabbers_obj[grabber_name] = grabber_obj;
    grabber_obj.on_grab = process_on_grab;
    grabber_obj.grab(playlist_pars[playlist_load_idx]);
}

function on_play_finished(event) {
    let dir = event && typeof(event.dir) !== 'undefined'? event.dir:1;
    let index;
    let vid = '';
    if (typeof(dir) == 'string') {
        if (typeof(playlist_map[dir]) !== 'undefined') {
            index = playlist_map[dir];
        }
        else {
            vid = dir;
            index = playlist_arr.length;
        }
    }
    else {
        index = current_item? playlist_map[current_item.uid] + dir:0;
    }
    console.log('Index found is ' + index);
    if (index < playlist_arr.length) {
        set_next_button_enabled(index < playlist_arr.length - 1);
        set_prev_button_enabled(index > 0);
        current_item = playlist_arr[index];
        vid = current_item.uid;
        set_video_title(current_item.title);
        set_video_enabled(vid);
    }
    if (vid.length) {
        video_manager_obj.play_video_id(vid);
    }
    save_playlist_settings(vid);
}

function on_player_load(name, manager_obj) {
    video_manager_obj = manager_obj;
    video_manager_obj.on_play_finished = on_play_finished;
    let sta;
    let event = undefined;
    if (urlParams.has('sta') && (sta = urlParams.get('sta')) && playlist_map[sta] !== undefined) {
        event = {dir: sta};
    }
    on_play_finished(event);
}

function go_to_next_video() {
    if (video_manager_obj)
        on_play_finished(null);
}
function go_to_prev_video() {
    if (video_manager_obj)
        on_play_finished({dir: -1});
}

function go_to_video(mydir) {
    if (video_manager_obj)
        on_play_finished({dir: mydir});
}

function playlist_load_grabber_module(idx) {
    if (!playlist_pars.length || !playlist_grabbers.length ||
        playlist_pars.length != playlist_grabbers.length)
        console.error('Check url parameters: they seem to be wrong!');
    else {
        if (idx < playlist_pars.length) {
            playlist_load_idx = idx;
            let playlist_grabber = playlist_grabbers[idx];
            if (!playlist_grabbers_obj[playlist_grabber])
                dyn_module_load('./' + playlist_grabber + '_grabber.js');
            else
                playlist_grabbers_obj[playlist_grabber].grab(playlist_pars[idx]);
        }
        else {
            parse_list(playlist_raw_list);
            dyn_module_load('./' + playlist_player + '_player.js');
        }
    }
}

function save_playlist_settings(vid) {
    if (playlist_saves.length) {
        let content_obj = {};
        if (vid && vid.length) {
            urlParams.delete('sta');
            urlParams.append('sta', vid);
        }
        search_var = urlParams.toString();
        for (let key of playlist_saves) {
            content_obj[key] = search_var;
        }
        let el = new MainWSQueueElement({
            msg: 'setsettings',
            content: content_obj
        }, function(msg) {
            if (msg.msg === 'R_setsettings') {
                for (let key of playlist_saves) {
                    if (msg.content[key] === undefined)
                        return null;
                }
                return msg.content;
            }
            return null;
        }, 3000, 1);
        el.enqueue().then(function(settings) {
            console.log('Settings saved ' + JSON.stringify(settings));
        })
            .catch(function(err) {
                console.log('Settings NOT saved ' + err);
            });
    }
}

function get_startup_settings() {
    let orig_up = new URLSearchParams(search_var);
    const settings_key_pre = get_template_name() + '_conf_';
    let hasname = orig_up.has('name');
    let hasgrabber = orig_up.has('grabber');
    let keys_arr = [];
    let namekey = 'a', lastkey;
    if (hasname)
        keys_arr.push(namekey = settings_key_pre + orig_up.get('name'));
    keys_arr.push(lastkey = settings_key_pre + lastconf_key);
    let onSettingsOK = function(settings) {
        let s = null;
        let delete_name = false;
        if ((s = settings[namekey]) && settings[namekey].length);
        else if ((s = settings[lastkey]) && settings[lastkey].length)
            delete_name = !hasgrabber && hasname;
        if (s) {
            let up = new URLSearchParams(s);
            if (hasgrabber) {
                up.delete('grabber');
                up.delete('par');
            }
            if (delete_name)
                orig_up.delete('name');
            for(let pair of up.entries()) {
                if ((!hasgrabber && (pair[0] == 'par' || pair[0] == 'grabber')) ||
                    !orig_up.has(pair[0]))
                    orig_up.append(pair[0], pair[1]);
            }
        }
        search_var = orig_up.toString();
        let playlist_save_name = orig_up.get('name') || '';
        if (playlist_save_name.length)
            playlist_saves = [namekey = settings_key_pre + playlist_save_name, lastkey];
        else
            playlist_saves = [lastkey];
        urlParams = orig_up;
        init_video_manager();
    };
    let el = new MainWSQueueElement({
        msg: 'getsettings',
        content: {
            keys: keys_arr
        }
    }, function(msg) {
        if (msg.msg === 'R_getsettings') {
            for (let key of keys_arr) {
                if (msg.content[key] === undefined)
                    return null;
            }
            return msg.content;
        }
        return null;
    }, 5000, 3);
    el.enqueue().then(onSettingsOK).catch(function(err) {
        console.error('Error is ' + err+' defaulting');
        if (search_var.indexOf('grabber') < 0)
            search_var = 'grabber=youtube&par=UUXDorkXBjDsh0wNethPe-zQ&grabber=youtube&par=wP6l4MD1tTc&grabber=personal&par=subs';
        else
            playlist_saves = [lastkey];
        urlParams = new URLSearchParams(search_var);
        init_video_manager();
    });
    /*let settings = {};
    for (let key of keys_arr) {
        settings[key] = docCookies.getItem(key);
    }*/
    
}

function init_video_manager() {
    init_width_height_from_url();
    init_video_params_from_url();
    playlist_load_grabber_module(0);
}
get_startup_settings();
