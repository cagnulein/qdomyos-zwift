let training_program_to_load = '';
let training_unsaved_changes = false;

//bootstrap-table-formatter per formattare gli elementi di una certa colonna

function x_editable_adapt_bootstrap_v4() {
    $.fn.editable.defaults.mode = 'inline';
    $.fn.editableform.buttons =
        '<button type="submit" class="btn btn-primary btn-sm editable-submit">' +
        '<i class="fa fa-fw fa-check"></i>' +
        '</button>' +
        '<button type="button" class="btn btn-warning btn-sm editable-cancel">' +
        '<i class="fa fa-fw fa-times"></i>' +
        '</button>';
}

class SecondsDurationStorage {
    constructor() {
        this.myseconds = 0;
    }

    setSecondsValue(value) {
        this.myseconds = parseInt(value);
    }
}

function training_type_detect(rows) {
    let tp = null;
    for (let r of rows) {
        if ((r.speed > 0 && r.forcespeed) || (r.inclination >= 0 && r.inclination <= 100)) {
            tp = '#treadmill-radio';
        }
        else if (r.zoneHR > 0) {
            tp = '#hrzones-radio';
        }
        else {
            tp = '#bike-radio';
        }
        $(tp).prop('checked', true);
        $(tp).trigger('change');
        return;
    }
}

function bootstrap_table_get_data_ws(params) {
    if (!training_program_to_load.length) {
        params.success({
            rows: [],
            total: 0
        });
    }
    else {
        let el = new MainWSQueueElement({
            msg: 'loadtrainingprograms',
            content: training_program_to_load
        }, function(msg) {
            if (msg.msg === 'R_loadtrainingprograms' && msg.content.name == training_program_to_load && msg.content.list!==undefined) {
                return msg.content.list;
            }
            else
                return null;
        }, 5000, 3);
        el.enqueue().then(function(list) {
            if (list && list.length) {
                let rows = list.map(el => {
                    el.duration = training_render_duration_hms(training_parse_duration(el.duration));
                    return el;
                });
                training_type_detect(rows);
                params.success({
                    'rows': rows,
                    'total': rows.length
                });
                training_toast_duration_print();
            }
            else
                params.error(new Error('Empty list!'));
            
        })
            .catch(function(err) {
                console.error('Cannot load saved playlists: ' + err);
                params.error(err);
            });
    }
}

function x_editable_validate_number(parseFun, min, max, name) {
    return function(val) {
        let flt = parseFun(val);
        if (isNaN(flt) || flt > max || flt < min)
            return name +' should be a number between ' + min +' and ' + max +'.';
    };
}

function x_editable_validate_duration(val) {
    let s = training_parse_duration(val);
    if (s === null)
        return 'Please specify a valid duration.';
    else if (!s)
        return 'Please specify a not null duration.';
}


function x_editable_seconds_display(value, sourceData) {
    $(this).html(training_render_duration_hms(training_parse_duration(value + ''), 3));
}

function x_editable_round_display(value, sourceData) {
    let f = Math.round(value);
    $(this).html(f < 0?'Ignored':f+'');
}

function x_editable_forcespeed_display(value, sourceData) {
    let check;
    if (Array.isArray(value))
        check = value[0].length;
    else if (typeof(value) == 'string')
        check = parseInt(value);
    else
        check = value;
    $(this).html(check?'<i class="far fa-check-square"></i>&nbsp;&nbsp;Speed ON':'<i class="far fa-square"></i>&nbsp;&nbsp;Speed OFF');
}

function x_editable_zoneHR_display(value, sourceData) {
    if (value == 1) $(this).append($('<span class="badge badge-info">Zone 1</span>'));
    else if (value == 2) $(this).append($('<span class="badge badge-success">Zone 2</span>'));
    else if (value == 3) $(this).append($('<span class="badge badge-warning">Zone 3</span>'));
    else if (value == 4) $(this).append($('<span class="badge badge-danger">Zone 4</span>'));
}

function x_editable_configure_duration(idx, row) {
    return {
        validate: x_editable_validate_duration,
        name: 'duration',
        type: 'durationinputspinner'
    };
}

function x_editable_configure_speed(idx, row) {
    return {
        validate: x_editable_validate_number(parseFloat, 0, 100, 'Speed'),
        name: 'speed',
        type: 'inputspinner',
        display: x_editable_round_display
    };
}

