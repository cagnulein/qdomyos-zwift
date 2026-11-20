const fs = require('fs');
const path = require('path');
const vm = require('vm');

const repoRoot = path.resolve(__dirname, '..', '..');
const chartDir = path.join(repoRoot, 'src', 'inner_templates', 'chartjs');
const artifactDir = path.join(__dirname, 'artifacts');
fs.mkdirSync(artifactDir, { recursive: true });

const results = [];
const events = [];
const savedChartNames = [];
const pendingTimers = [];
const renderedText = new Map();
function check(name, condition, details) {
    const ok = Boolean(condition);
    results.push({ name, ok, details });
    console.log(`${ok ? 'PASS' : 'FAIL'}: ${name}${details ? ` - ${details}` : ''}`);
    return ok;
}

class JQueryMock {
    constructor(selector) {
        this.selector = selector;
        this.length = 1;
    }
    text(value) { if (arguments.length) renderedText.set(this.selector, value); return this; }
    attr() { return this; }
    on() { return this; }
    ready() { return this; }
    parent() { return new JQueryMock(`${this.selector}:parent`); }
    hide() { return this; }
    show() { return this; }
    next() { return new JQueryMock(`${this.selector}:next`); }
    prev() { return new JQueryMock(`${this.selector}:prev`); }
    is() { return false; }
    appendTo(target) {
        if (String(this.selector).includes('canvasSpeedInclination')) {
            events.push(`move-speed-container:${target}`);
        }
        return this;
    }
    closest() { return new JQueryMock(`${this.selector}:closest`); }
    find() { return new JQueryMock(`${this.selector}:find`); }
    eq() { return new JQueryMock(`${this.selector}:eq`); }
    removeAttr() { return this; }
}

global.window = global;
global.$ = function(selector) { return new JQueryMock(selector); };
global.setTimeout = function(callback) {
    pendingTimers.push(callback);
    return pendingTimers.length;
};
global.clearTimeout = function() {};
global.html2canvas = function() {
    const canvas = { toDataURL: () => 'data:image/png;base64,treadmill-badge' };
    return {
        then(callback) {
            callback(canvas);
            return { catch() {} };
        },
        catch() {}
    };
};

const canvases = new Map();
function canvasFor(id) {
    if (!canvases.has(id)) {
        const canvas = {
            id,
            toDataURL: () => `data:image/png;base64,${id}`,
            getContext() { return { canvas: this }; }
        };
        canvases.set(id, canvas);
    }
    return canvases.get(id);
}

global.document = {
    getElementById(id) { return canvasFor(id); }
};

class FakeChart {
    static instances = new Map();
    static getChart(item) {
        const canvas = item && item.canvas ? item.canvas : item;
        return FakeChart.instances.get(canvas);
    }
    constructor(item, config) {
        this.canvas = item && item.canvas ? item.canvas : item;
        this.data = config.data || { datasets: [] };
        this.options = config.options || {};
        this.updateCalls = [];
        this.resizeCalls = 0;
        if (this.canvas && this.canvas.id === 'canvasSpeedInclination') {
            events.push('create-speed-chart');
        }
        FakeChart.instances.set(this.canvas, this);
    }
    static completeAnimations() {
        for (const chart of FakeChart.instances.values()) {
            const callback = chart.options.animation && chart.options.animation.onComplete;
            if (typeof callback === 'function') callback();
        }
    }
    update(mode) { this.updateCalls.push(mode); }
    resize() { this.resizeCalls++; }
    toBase64Image() { return this.canvas.toDataURL(); }
}
global.Chart = FakeChart;

global.MainWSQueueElement = class {
    constructor(message) { this.message = message; }
    enqueue() {
        if (this.message && this.message.msg === 'savechart') {
            savedChartNames.push(this.message.content.name);
        }
        return Promise.resolve(null);
    }
};

