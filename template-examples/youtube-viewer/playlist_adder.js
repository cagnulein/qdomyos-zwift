let playlist_grabbers_obj = {};
let playlist_raw_list = [];
let playlist_grabbers = ['youtube', 'personal', 'twitch'];
let badge_colors = ['primary', 'success', 'secondary', 'danger', 'warning', 'info', 'light', 'dark'];
let playlist_load_idx = 0;
let playlist_current_id = 0;
let saved_playlists = {};
let loading_playlist = {};

function on_grabber_load(grabber_name, grabber_obj) {
    console.log(grabber_name + ' loaded');
    playlist_grabbers_obj[grabber_name] = grabber_obj;
    let btn = $('button[data-grabber]');
    let grabber_btn = btn.data('grabber');
    if (grabber_btn == grabber_name) {
        let clp = $('#grabber-settings');
        let sf = grabber_obj.get_settings_form(prepare_url, saved_playlists);
        if (sf)
            clp.append(sf);
    }
    grabber_obj.on_grab = process_on_grab;
    playlist_load_grabber_module(playlist_load_idx + 1);
}

function prepare_url() {
    const params = new URLSearchParams();
    for (let [key, value] of Object.entries(playlist_grabbers_obj)) {
        value.form2params(params);
    }
    if (!playlist_raw_list.length) {
        $('#link-result').val('');
        $('#link-button').attr('disabled', true);
    }
    else {
        for (let p of playlist_raw_list) {
            params.append('grabber', p.grabber);
            params.append('par', p.par);
        }
        let v = $('#control-video-id').val();
        if (v)
            params.append('sta', v);
        v = $('#input-playlist-name').val();
        if (v)
            params.append('name', v);
        v = $('#video-height').val();
        if (v)
            params.append('height', v);
        v = $('#video-width').val();
        if (v)
            params.append('width', v);
        v = $('#video-type').val();
        if (v)
            params.append('player', v);
        urlParams = params;
        search_var = urlParams.toString();
        $('#link-result').val(get_url_without_file() + workout_file + get_search_start_char() + params.toString());
        $('#link-button').prop('disabled', false);
    }
}

function stack_toast(objadd) {
    let s1,s2;
    let update_main = true;
    if (typeof(objadd) == 'string') {
        s1 = 'Problem detected';
        s2 = objadd;
        update_main = false;
    }
    else if (objadd) {
        s1 = objadd.par;
        s2 = `I added ${objadd.items.length} videos (${format_duration(objadd.duration)}).`;
    }
    if (update_main) {
        let numvideos = 0, total_duration = 0;
        for (let playlist of playlist_raw_list) {
            numvideos += playlist.items.length;
            total_duration += playlist.duration;
        }
        $('#main-toast .toast-body').text(`Total videos: ${numvideos} (${format_duration(total_duration)})`);
        $('#main-toast').toast(numvideos?'show':'hide');
    }
    if (objadd) {
        $('#secondary-toast .toast-header').children('.text-primary').text(s1);
        $('#secondary-toast .toast-body').text(s2);
        $('#secondary-toast').toast('show');
    }
}

function hide_progress_modal() {
    if (!$('#modal-progress').hasClass('show')) {
        $('#modal-progress').on('shown.bs.modal', function (e) {
            setTimeout(function() {
                $('#modal-progress').modal('hide');
            },1500);
        });
    }
    else
        setTimeout(function() {
            $('#modal-progress').modal('hide');
        },1500);
}

