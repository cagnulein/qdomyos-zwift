// Runs a live chart page script (dochartlive.js of chartlive.htm, dotreadmillchartlive.js of
// treadmillchartlive.htm) with livesmooth.js in Node without a browser, for
// tests/test_live_chart_smoothing.py.
//
// stdin: {"sources": [<livesmooth.js>, <page script>], "dataset": <index of the line in the chart>,
//         "settings": {<key>: <value as the app sends it>}, "session": [<points of getsessionarray>],
//         "workouts": [<the 'workout' messages, one per poll of the page>]}
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
    if (config.data.datasets.length > input.dataset && chart === null) {
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

const canvas = {
    offsetParent: {},
    style: {},
    getContext() { return { canvas: canvas }; },
};

let workoutIndex = 0;
// the app server: the settings, the session and the 'workout' messages; anything else (the
// training program) is not answered
function MainWSQueueElement(request, handler) {
    this.enqueue = function () {
        const msg = request && request.msg;
        let answer = null;
        if (msg === 'getsettings')
            answer = { msg: 'R_getsettings', content: input.settings };
        else if (msg === 'getsessionarray')
            answer = { msg: 'R_getsessionarray', content: input.session };
        else if (msg === null && workoutIndex < input.workouts.length)
            answer = { msg: 'workout', content: input.workouts[workoutIndex++] };
        if (answer === null)
            return new Promise(function () {});
        const result = handler(answer);
        return result === null ? Promise.reject(new Error('no answer to ' + msg)) : Promise.resolve(result);
    };
}

const context = {
    window: {},
    document: {
        getElementById() { return canvas; },
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
    context.dochart_init();
    for (let i = 0; i < 20 + 5 * input.workouts.length; i++)
        await settle();
    process.stdout.write(JSON.stringify({ lines: lines, errors: errors }));
})().catch(function (err) {
    process.stderr.write(String(err && err.stack || err));
    process.exit(1);
});
