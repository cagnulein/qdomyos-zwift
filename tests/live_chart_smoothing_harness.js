// Runs a live chart page script (dochartlive.js of chartlive.htm, dotreadmillchartlive.js of
// treadmillchartlive.htm) with livesmooth.js in Node without a browser, for
// tests/test_live_chart_smoothing.py.
//
// stdin: {"sources": [<the scripts in the order of the .htm>], "inits": [<their init functions in
//         the same order, as the window 'load' handlers call them>], "canvas": <id of the canvas of
//         the checked chart>, "dataset": <index of the line in the chart>,
//         "settings": {<key>: <value as the app sends it>}, "session": [<points of getsessionarray>],
//         "workouts": [<the 'workout' messages, one per poll of each chart: the app sends every
//         message to all the charts of the page>]}
// stdout: {"lines": [{"points": [...], "cubicInterpolationMode": ...}, ...], "errors": [...]} –
//   the line as Chart.js gets it: when the chart is made (the session drawn at the page opening)
//   and at every update after a 'workout' message. jQuery, Chart.js and the app server are stubs:
//   only what the page script computes is compared, which is what Chart.js draws
'use strict';

const fs = require('fs');
const vm = require('vm');

const input = JSON.parse(fs.readFileSync(0, 'utf8'));
const lines = [];
const errors = [];
let chart = null;

function snapshot() {
    const d = chart.data.datasets[input.dataset];
    lines.push({
        points: d.data.map(function (p) { return { x: p.x, y: p.y }; }),
        cubicInterpolationMode: d.cubicInterpolationMode === undefined ? null : d.cubicInterpolationMode,
    });
}

function Chart(ctx, config) {
    this.data = config.data;
    this.options = config.options;
    this.update = function () {
        if (chart === this)
            snapshot();
    };
    this.destroy = function () {};
    if (ctx && ctx.canvas && ctx.canvas.id === input.canvas && chart === null) {
        chart = this;
        snapshot();
    }
}

function jquery() {
    const chain = {
        0: {},
        length: 1,
        text() { return chain; },
        attr() { return chain; },
        hide() { return chain; },
        show() { return chain; },
        css() { return chain; },
        on() { return chain; },
        ready() { return chain; },
    };
    return chain;
}

const elements = {};
function element(id) {
    if (!elements[id]) {
        const el = { id: id, offsetParent: {}, style: {} };
        el.getContext = function () { return { canvas: el }; };
        elements[id] = el;
    }
    return elements[id];
}

// the 'workout' messages: each chart of the page gets all of them, so the next one is counted per
// poller (refresh of dochartlive.js, refresh_heart of dochartliveheart.js...), told by the caller
const workoutIndex = {};
function poller() {
    const frames = String(new Error().stack).split('\n');
    // [0] Error, [1] poller, [2] enqueue, [3] the function that polls
    return frames.length > 3 ? frames[3].trim().split(' ')[1] : '';
}

// the app server: the settings, the session and the 'workout' messages; anything else (the
// training program) is not answered
function MainWSQueueElement(request, handler) {
    this.enqueue = function () {
        const msg = request && request.msg;
        let answer = null;
        if (msg === 'getsettings') {
            // as onGetSettings of templateinfosenderbuilder.cpp: the asked keys, null when not saved
            const content = {};
            for (const key of request.content.keys)
                content[key] = key in input.settings ? input.settings[key] : null;
            answer = { msg: 'R_getsettings', content: content };
        }
        else if (msg === 'getsessionarray')
            answer = { msg: 'R_getsessionarray', content: input.session };
        else if (msg === null) {
            const who = poller();
            const n = workoutIndex[who] || 0;
            if (n < input.workouts.length) {
                workoutIndex[who] = n + 1;
                answer = { msg: 'workout', content: input.workouts[n] };
            }
        }
        if (answer === null)
            return new Promise(function () {});
        // the answer comes later, as over the WebSocket: after the other charts of the page have
        // started too (their init in the same window 'load' event)
        return new Promise(function (resolve, reject) {
            setImmediate(function () {
                const result = handler(answer);
                if (result === null)
                    reject(new Error('no answer to ' + msg));
                else
                    resolve(result);
            });
        });
    };
}

const context = {
    window: {},
    document: {
        getElementById: element,
        addEventListener() {},
    },
    $: jquery,
    Chart: Chart,
    MainWSQueueElement: MainWSQueueElement,
    setTimeout() { return 0; },
    clearTimeout() {},
    setInterval() { return 0; },
    clearInterval() {},
    console: {
        log() {},
        warn() {},
        error() { errors.push(Array.prototype.join.call(arguments, ' ')); },
    },
};
vm.createContext(context);
for (const source of input.sources)
    vm.runInContext(fs.readFileSync(source, 'utf8'), context, { filename: source });

function settle() {
    return new Promise(function (resolve) { setImmediate(resolve); });
}

(async function () {
    for (const init of input.inits)
        context[init]();
    for (let i = 0; i < 20 + 10 * input.workouts.length; i++)
        await settle();
    process.stdout.write(JSON.stringify({ lines: lines, errors: errors }));
})().catch(function (err) {
    process.stderr.write(String(err && err.stack || err));
    process.exit(1);
});