function process_on_grab(listout, searchid, grabbername) {
    hide_progress_modal();
    if (!listout || !listout.length) {
        stack_toast('Cannot add \'' + searchid + '\'. Please check search terms!');
        return;
    }
    playlist_current_id ++;
    let objadd = {
        par: searchid,
        id: playlist_current_id,
        grabber: grabbername,
        items: listout,
        color: badge_colors[playlist_raw_list.length % badge_colors.length],
        duration: 0
    };
    let grabbercolor = badge_colors[playlist_grabbers.indexOf(grabbername) % badge_colors.length];
    let t = $('<div>');
    let ael = $(`
        <a href="#" data-id="${playlist_current_id}" class="badge badge-${objadd.color} even-larger-badge playlist${playlist_current_id}"><i data-id="${playlist_current_id}" class="fas fa-times"></i>&nbsp;&nbsp;${t.text(searchid).html()}</a>
        `);
    ael.click(function(e) {
        let ids = $(e.target).data('id');
        if (ids)
            $('.playlist' + ids).remove();
        for(let i = 0; i<playlist_raw_list.length; i++) {
            if (playlist_raw_list[i].id == ids) {
                playlist_raw_list.splice(i, 1);
                break;
            }
        }
        prepare_url();
        stack_toast(null);
    });
    $('.top-buffer').append(ael);
    ael = `
        <a href="#" class="badge badge-${grabbercolor}">${grabbername}</a>
        `;
    let rv = $('.row-video');
    let lastrow = null;
    let numitems = 0;
    let tot_dur = 0;
    let bp = bootstrapDetectBreakpoint();
    let conf = bp.name == 'sm'?1:(bp.name == 'md'?3:4);
    for (let video of listout) {
        if ((numitems % conf) == 0 || !lastrow) {
            lastrow = $('<div class="row row-video">');
            $('#main-container').append(lastrow);
        }
        $('#control-video-id').append(
            $('<option>').addClass('playlist' + playlist_current_id)
                .val(video.uid)
                .text(video.uid + ' - ' +video.title));
        numitems ++;
        let uploader = video?.uploader?.name || 'N/A';
        let durationi = video?.length || video?.dur || 0;
        tot_dur += durationi;
        let duration = format_duration(durationi);
        let thumburl = video?.thumbnailURL || video?.img;
        let divel =  `
            <div class="card col-sm-12 col-md-4 col-lg-3 playlist${playlist_current_id}">
                <a href="${video.link}" target="_blank"><img class="card-img-top" src="${thumburl}"/></a>

                <div class="card-body">
                    <a href="${video.link}" target="_blank"><h5 class="card-title">${t.text(video.title).html()}</h5></a>
                    <p class="card-text"><small class="text-muted">Uploader: ${t.text(uploader).html()}<br />Duration: ${duration}</small></p>
                    <a href="#" class="badge badge-${objadd.color}">${t.text(video.uid).html()}</a>${ael}
                </div>
            </div> `;
        $(lastrow).append($(divel));
    }
    objadd.duration = tot_dur;
    playlist_raw_list.push(objadd);
    stack_toast(objadd);
    prepare_url();
    if (loading_playlist.idx !== undefined)
        load_saved_playlist_item(loading_playlist.idx + 1);
}

function playlist_load_grabber_module(idx) {
    if (idx < playlist_grabbers.length) {
        playlist_load_idx = idx;
        dyn_module_load('./' + playlist_grabbers[idx] + '_grabber.js');
    }
}

function load_saved_playlist_item(idx) {
    if (idx < loading_playlist.grabbers.length) {
        loading_playlist.idx = idx;
        playlist_grabbers_obj[loading_playlist.grabbers[idx]].grab(loading_playlist.pars[idx]);
    }
    else {
        let v;
        if ((v = loading_playlist.up.get('sta'))) {
            let clp = $('#control-video-id');
            clp.val(v);
            clp.trigger('change');
        }
        loading_playlist = {};
    }
}