function x_editable_configure_fanspeed(idx, row) {
    return {
        validate: x_editable_validate_number(parseFloat, 0, 100, 'Speed'),
        name: 'speed',
        type: 'inputspinner',
        display: x_editable_round_display
    };
}

function x_editable_configure_forcespeed(idx, row) {
    return {
        name: 'forcespeed',
        type: 'checklist',
        value: [1],    
        source: [
            {value: 1, text: 'Speed ON'}
        ],
        display: x_editable_forcespeed_display
    };
}

function x_editable_configure_inclination(idx, row) {
    return {
        validate: x_editable_validate_number(parseFloat, -1, 100, 'Inclination'),
        name: 'inclination',
        type: 'inputspinner',
        display: x_editable_round_display
    };
}

function x_editable_configure_resistance(idx, row) {
    return {
        validate: x_editable_validate_number(parseFloat, 0, 100, 'Resistance'),
        name: 'resistance',
        type: 'inputspinner',
        display: x_editable_round_display
    };
}

function x_editable_configure_zoneHR(idx, row) {
    return {
        validate: x_editable_validate_number(parseFloat, 1, 4, 'HR Zone'),
        name: 'zoneHR',
        type: 'inputspinner',
        display: x_editable_zoneHR_display
    };
}

function x_editable_configure_requested_peloton_resistance(idx, row) {
    return {
        validate: x_editable_validate_number(parseFloat, 1, 100, 'Peloton Resistance'),
        name: 'requested_peloton_resistance',
        type: 'inputspinner',
        display: x_editable_round_display
    };
}

function x_editable_configure_cadence(idx, row) {
    return {
        validate: x_editable_validate_number(parseFloat, -1, 500, 'Cadence'),
        name: 'cadence',
        type: 'inputspinner',
        display: x_editable_round_display
    };
}

function x_editable_configure_maxSpeed(idx, row) {
    return {
        validate: x_editable_validate_number(parseFloat, 0, 100, 'Maximum Speed'),
        name: 'maxSpeed',
        type: 'inputspinner',
        display: x_editable_round_display
    };
}

function x_editable_configure_loopTimeHR(idx, row) {
    return {
        validate: x_editable_validate_number(parseFloat, 1, 100, 'Loop Time'),
        name: 'loopTimeHR',
        type: 'inputspinner',
        display: x_editable_seconds_display
    };
}

function training_duration_init(form) {
    let training_duration = $(`
        <div class="form-group">
            <label for="training-duration">Duration</label>
            <input type="number" placeholder="Duration" id="training-duration" value="1" min="0" max="7200" step="1" required/>
        </div>
        `);
    training_duration.find('input').inputSpinner({editor: DurationEditor});
    form.prepend(training_duration);
}

let training_fields_defaults = {
    duration: 0,
    speed: -1,
    fanspeed: -1,
    inclination:-1,
    resistance: -1,
    requested_peloton_resistance:-1,
    cadence:-1,
    forcespeed: false,
    loopTimeHR: 10,
    zoneHR:-1,
    maxSpeed:-1
};

