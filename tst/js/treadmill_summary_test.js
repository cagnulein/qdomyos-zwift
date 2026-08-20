const fs = require('fs');
const path = require('path');
const vm = require('vm');

const repoRoot = path.resolve(__dirname, '..', '..');
const chartDir = path.join(repoRoot, 'src', 'inner_templates', 'chartjs');
const artifactDir = path.join(__dirname, 'artifacts');
fs.mkdirSync(artifactDir, { recursive: true });

const results = [];
const events = [];
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
    text() { return this; }
    attr() { return this; }
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
global.setTimeout = function() { return 1; };
global.clearTimeout = function() {};
global.html2canvas = undefined;

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
    update(mode) { this.updateCalls.push(mode); }
    resize() { this.resizeCalls++; }
    toBase64Image() { return this.canvas.toDataURL(); }
}
global.Chart = FakeChart;

global.MainWSQueueElement = class {
    enqueue() { return Promise.resolve(null); }
};

const doChartSource = fs.readFileSync(path.join(chartDir, 'dochart.js'), 'utf8');
const treadmillSource = fs.readFileSync(path.join(chartDir, 'treadmill_summary.js'), 'utf8');
vm.runInThisContext(doChartSource, { filename: 'dochart.js' });
vm.runInThisContext(treadmillSource, { filename: 'treadmill_summary.js' });

const fixture = [
    { elapsed_h: 0, elapsed_m: 0, elapsed_s: 0,  deviceType: 1, speed: 10, inclination: 2, distance: 0,        heart_avg: 140, calories: 0 },
    { elapsed_h: 0, elapsed_m: 0, elapsed_s: 10, deviceType: 1, speed: 0,  inclination: 4, distance: 0.027778, heart_avg: 141, calories: 2 },
    { elapsed_h: 0, elapsed_m: 0, elapsed_s: 20, deviceType: 1, speed: 12, inclination: 6, distance: 0.061111, heart_avg: 142, calories: 4 }
];
const fixtureBefore = JSON.stringify(fixture);

let fatalError = null;
try {
    window.process_arr(fixture);
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
