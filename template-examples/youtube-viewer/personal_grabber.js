class PersonalGrabber {
    constructor(urlParams) {
        this.on_grab = null;
        this.form =  $(`
            <form class="col-md-12" autocomplete="on">
            <div class="form-group">
                <label for="personal-url">URL</label>
                <input type="url" class="form-control" id="personal-url" aria-describedby="personal-url-help" placeholder="Server URL" required>
                <small id="input-playlist-name-help" class="form-text text-muted">Enter Server URL.</small>
            </div>
            <div class="form-group">
                <label for="personal-username">Username</label>
                <input type="text" class="form-control" id="personal-username" aria-describedby="personal-username-help" placeholder="Server Username" required>
                <small id="input-playlist-name-help" class="form-text text-muted">Enter Server Username.</small>
            </div>
            <div class="form-group">
                <label for="personal-conv">Conv</label>
                <input type="number" placeholder="Conv" id="personal-conv" value="0" min="0" max="2" step="1" required/>
            </div>
            </form>
        `);
        if (this.form.find('#personal-conv').inputSpinner)
            this.form.find('#personal-conv').inputSpinner();
        this.configure(urlParams);
    }
    configure(urlParams) {
        this.url = urlParams.has('personal-url')?urlParams.get('personal-url'):(typeof(DEFAULT_PERSONAL_URL) != 'string'? '':DEFAULT_PERSONAL_URL);
        this.user = urlParams.has('personal-username')?urlParams.get('personal-username'):(typeof(DEFAULT_PERSONAL_USER) != 'string'? '':DEFAULT_PERSONAL_USER);
        this.conv = urlParams.has('personal-conv')?urlParams.get('personal-conv'):0;
        this.ajax_settings = {
            type        : 'GET',
            url         : this.url,
            data: {
                username: this.user,
                name: '',
                conv: this.conv,
                fmt: 'json'
            },
            dataType    : 'json',
            contentType : 'application/json',
            retries     : -1
        };
        this.set_form_values();
    }

    set_form_values() {
        this.form.find('#personal-conv').val(this.conv);
        this.form.find('#personal-username').val(this.user);
        this.form.find('#personal-url').val(this.url);
    }
    get_settings_form(call_on_change){
        this.form.find('#personal-conv').change(call_on_change);
        this.form.find('#personal-username').change(call_on_change);
        this.form.find('#personal-url').change(call_on_change);
        this.set_form_values();
        return this.form;
    }
    form2params(sp) {
        let conv, url, user;
        if ((conv = $('#personal-conv')).length && (url = $('#personal-url')).length && (user = $('#personal-username')).length) {
            sp.append('personal-conv', conv.val());
            sp.append('personal-username', user.val());
            sp.append('personal-url', url.val());
            this.configure(sp);
        }
        else {
            sp.append('personal-conv', this.conv);
            sp.append('personal-username', this.user);
            sp.append('personal-url', this.url);
        }
    }
    grab(pls_name) {
        this.ajax_settings.data.name = pls_name;
        console.log('notify_done is ' + this.notify_done);
        $.ajax(this.ajax_settings).fail(this.on_fail.bind(this)).done(this.notify_done.bind(this));
    }
    on_fail(e) {
        console.error('fail downloading / parsing ' + e);
        if (this.on_grab) {
            this.on_grab([], this.ajax_settings.data.name, 'personal');
        }
    }

    notify_done(json_list) {
        if (this.on_grab) {
            this.on_grab(json_list.items, this.ajax_settings.data.name, 'personal');
        }
    }
}

on_grabber_load('personal', new PersonalGrabber(urlParams));