let training_templates = {
    'treadmill': {
        'text': 'Treadmill',
        'creation': function() {
            let form = $(`
                <form class="col-md-12" id="form-create" auyocomplete="on">
                    <div class="form-group">
                        <label for="treadmill-speed">Speed</label>
                        <input type="number" placeholder="Treadmill Speed" id="treadmill-speed" value="5" min="-1" max="100" step="1" required/>
                    </div>
                    <div class="form-group">
                        <div class="form-check">
                            <input class="form-check-input" checked="true" type="checkbox" id="treadmill-forcespeed">
                            <label class="form-check-label" for="treadmill-forcespeed">
                                Force Speed
                            </label>
                        </div>
                    </div>
                    <div class="form-group">
                        <label for="treadmill-inclination">Inclination</label>
                        <input type="number" placeholder="Treadmill Inclination" id="treadmill-inclination" value="0" min="-1" max="100" step="1" required/>
                    </div>
                    <div class="form-group">
                        <label for="treadmill-fanspeed">Fan Speed</label>
                        <input type="number" placeholder="Fan Speed" id="treadmill-fanspeed" value="-1" min="0" max="100" step="1" required/>
                    </div>
                </form>
            `);
            form.find('#treadmill-forcespeed').change(function() {
                $('#treadmill-speed').prop('disabled', !this.checked);
            });
            form.find('input[type="number"]').inputSpinner();
            return form;
        },
        'object2form': function(val) {
            $('#treadmill-speed').val(val.speed);
            $('#treadmill-inclination').val(val.inclination);
            $('#treadmill-fanspeed').val(val.fanspeed);
            $('#treadmill-forcespeed').prop('checked', val.forcespeed);
        },
        'form2object': function() {
            return {
                speed: $('#treadmill-speed').val(),
                inclination: $('#treadmill-inclination').val(),
                fanspeed: $('#treadmill-fanspeed').val(),
                forcespeed: $('#treadmill-forcespeed').is(':checked'),
            };
        },
        'columns': ['forcespeed', 'speed', 'inclination', 'fanspeed'],
    },
    'bike': {
        'text': 'Bike',
        'creation': function() {
            let form = $(`
                <form class="col-md-12" id="form-create" auyocomplete="on">
                    <div class="form-group">
                        <label for="bike-cadence">Cadence</label>
                        <input type="number" data-suffix="rpm" placeholder="Bike Cadence" id="bike-cadence" value="0" min="-1" max="500" step="1" required/>
                    </div>
                    <div class="form-group">
                        <label for="bike-resistance">Resistance</label>
                        <input type="number" placeholder="Bike Resistance" id="bike-resistance" value="0" min="-1" max="100" step="1" required/>
                    </div>
                    <div class="form-group">
                        <label for="bike-peloton-resistance">Peloton Resistance</label>
                        <input type="number" placeholder="Peloton Resistance" id="bike-peloton-resistance" value="-1" min="0" max="100" step="1" required/>
                    </div>
                </form>
            `);
            form.find('input[type="number"]').inputSpinner();
            return form;
        },
        'object2form': function(val) {
            $('#bike-cadence').val(val.cadence);
            $('#bike-resistance').val(val.resistance);
            $('#bike-peloton-resistance').val(val.requested_peloton_resistance);
        },
        'form2object': function() {
            return {
                cadence: $('#bike-cadence').val(),
                resistance: $('#bike-resistance').val(),
                requested_peloton_resistance: $('#bike-peloton-resistance').val()
            };
        },
        'columns': ['cadence', 'resistance', 'requested_peloton_resistance'],
    },
    'hrzones': {
        'text': 'Bike',
        'creation': function() {
            let form = $(`
                <form class="col-md-12" id="form-create" auyocomplete="on">
                    <div class="form-group">
                        <label for="hrzones-zonehr">HR Zone</label>
                        <input type="number" placeholder="HR Zone" id="hrzones-zonehr" value="1" min="1" max="4" step="1" required/>
                    </div>
                    <div class="form-group">
                        <label for="hrzones-maxspeed">Max Speed</label>
                        <input type="number" placeholder="Max Speed" id="hrzones-maxspeed" value="0" min="0" max="100" step="1" required/>
                    </div>
                    <div class="form-group">
                        <label for="hrzones-looptimehr">Loop Time</label>
                        <input type="number" data-suffix="s" placeholder="Loop Time" id="hrzones-looptimehr" value="1" min="1" max="100" step="1" required/>
                    </div>
                    <div class="form-group">
                        <label for="treadmill-fanspeed">Fan Speed</label>
                        <input type="number" placeholder="Fan Speed" id="treadmill-fanspeed" value="0" min="0" max="100" step="1" required/>
                    </div>
                </form>
            `);
            form.find('input[type="number"]').inputSpinner();
            return form;
        },
        'object2form': function(val) {
            $('#hrzones-zonehr').val(val.zoneHR);
            $('#hrzones-maxspeed').val(val.maxSpeed);
            $('#hrzones-looptimehr').val(val.loopTimeHR);
            $('#treadmill-fanspeed').val(val.fanspeed);
        },
        'form2object': function() {
            return {
                zoneHR: $('#hrzones-zonehr').val(),
                maxSpeed: $('#hrzones-maxspeed').val(),
                loopTimeHR: $('#hrzones-looptimehr').val(),
                fanspeed: $('#treadmill-fanspeed').val()
            };
        },
        'columns': ['zoneHR', 'maxSpeed', 'loopTimeHR', 'fanspeed'],
    },
};

