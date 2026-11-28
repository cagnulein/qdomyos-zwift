// Runs src/inner_templates/chartjs/dochart.js (the Charts page, also the charts of the workout
// email) in Node without a browser, for tests/test_chart_smoothing.py.
//
// stdin: {"source": <path of dochart.js>, "settings": {<key>: <value as the app sends it>},
//         "running": <the workout runs: the page polls it>, "session": [<points of getsessionarray>],
//         "liveSession": [<points>] (optional: the session again while the workout runs),
//         "windowsFor": {"seconds": [...], "running": <bool>} (optional: smoothWindow() of these lengths),
//         "stopLater": <bool> (optional: the page stays open, then the workout is stopped)}
// stdout: {"renders": [{"charts": [...], "texts": {...}}, ...], "errors": [<console.error of the page>],
//          "windows": [...] or null, "stopAt": <renders drawn before the stop> or null} –
//   one render per drawing of the page:
//   every chart given to Chart.js (canvas id, type, every line: label, interpolation, points,
//   whether it is drawn with the animation – the screenshots of the email are taken at its end) and
//   the texts of the summary. jQuery, Chart.js, the theme and the app server are stubs: only what
//   dochart.js computes is compared, which is what Chart.js draws and saves for the email
'use strict';

const fs = require('fs');
const vm = require('vm');

const input = JSON.parse(fs.readFileSync(0, 'utf8'));
const renders = [];
let current = null;

function render() {
    if (current === null) {
        current = { charts: [], texts: {} };
        renders.push(current);
    }
    return current;
}

function jquery(selector) {
    const key = typeof selector === 'string' ? selector : '';
    const element = {};
    const chain = {
        0: element,
        length: 1,
        text(value) {
            if (value !== undefined)
                render().texts[key] = String(value);
            return chain;
        },
        attr() { return chain; },
        removeAttr() { return chain; },
        hide() { return chain; },
        show() { return chain; },
        css() { return chain; },
        on() { return chain; },
        ready() { return chain; },
    };
    return chain;
}

function canvas(id) {
    return {
        id: id,
        getContext() { return { canvas: this }; },
        getBoundingClientRect() { return { top: 0, bottom: 100 }; },
    };
}

const canvases = {};
const theme = {
    create(ctx, config) {
        render().charts.push({
            canvas: ctx.canvas.id,
            type: config.type,
            lines: config.data.datasets.map(function (d) {
                return {
                    label: d.label === undefined ? null : d.label,
                    cubicInterpolationMode: d.cubicInterpolationMode === undefined ? null : d.cubicInterpolationMode,
                    data: d.data,
                };
            }),
            // false: drawn without the animation, so without the screenshots of onComplete
            animation: !(config.options && config.options.animation === false),
        });
        return { ctx: ctx, data: config.data, options: config.options, update() {}, destroy() {} };
    },
    restyle() {},
    image() { return ''; },
    sportIcon() {},
    withLight() {},
    snapshotOptions() { return {}; },
    background() { return ''; },
};

// the app server: the settings, the session and the state of the workout
function MainWSQueueElement(request, handler) {
    this.enqueue = function () {
        const msg = request && request.msg;
        let answer = null;
        if (msg === 'getsettings')
            answer = { msg: 'R_getsettings', content: input.settings };
        else if (msg === 'getsessionarray')
            answer = { msg: 'R_getsessionarray', content: input.session };
        else if (msg === null)
            answer = { msg: 'workout', content: { deviceId: 1, deviceConnected: true, devicePaused: !input.running } };
        if (answer === null)
            return new Promise(function () {}); // no reply (the Peloton picture), as without a class
        const result = handler(answer);
        return result === null ? Promise.reject(new Error('no answer to ' + msg)) : Promise.resolve(result);
    };
}

const timers = []; // {id, fn}: run by runTimers(), not by time
let timerId = 0;
const cancelled = new Set(); // ids given to clearTimeout
const errors = []; // console.error of the page
const window = { innerWidth: 1080, innerHeight: 2400, addEventListener() {} };
const context = {
    window: window,
    document: {
        hidden: false,
        addEventListener() {},
        getElementById(id) {
            if (!canvases[id])
                canvases[id] = canvas(id);
            return canvases[id];
        },
    },
    $: jquery,
    // no chart is open on a canvas, so every redraw makes the chart anew (makeChart), and its
    // points are what create() gets; the page updates an open chart in place (updateInPlace)
    Chart: { getChart() { return undefined; } },
    qzChartTheme: theme,
    MainWSQueueElement: MainWSQueueElement,
    setTimeout(fn) { timers.push({ id: ++timerId, fn: fn }); return timerId; },
    clearTimeout(id) { cancelled.add(id); },
    console: {
        log() {},
        warn() {},
        error() { errors.push(Array.prototype.join.call(arguments, ' ')); },
    },
};
vm.createContext(context);
vm.runInContext(fs.readFileSync(input.source, 'utf8'), context, { filename: input.source });

function settle() {
    return new Promise(function (resolve) { setImmediate(resolve); });
}

async function runTimers(rounds) {
    for (let r = 0; r < rounds; r++) {
        const due = timers.splice(0);
        for (const t of due) {
            if (cancelled.has(t.id))
                continue;
            current = null; // a drawing from a timer is a render of its own
            t.fn();
            for (let i = 0; i < 10; i++)
                await settle();
        }
    }
}

(async function () {
    context.dochart_init();
    for (let i = 0; i < 20; i++)
        await settle();
    let windows = null;
    if (input.windowsFor) {
        // smoothWindow() for workouts of these lengths, the workout running or not
        vm.runInContext('liveActive = ' + (input.windowsFor.running ? 'true' : 'false') + ';', context);
        windows = input.windowsFor.seconds.map(function (s) { return context.smoothWindow(s); });
    }
    if (input.liveSession) {
        // the workout runs: the page draws the session again in the polling (the 'workout' push
        // says so), as the app does once a second
        current = null;
        vm.runInContext('liveActive = true; liveRefresh = true;', context);
        context.process_arr(input.liveSession);
    }
    let stopAt = null;
    if (input.stopLater) {
        // the page is open while the workout runs, then Stop: the timers of the page (the watch
        // of the 'workout' push every 500 ms, the polling) run as the browser would, a round at a
        // time, and the push says "paused" from the stop on; the session does not grow
        await runTimers(6);
        stopAt = renders.length;
        input.running = false;
        await runTimers(6);
    }
    process.stdout.write(JSON.stringify({ renders: renders, errors: errors, windows: windows, stopAt: stopAt }));
})().catch(function (err) {
    process.stderr.write(String(err && err.stack || err));
    process.exit(1);
});