function sample(overrides) {
    return Object.assign({
        elapsed_h: 0, elapsed_m: 0, elapsed_s: 0,
        deviceType: 1,
        workoutName: 'Test Run', workoutStartDate: '2026-08-20', instructorName: '',
        watts: 0, watts_avg: 0, watts_max: 0, req_power: 0, jouls: 0,
        heart: 140, heart_avg: 140, heart_max: 145,
        cadence: 0, cadence_avg: 0, req_cadence: 0,
        resistance: 0, resistance_avg: 0, req_resistance: 0,
        peloton_resistance: 0, peloton_resistance_avg: 0, peloton_req_resistance: 0,
        speed: 0, inclination: 0, distance: 0, calories: 0
    }, overrides);
}

const fixture = [
    sample({ elapsed_s: 0,  speed: 10, inclination: 2, distance: 0,        heart_avg: 140, calories: 0 }),
    sample({ elapsed_s: 10, speed: 0,  inclination: 4, distance: 0.027778, heart_avg: 141, calories: 2 }),
    sample({ elapsed_s: 20, speed: 12, inclination: 6, distance: 0.061111, heart_avg: 142, calories: 4 })
];
const fixtureBefore = JSON.stringify(fixture);

let fatalError = null;
try {
    const doChartSource = fs.readFileSync(path.join(chartDir, 'dochart.js'), 'utf8');
    const treadmillSource = fs.readFileSync(path.join(chartDir, 'treadmill_summary.js'), 'utf8');
    vm.runInThisContext(doChartSource, { filename: 'dochart.js' });
    vm.runInThisContext(treadmillSource, { filename: 'treadmill_summary.js' });

    window.process_arr(fixture);
    FakeChart.completeAnimations();
    while (pendingTimers.length > 0) {
        const callback = pendingTimers.shift();
        if (typeof callback === 'function') callback();
    }
    const chart = FakeChart.getChart(canvasFor('canvasSpeedInclination'));
    check('Speed/Inclination chart exists', Boolean(chart));

    const moveIndex = events.indexOf('move-speed-container:#watt_badge');
    const createIndex = events.indexOf('create-speed-chart');
    check('Treadmill canvas reaches final parent before Chart.js creation',
        moveIndex >= 0 && createIndex >= 0 && moveIndex < createIndex,
        JSON.stringify(events));

    if (chart) {
        const speed = chart.data.datasets[0].data;
        const incline = chart.data.datasets[1].data;
        check('Elapsed x values stay in seconds',
            JSON.stringify(speed.map(p => p.x)) === JSON.stringify([0, 10, 20]),
            JSON.stringify(speed.map(p => p.x)));
        check('X axis max matches a 20 second workout', chart.options.scales.x.max === 20,
            `max=${chart.options.scales.x.max}`);
        check('Reported/derived treadmill speed is non-zero',
            speed.length === 3 && speed.every(p => Number.isFinite(p.y) && p.y > 0),
            JSON.stringify(speed));
        check('Missing speed is derived from distance/time',
            Math.abs(speed[1].y - 10.00008) < 0.05,
            `derived=${speed[1].y}`);
        check('Inclination data reaches the chart',
            JSON.stringify(incline.map(p => p.y)) === JSON.stringify([2, 4, 6]),
            JSON.stringify(incline));
        check('Chart is resized after final layout', chart.resizeCalls > 0, `resizeCalls=${chart.resizeCalls}`);
        check('Chart is explicitly updated after treadmill patch', chart.updateCalls.includes('none'),
            JSON.stringify(chart.updateCalls));

        const tickCallback = chart.options.scales.x.ticks && chart.options.scales.x.ticks.callback;
        const tick10 = typeof tickCallback === 'function' ? tickCallback(10, 0, []) : null;
        const tick20 = typeof tickCallback === 'function' ? tickCallback(20, 1, []) : null;
        check('Sub-minute x-axis ticks show seconds', tick10 === '00:10' && tick20 === '00:20',
            `10s=${JSON.stringify(tick10)}, 20s=${JSON.stringify(tick20)}`);
    }

    const diagnostics = window.qzTreadmillSummaryDiagnostics;
    check('Runtime diagnostics validate chart data', diagnostics && diagnostics.valid === true,
        diagnostics ? JSON.stringify(diagnostics) : 'missing diagnostics');
    check('Runtime diagnostics retain 20 second duration', diagnostics && diagnostics.durationSeconds === 20,
        diagnostics ? `duration=${diagnostics.durationSeconds}` : 'missing diagnostics');
    check('Treadmill presentation does not mutate session samples', JSON.stringify(fixture) === fixtureBefore);
    check('Treadmill export skips stale legacy captures',
        !savedChartNames.includes('speedInclination') &&
        savedChartNames.filter(name => name === 'power').length === 1 &&
        savedChartNames.filter(name => name === 'power_badge').length === 1,
        JSON.stringify(savedChartNames));

    check('Average GAP uses the Minetti model per segment', renderedText.get('.summary_gap_avg') === '4:23 /km',
        'avgGAP=' + renderedText.get('.summary_gap_avg'));
    check('Average GAP label is present', renderedText.get('.treadmill_gap_label') === 'Avg GAP');

    window.miles = 0.621371;
    window.process_arr(fixture);
    check('Average GAP converts to min/mile', renderedText.get('.summary_gap_avg') === '7:04 /mi',
        'avgGAP=' + renderedText.get('.summary_gap_avg'));

    window.miles = 1;
    const flatFixture = fixture.map(s => Object.assign({}, s, { inclination: 0 }));
    window.process_arr(flatFixture);
    check('Flat average GAP matches average pace',
        renderedText.get('.summary_gap_avg') === renderedText.get('.summary_watts_avg'),
        'gap=' + renderedText.get('.summary_gap_avg') + ' pace=' + renderedText.get('.summary_watts_avg'));

    const stationaryFixture = fixture.map(s => Object.assign({}, s, { distance: 0 }));
    window.process_arr(stationaryFixture);
    check('Zero distance returns unavailable GAP', renderedText.get('.summary_gap_avg') === '--:-- /km',
        'avgGAP=' + renderedText.get('.summary_gap_avg'));

    const logFixture = [
        sample({ elapsed_s: 0, speed: 1, inclination: 0, distance: 0 }),
        sample({ elapsed_m: 5, speed: 6, inclination: 15, distance: 0.402 }),
        sample({ elapsed_m: 10, speed: 5.1, inclination: 25, distance: 0.706 }),
        sample({ elapsed_m: 20, speed: 5.1, inclination: 25, distance: 1.461 }),
        sample({ elapsed_m: 40, speed: 5.1, inclination: 25, distance: 2.914 }),
        sample({ elapsed_m: 45, elapsed_s: 50, speed: 0, inclination: 0, distance: 3.305 })
    ];
    window.process_arr(logFixture);
    const logChart = FakeChart.getChart(canvasFor('canvasSpeedInclination'));
    const logInclination = logChart.data.datasets[1].data.map(point => point.y);
    check('Log inclination values reach the exported chart unchanged',
        logInclination.includes(15) && logInclination.includes(25) && logInclination[logInclination.length - 1] === 0,
        JSON.stringify(logInclination));
} catch (err) {
    fatalError = err && err.stack ? err.stack : String(err);
    console.error(fatalError);
}

const report = {
    generatedAt: new Date().toISOString(),
    fixtureDurationSeconds: 20,
    events,
    diagnostics: window.qzTreadmillSummaryDiagnostics || null,
    results,
    fatalError,
    passed: fatalError === null && results.length > 0 && results.every(r => r.ok)
};
fs.writeFileSync(path.join(artifactDir, 'treadmill-summary-report.json'), JSON.stringify(report, null, 2));
fs.writeFileSync(path.join(artifactDir, 'treadmill-summary-report.html'), `<!doctype html><meta charset="utf-8"><title>QZ treadmill summary test</title><h1>QZ treadmill summary test</h1><p>Passed: <strong>${report.passed}</strong></p><pre>${JSON.stringify(report, null, 2).replace(/&/g,'&amp;').replace(/</g,'&lt;')}</pre>`);

if (!report.passed) process.exit(1);