function training_add_or_edit(e) {
    let form = $('form');
    if (form[0].checkValidity()) {
        let active = $('input[name="type-radio"]:checked').val();
        let obj = training_templates[active].form2object();
        obj.duration = training_render_duration_hms($('#training-duration').val());
        if ($(e.target).attr('id').startsWith('add')) {
            $('#output-table').bootstrapTable('append', {...training_fields_defaults, ...obj});
            training_unsaved_changes = true;
        }
        else {
            let idx = parseInt($('input[name="btSelectItem"]:checked').data('index'), 10);
            if (typeof(idx) == 'number' && idx >=0) {
                $('#output-table').bootstrapTable('updateRow', [{
                    index: idx,
                    row: obj,
                    replace: false
                }]);
                training_unsaved_changes = true;
            }
        }
        training_toast_duration_print();
        button_set_enabled('#upload-button', training_unsaved_changes);
    }
    form.addClass('was-validated');
}

function training_type_change() {
    let active = $('input[name="type-radio"]:checked').val();
    let t = training_templates[active];
    let form = t.creation();
    training_duration_init(form);
    $('#form-container').empty().append(form);
    let bt = $('#output-table');
    for (let c of Object.keys(training_fields_defaults)) {
        bt.bootstrapTable(t.columns.indexOf(c) >=0 || c == 'duration' ?'showColumn':'hideColumn', c);
    }
}

function training_put_selection_in_form() {
    let sels = $('#output-table').bootstrapTable('getSelections');
    if (sels.length) {
        let sel = sels[0];
        let active = $('input[name="type-radio"]:checked').val();
        $('#training-duration').val(training_parse_duration(sel.duration));
        training_templates[active].object2form(sel);
    }
    return sels;
}

function training_init() {
    training_download_programs_from_server();
    let $table = $('#output-table');
    let $load_training = $('#control-load-training');
    $table.bootstrapTable();
    $('#add-button').click(training_add_or_edit);
    $('#edit-button').click(training_add_or_edit);
    $('#remove-button').click(function() {
        let idx = parseInt($('input[name="btSelectItem"]:checked').data('index'), 10);
        if (typeof(idx) == 'number' && idx >=0) {
            $table.bootstrapTable('remove', {
                field: '$index',
                values: [idx]
            });
            training_unsaved_changes = true;
            training_toast_duration_print();
            if (!$table.bootstrapTable('getData').length) {
                button_set_enabled(['#remove-button', '#edit-button', '#upload-button'], false);
            }
            else
                button_set_enabled('#upload-button', true);
        }
    });
    $table.on('check.bs.table uncheck.bs.table ' +
      'check-all.bs.table uncheck-all.bs.table', function() {
        button_set_enabled(['#remove-button', '#edit-button'], training_put_selection_in_form().length);
    });
    $load_training.change(function (e) {
        let v = $load_training.val();
        if (v && v.length) {
            if (training_unsaved_changes) {
                let mod = $('#unsaved-modal');
                mod.modal('show');
                mod.on('hidden.bs.modal', function (e) {
                    $load_training.val('');
                });
            }
            else {
                training_download_program_from_server();
            }
        }
    });
    $('#upload-button').click(function() {
        let nm = $('#training-name').val();
        let data = $table.bootstrapTable('getData').map(el => {
            el.duration = training_render_duration_XML(training_parse_duration(el.duration));
            el.forcespeed = Array.isArray(el.forcespeed)?el.forcespeed.length:el.forcespeed ^ 1;
            for (let k of Object.keys(training_fields_defaults)) {
                if (k != 'duration' && k != 'forcespeed') {
                    if (typeof(el[k]) == 'string')
                        el[k] = parseInt(el[k]);
                }
            }
            return el;
        });
        if (data.length) {
            if (main_ws_is_connected()) {
                let el = new MainWSQueueElement({
                    msg: 'savetrainingprogram',
                    content: {
                        name: nm,
                        list: data
                    }
                }, function(msg) {
                    if (msg.msg === 'R_savetrainingprogram' && msg.content.name == nm && msg.content.list!==undefined) {
                        return msg.content.list;
                    }
                    else
                        return null;
                }, 5000, 3);
                el.enqueue().then(function(list) {
                    if (list) {
                        toast_msg('Training program ' + nm + ' saved: Number of rows ' + list +'.', 'success');
                        training_unsaved_changes = false;
                        if (!$('#control-load-training option[value="' + nm + '"]').length)
                            $load_training.append($('<option>').val(nm).text(nm));
                        $load_training.val(nm);
                    }
                    
                })
                    .catch(function(err) {
                        toast_msg('Error saving training program '+ name +': ' + err, 'danger');
                        console.error('Error saving training program '+ name +': ' + err);
                    });
            }
            else {
                let doc = document.implementation.createDocument('', '', null);
                let rowsElem = doc.createElement('rows');
                let active = $('input[name="type-radio"]:checked').val();
                let cols = training_templates[active].columns;
                cols = [...cols, 'duration'];
                for (let row of data) {
                    let rowElem1 = doc.createElement('row');
                    for (let col of cols) {
                        rowElem1.setAttribute(col, row[col]);
                    }
                    rowsElem.appendChild(rowElem1);
                }
                doc.appendChild(rowsElem);
                let xmlString = '<?xml version="1.0" encoding="UTF-8"?>\n' + XML.toString(doc, true);
                js_download(nm + '.xml', xmlString, 'text/xml');
                toast_msg('Training program ' + nm + ' saved locally (connection with QZ unavailable): Number of rows ' + data.length +'.', 'success');
            }
        }
    });
    $table.on('editable-save.bs.table', function(field, row, rowIndex, oldValue, $el) {
        training_put_selection_in_form();
        training_toast_duration_print();
        training_unsaved_changes = true;
        button_set_enabled('#upload-button', training_unsaved_changes);
    });
    $('input[name="type-radio"]').change(training_type_change);
    $('#unsaved-modal .btn-primary').click(function() {
        $('#unsaved-modal').modal('hide');
        training_download_program_from_server();
    });
    training_type_change();
}

