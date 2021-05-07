function training_parse_duration(val) {
    val = val.trim();
    //let s = /^(?:([0-9]+)(?:\s*h\s*|:))?(?:([0-9]+)(?:\s*m\s*|:))?([0-9]+)?(?:\s*s)?$/gi.exec(val);
    let rexps = [{
        rexp: /^(?:([0-9]+)\s*h\s*)?(?:([0-9]+)\s*m\s*)?(?:([0-9]+)\s*s)?$/gi,
        ss: 3,
        mm: 2,
        hh: 1
    },{
        rexp: /^(?:([0-9]+):)?([0-9]+)$/gi,
        ss: 2,
        mm: 1,
        hh: 100
    },{
        rexp: /^([0-9]+):([0-9]+):([0-9]+)$/gi,
        ss: 3,
        mm: 2,
        hh: 1
    }
    ];
    let s;
    for (let r of rexps) {
        if ((s = r.rexp.exec(val))) {
            let dur = 0;
            if (s[r.hh])
                dur+=parseInt(s[r.hh]) * 3600;
            if (s[r.mm])
                dur+=parseInt(s[r.mm]) * 60;
            if (s[r.ss])
                dur+=parseInt(s[r.ss]);
            return dur;
        }
    }
    return null;
}

function training_render_duration_XML(s) {
    let hh = Math.floor(s / 3600);
    let rem = (s % 3600);
    let mm = Math.floor(rem / 60);
    let ss = rem % 60;
    return pad(hh, 2)+':'+pad(mm, 2)+':'+pad(ss, 2);
}

function training_render_duration_hms(s, exclude) {
    let hh = Math.floor(s / 3600);
    let rem = (s % 3600);
    let mm = Math.floor(rem / 60);
    let ss = rem % 60;
    return ((exclude&1) && !hh?'':pad(hh, 2)+' h ')+((exclude&2) && !mm && !hh?'':pad(mm, 2)+' m ')+pad(ss, 2) + ' s';
}

let DurationEditor = function (props, element) {
    // could be implemented more elegant maybe, but works
    this.parse = function (customFormat) {
        return training_parse_duration(customFormat);
    };
    this.render = function (s) {
        return training_render_duration_hms(s);
    };
};