function playlist_adder_init() {
    urlParams = new URLSearchParams(search_var);
    let elements = $('[data-grabber]');
    elements.click(function(event) {
        let et = $(event.target);
        if (et.prop('tagName') == 'I') {
            et = et.parent();
        }
        let val = $('#video-input').val();
        let grabber = et.data('grabber');
        console.log('Grabber onclickr '+ grabber + ' tn = ' + et.prop('tagName'));
        if (et.prop('tagName') == 'A') {
            let btn = $('button[data-grabber]');
            let grabber_btn = btn.data('grabber');
            console.log('Elem a with grabber '+ grabber + ' btn ' + grabber_btn);
            if (grabber_btn != grabber) {
                btn.attr('data-grabber', grabber);
                btn.data('grabber', grabber);
                btn.html(et.html());
                let gs = $('#grabber-settings');
                gs.empty();
                let sf = playlist_grabbers_obj[grabber].get_settings_form(prepare_url, saved_playlists);
                if (sf)
                    gs.append(sf);
            }
            return;
        }
        if (val && val.length && grabber) {
            $('#modal-progress p').text(`Please wait while downloading ${val} playlist with ${grabber} grabber`);
            $('#modal-progress').modal('show');
            playlist_grabbers_obj[grabber].grab(val);
        }
    });
    $('#link-button').click(function(e) {
        let form = $('form');
        if (form[0].checkValidity()) {
            let val = $('#link-result').val();
            if (val && val.length) {
                save_playlist_settings(urlParams);
            }
        }
        form.addClass('was-validated');

    });
    $('#video-width').on('input', function(e) {
        prepare_url();
    });
    $('#video-height').on('input', function(e) {
        prepare_url();
    });
    $('#video-type').on('input', function(e) {
        prepare_url();
    });
    let clp = $('#control-load-playlist');
    for (let pl_name of Object.keys(saved_playlists))
        clp.append($('<option>').val(pl_name).text(pl_name));
    clp.on('change', function(e) {
        let v = $(this).val();
        $('.even-larger-badge').click();
        if (saved_playlists[v]) {
            let up = new URLSearchParams(saved_playlists[v]);
            let grabbers = up.getAll('grabber');
            let pars = up.getAll('par');
            loading_playlist.grabbers = grabbers;
            loading_playlist.pars = pars;
            loading_playlist.up = up;
            loading_playlist.idx = 0;
            loading_playlist.name = v;
            $('#input-playlist-name').val(v);
            if ((v = loading_playlist.up.get('height'))) {
                $('#video-height').val(v);
            }
            if ((v = loading_playlist.up.get('width'))) {
                $('#video-width').val(v);
            }
            if ((v = loading_playlist.up.get('player')))
                $('#video-type').val(v);
            else
                $('#video-type').val('youtube');
            for (let [grabber_name, go] of Object.entries(playlist_grabbers_obj)) {
                go.configure(loading_playlist.up);
            }
            load_saved_playlist_item(0);
        }
    });
    clp = $('#control-video-id');
    clp.on('change', function(e) {
        prepare_url();
    });
    clp = $('#input-playlist-name');
    clp.change(function(e) {
        prepare_url();
    });
    playlist_load_grabber_module(0);
}

function save_playlist_settings(urlParams) {
    let settings_key_pre = get_template_name() + '_conf_';
    let key = settings_key_pre + urlParams.get('name');
    let content_obj = {};
    content_obj[key] = urlParams.toString();
    let el = new MainWSQueueElement({
        msg: 'setsettings',
        content: content_obj
    }, function(msg) {
        return msg.msg === 'R_setsettings' && msg.content[key] !== undefined? msg.content:null;
    }, 3000, 3);
    el.enqueue().then(function(settings) {
        console.log('Settings saved ' + JSON.stringify(settings));
        let params = new URLSearchParams();
        params.append('name', urlParams.get('name'));
        window.location = get_url_without_file() + workout_file + get_search_start_char() + params.toString();
    })
        .catch(function(err) {
            stack_toast('Playlist NOT saved ' + err);
            console.log('Settings NOT saved ' + err);
        });
}

function get_startup_settings() {
    let settings_key_pre = get_template_name() + '_conf_';
    let allconf_key = '$' + settings_key_pre + '*';
    let el = new MainWSQueueElement({
        msg: 'getsettings',
        content: {
            keys: [allconf_key]
        }
    }, function(msg) {
        if (msg.msg === 'R_getsettings' && msg.content[allconf_key]!==undefined) {
            return msg.content;
        }
        else
            return null;
    }, 5000, 3);
    el.enqueue().then(function(playlists) {
        if (playlists) {
            for (let [key, value] of Object.entries(playlists)) {
                if (!key.startsWith('$') && !key.endsWith(lastconf_key)) {
                    let pls_name = key.substring(settings_key_pre.length);
                    saved_playlists[pls_name] = value;
                }
            }
        }
        playlist_adder_init();
    })
        .catch(function(err) {
            console.error('Cannot load saved playlists: ' + err);
            playlist_adder_init();
        });
}

$(window).on('load', function() {
    $('#video-width').inputSpinner();
    $('#video-height').inputSpinner();
    get_startup_settings();
});