function training_get_total_duration() {
    let data = $('#output-table').bootstrapTable('getData');
    let ss = 0;
    for (let r of data) {
        ss += training_parse_duration(r.duration);
    }
    return ss;
}

function training_toast_duration_print() {
    let name = $('#training-name').val().trim();
    if (name.length) {
        name = ' ' + name;
    }
    $('#training-info').text('Training'+ name +' duration is ' + training_render_duration_hms(training_get_total_duration()) + '.');
}

function toast_msg(msg, type) {
    let div = type == 'danger'?$('<strong>'):$('<i>');
    let el = $(
        `
        <div class="col-md-12 alert alert-${type} alert-dismissible fade show" role="alert">
            ${div.text(msg).prop('outerHTML')}
            <button type="button" class="close" data-dismiss="alert" aria-label="Close">
                <span aria-hidden="true">&times;</span>
            </button>
        </div>
        `);
    $('#alert-row').empty().append(el);
}

function training_download_program_from_server() {
    let v = $('#control-load-training').val();
    training_program_to_load = v;
    $('#training-name').val(v);
    $('#output-table').bootstrapTable('refresh');
    training_unsaved_changes = false;
}

function  training_download_programs_from_server() {
    let el = new MainWSQueueElement({
        msg: 'loadtrainingprograms',
        content: ''
    }, function(msg) {
        if (msg.msg === 'R_loadtrainingprograms' && msg.content.name == '' && msg.content.list!==undefined) {
            return msg.content.list;
        }
        else
            return null;
    }, 5000, 3);
    el.enqueue().then(function(list) {
        if (list && list.length) {
            let clp = $('#control-load-training');
            clp.closest('div').show();
            toast_msg('Existing training programs [' + list.length +']: ' + list.join(','), 'success');
            for (let row of list) {
                clp.append($('<option>').val(row).text(row));
            }
            $('#upload-button').html('<i class="fas fa-cloud-upload-alt"></i>&nbsp;&nbsp;Upload');
        }
        else
            toast_msg('No existing program found', 'info');
        
    })
        .catch(function(err) {
            toast_msg('Cannot load existing training programs: ' + err, 'danger');
            console.error('Cannot load saved trainings: ' + err);
        });
}

$(window).on('load', function () {
    let clp = $('#control-load-training');
    clp.closest('div').hide();
    x_editable_adapt_bootstrap_v4();
    training_init();

});