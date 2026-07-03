window.chartColors = {
    red: 'rgb(255, 29, 0)',
    redt: 'rgb(255, 29, 0, 0.55)',
    orange: 'rgb(255, 159, 64)',
    oranget: 'rgb(255, 159, 64, 0.55)',
    darkorange: 'rgb(255, 140, 0)',
    darkoranget: 'rgb(255, 140, 0, 0.55)',
    orangered: 'rgb(255, 69, 0)',
    orangeredt: 'rgb(255, 69, 0, 0.55)',
    yellow: 'rgb(255, 205, 86)',
    yellowt: 'rgb(255, 205, 86, 0.55)',
    green: 'rgb(75, 192, 192)',
    greent: 'rgb(75, 192, 192, 0.55)',
    limegreen: 'rgb(50, 205, 50)',
    limegreent: 'rgb(50, 205, 50, 0.55)',
    gold: 'rgb(255, 215, 0)',
    goldt: 'rgb(255, 215, 0, 0.55)',
    grey: 'rgb(201, 203, 207)',
    greyt: 'rgb(201, 203, 207, 0.55)',
    black: 'rgb(0, 0, 0)',
    blackt: 'rgb(0, 0, 0, 0.55)',
};

var treadmillChart = null;
var speed_max = 0;
var incline_max = 0;
var miles = 1; // 1 = km, 0.621371 = miles

// Whole-workout target curve, pre-rendered from the training program (mirrors dochartlive.js'
// reqpower/rebuildTrainingProgramTarget) so the user sees the entire planned speed/inclination
// profile up front, with the actual live values then progressing across that fixed timeline --
// exactly like the bike power chart, instead of a target line that only ever grows one point at
// a time and never shows what's coming next.
var trainingProgramRows = [];
var trainingProgramDurations = [];
var firstElapsedTargetSpeed = 0;

function t(key, fallback) {
    return window.qzTranslate ? window.qzTranslate(key, fallback) : fallback;
}

function isTrueSetting(value) {
    return value === true || value === 'true' || value === 1 || value === '1';
}

// Incline has no equivalent zone concept, so it just uses the same flat color as the Workout Editor
// preview (workout-editor-app.js) for visual consistency with what the user sees when building the
// training program.
const inclineColor = '#26c6da';

// Speed zones derived from the user's own race pace settings (Settings -> Training Program Options ->
// 1 Mile/5K/10K/Half Marathon/Marathon pace), the same reference paces QZ uses to turn a Zwift running
// workout's %pace target into an actual km/h (see zwiftworkout::speedFromPace). This mirrors how the
// bike power chart derives its zones from FTP, but for running there's no single "reference value" like
// FTP -- each race distance gives its own reference pace -- so we use all five, sorted from slowest
// (marathon) to fastest (1 mile), as the zone boundaries.
var paceZoneLabels = ['Marathon', 'Half Marathon', '10K', '5K', '1 Mile'];
var paceZones = [8, 10, 11, 12, 14]; // km/h fallback if settings can't be read; overwritten in dochart_init()
var paceZoneColors = ['grey', 'limegreen', 'gold', 'orange', 'darkorange', 'red'];
var paceZoneColorsT = ['greyt', 'limegreent', 'goldt', 'oranget', 'darkoranget', 'redt'];

// Background bands for each pace zone, from 0 up to the fastest zone's top. Built as a function
// (rather than computed once) so it always reflects the current paceZones/speed_max at chart
// creation time, after the settings fetch in dochart_init() has resolved.
function paceZoneBoxes() {
    const boundaries = [0, ...paceZones, Math.max(speed_max, paceZones[paceZones.length - 1] + 1)];
    const boxes = {};
    for (let i = 0; i < paceZoneColorsT.length; i++) {
        boxes['box' + i] = {
            type: 'box',
            xMin: 0,
            yMin: boundaries[i],
            yMax: boundaries[i + 1],
            backgroundColor: window.chartColors[paceZoneColorsT[i]],
            yScaleID: 'y-speed',
        };
    }
    return boxes;
}

// How many seconds a training program row should occupy on the timeline. Open-ended rows (e.g.
// "run until lap button" / HR-triggered rows with no fixed duration) don't have a real duration
// up front, so fall back to the preview's visual duration and let syncTrainingProgramTimeline()
// stretch it live once we know how long the user actually spent in it.
function trainingProgramBaseDuration(row) {
    const duration = Number(row.duration_s);
    const visualDuration = Number(row.visual_duration_s);
    if (isTrueSetting(row.openEnded)) {
        const fallbackDuration = Number.isFinite(visualDuration) ? visualDuration :
                                 Number.isFinite(duration) ? duration : 1;
        return Math.max(1, fallbackDuration);
    }
    const fallbackDuration = Number.isFinite(duration) ? duration :
                             Number.isFinite(visualDuration) ? visualDuration : 0;
    return Math.max(0, fallbackDuration);
}

function latestWorkoutElapsed(datasetIndex) {
    if (!treadmillChart || !treadmillChart.data.datasets[datasetIndex] || !treadmillChart.data.datasets[datasetIndex].data.length) {
        return 0;
    }
    const data = treadmillChart.data.datasets[datasetIndex].data;
    return data[data.length - 1].x;
}

// (Re)builds the target speed/inclination datasets for the *entire* training program, one point
// per second across every row. Called once the training program loads, and again whenever an
// open-ended row's live duration needs correcting (syncTrainingProgramTimeline).
function rebuildTrainingProgramTarget(updateChart) {
    if (!treadmillChart) {
        return;
    }

    let elapsed = 0;
    let reqspeed = [];
    let reqinclination = [];

    for (let rowIndex = 0; rowIndex < trainingProgramRows.length; rowIndex++) {
        const el = trainingProgramRows[rowIndex];
        const duration = Math.max(0, Math.round(Number(trainingProgramDurations[rowIndex]) || 0));
        const openEnded = isTrueSetting(el.openEnded);
        const hasSpeed = Number(el.speed) !== -1 && el.speed !== undefined;
        const hasInclination = el.inclination !== undefined;

        if (hasSpeed && speed_max < Number(el.speed) * miles) speed_max = Number(el.speed) * miles;
        if (hasInclination && incline_max < Number(el.inclination)) incline_max = Number(el.inclination);

        for (let i = 0; i < duration; i++) {
            reqspeed.push({x: elapsed, y: hasSpeed ? Number(el.speed) * miles : null, openEnded: openEnded});
            reqinclination.push({x: elapsed, y: hasInclination ? Number(el.inclination) : null, openEnded: openEnded});
            elapsed++;
        }
    }

    treadmillChart.data.datasets[0].data = reqspeed;
    treadmillChart.data.datasets[1].data = reqinclination;

    const finalElapsed = Math.max(elapsed, latestWorkoutElapsed(0), latestWorkoutElapsed(2));
    treadmillChart.options.scales.x.max = finalElapsed;

    if (paceZones.length > 0 && speed_max < paceZones[paceZones.length - 1]) {
        // Always show all pace zone bands/labels on the axis, even if the workout
        // never actually reaches the fastest configured pace.
        speed_max = paceZones[paceZones.length - 1];
    }
    treadmillChart.options.scales['y-speed'].max = Math.ceil(speed_max * 1.1);
    treadmillChart.options.scales['y-incline'].max = Math.ceil(Math.max(incline_max, 1) * 1.1);
    treadmillChart.options.plugins.annotation.annotations = paceZoneBoxes();

    if (updateChart) {
        treadmillChart.update('none');
    }
}

function process_trainprogram(arr) {
    trainingProgramRows = arr.list || [];
    trainingProgramDurations = trainingProgramRows.map(trainingProgramBaseDuration);
    rebuildTrainingProgramTarget(true);
}

// Open-ended rows (HR-triggered / "wait for lap") don't have a real duration in the plan, so the
// pre-rendered timeline above only has a guessed placeholder length for them. Once the user is
// actually inside (or has passed) an open-ended row, homeform tells us exactly how long it took
// (training_row_index/training_row_elapsed) -- use that to stretch the row to its real length so
// later rows in the plan line back up with where the live line actually is.
function syncTrainingProgramTimeline(arr, workoutElapsed) {
    if (!trainingProgramRows.length || !treadmillChart) {
        return;
    }

    const currentRowIndex = Number(arr.training_row_index);
    if (!Number.isFinite(currentRowIndex) || currentRowIndex < 0 || currentRowIndex >= trainingProgramRows.length) {
        return;
    }

    const currentRowElapsed = Math.max(0, Number(arr.training_row_elapsed) || 0);
    const currentRow = trainingProgramRows[currentRowIndex];
    let changed = false;

    const elapsedBeforeCurrent = Math.max(0, workoutElapsed - currentRowElapsed);
    let timelineBeforeCurrent = 0;
    let lastCompletedOpenEndedIndex = -1;
    for (let i = 0; i < currentRowIndex; i++) {
        timelineBeforeCurrent += Number(trainingProgramDurations[i]) || 0;
        if (isTrueSetting(trainingProgramRows[i].openEnded)) {
            lastCompletedOpenEndedIndex = i;
        }
    }

    if (lastCompletedOpenEndedIndex >= 0) {
        const delta = elapsedBeforeCurrent - timelineBeforeCurrent;
        if (Math.abs(delta) >= 1) {
            trainingProgramDurations[lastCompletedOpenEndedIndex] =
                    Math.max(1, (Number(trainingProgramDurations[lastCompletedOpenEndedIndex]) || 1) + delta);
            changed = true;
        }
    }

    if (isTrueSetting(currentRow.openEnded)) {
        const liveDuration = Math.max(1, currentRowElapsed);
        if (Math.abs((Number(trainingProgramDurations[currentRowIndex]) || 0) - liveDuration) >= 1) {
            trainingProgramDurations[currentRowIndex] = liveDuration;
            changed = true;
        }
    }

    if (changed) {
        rebuildTrainingProgramTarget(false);
    }
}

function process_arr(arr) {
    let ctx = document.getElementById('canvas').getContext('2d');
    let div = document.getElementById('divcanvas');

    let speed = [];
    let inclination = [];

    // The target (dashed) datasets are populated separately once the training program loads
    // (process_trainprogram/rebuildTrainingProgramTarget), covering the whole planned workout up
    // front. Here we only rebuild the actual (solid) history, aligned to the same workoutElapsed
    // baseline: the first moment the training program actually gave a speed target, mirroring how
    // dochartlive.js finds firstElapsedTargetPower for the bike power chart.
    for (let el of arr) {
        const time = el.elapsed_s + el.elapsed_m * 60 + el.elapsed_h * 3600;

        if (el.target_speed !== undefined && el.target_speed !== -1) {
            if (firstElapsedTargetSpeed === 0) {
                firstElapsedTargetSpeed = time;
            }
        }

        if (firstElapsedTargetSpeed === 0) {
            continue;
        }

        const workoutElapsed = time - firstElapsedTargetSpeed;

        if (el.speed !== undefined) {
            speed.push({x: workoutElapsed, y: el.speed * miles});
            if (speed_max < el.speed * miles) speed_max = el.speed * miles;
        }

        if (el.inclination !== undefined) {
            inclination.push({x: workoutElapsed, y: el.inclination});
            if (incline_max < el.inclination) incline_max = el.inclination;
        }
    }

    if (paceZones.length > 0 && speed_max < paceZones[paceZones.length - 1]) {
        // Always show all pace zone bands/labels on the axis, even if the workout
        // never actually reached the fastest configured pace.
        speed_max = paceZones[paceZones.length - 1];
    }
    speed_max = Math.ceil(speed_max * 1.1);
    incline_max = Math.ceil(incline_max * 1.1);

    const backgroundFill = {
        id: 'custom_canvas_background_color',
        beforeDraw: (chart) => {
            const ctx = chart.canvas.getContext('2d');
            ctx.save();
            ctx.globalCompositeOperation = 'destination-over';
            ctx.fillStyle = 'white';
            ctx.fillRect(0, 0, chart.width, chart.height);
            ctx.restore();
        }
    };

    let config = {
        type: 'line',
        plugins: [backgroundFill],
        data: {
            // Target datasets are listed first and actual datasets last: Chart.js paints datasets
            // in array order, so the actual (solid, zone-colored) lines are always drawn on top of
            // the dashed target lines instead of being hidden behind them when the two coincide
            // (e.g. with a Fake Treadmill, which has no acceleration lag and tracks target exactly).
            // Target data itself is filled in later by rebuildTrainingProgramTarget() once the
            // training program has loaded.
            datasets: [{
                label: miles === 1 ? t('chart.targetSpeedKmh', 'Target Speed (km/h)') : t('chart.targetSpeedMph', 'Target Speed (mph)'),
                backgroundColor: window.chartColors.black,
                borderColor: window.chartColors.black,
                data: [],
                fill: false,
                pointRadius: 0,
                borderWidth: 2,
                yAxisID: 'y-speed',
                borderDash: [5, 5]
            }, {
                label: t('chart.targetIncline', 'Target Incline'),
                backgroundColor: window.chartColors.grey,
                borderColor: window.chartColors.grey,
                data: [],
                fill: false,
                pointRadius: 0,
                borderWidth: 2,
                yAxisID: 'y-incline',
                borderDash: [5, 5]
            }, {
                label: miles === 1 ? t('workoutEditor.speedKmh', 'Speed (km/h)') : t('workoutEditor.speedMph', 'Speed (mph)'),
                backgroundColor: window.chartColors.red,
                borderColor: window.chartColors.red,
                data: speed,
                fill: false,
                pointRadius: 0,
                borderWidth: 2,
                yAxisID: 'y-speed',
                segment: {
                    borderColor: ctx => {
                        const y = ctx.p0.parsed.y;
                        for (let i = 0; i < paceZones.length; i++) {
                            if (y < paceZones[i]) return window.chartColors[paceZoneColors[i]];
                        }
                        return window.chartColors[paceZoneColors[paceZoneColors.length - 1]];
                    }
                }
            }, {
                label: t('workoutEditor.incline', 'Incline'),
                backgroundColor: inclineColor,
                borderColor: inclineColor,
                data: inclination,
                fill: false,
                pointRadius: 0,
                borderWidth: 2,
                yAxisID: 'y-incline'
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            interaction: {
                mode: 'index',
                intersect: false,
            },
            plugins: {
                tooltip: {
                    enabled: true,
                },
                legend: {
                    display: false
                },
                annotation: {
                    annotations: paceZoneBoxes()
                }
            },
            scales: {
                x: {
                    type: 'linear',
                    display: true,
                    title: {
                        display: false
                    },
                    ticks: {
                        callback: function(value) {
                            return value !== 0 ?
                                Math.floor(value / 3600).toString().padStart(2, "0") + ":" +
                                Math.floor((value / 60) - (Math.floor(value / 3600) * 60)).toString().padStart(2, "0") :
                                "";
                        },
                        padding: -20,
                        align: "end",
                    },
                },
                'y-speed': {
                    type: 'linear',
                    display: true,
                    position: 'left',
                    title: {
                        display: true,
                        text: miles === 1 ? t('workoutEditor.speedKmh', 'Speed (km/h)') : t('workoutEditor.speedMph', 'Speed (mph)'),
                        color: 'black',
                    },
                    min: 0,
                    max: speed_max,
                    ticks: {
                        stepSize: 1,
                        autoSkip: false,
                        color: 'black',
                        callback: value => {
                            const i = paceZones.findIndex(z => Math.abs(z - value) < 0.5);
                            return i === -1 ? Math.round(value) : `${Math.round(value)} (${paceZoneLabels[i]})`;
                        }
                    }
                },
                'y-incline': {
                    type: 'linear',
                    display: true,
                    position: 'right',
                    title: {
                        display: true,
                        text: t('workoutEditor.inclinePercent', 'Incline (%)'),
                        color: inclineColor,
                    },
                    min: 0,
                    max: incline_max,
                    grid: {
                        drawOnChartArea: false,
                    }
                }
            }
        }
    };

    treadmillChart = new Chart(ctx, config);

    refresh();
}

function refresh() {
    el = new MainWSQueueElement({
        msg: null
    }, function(msg) {
        if (msg.msg === 'workout') {
            return msg.content;
        }
        return null;
    }, 2000, 1);
    el.enqueue().then(process_workout).catch(function(err) {
        console.error('Error is ' + err);
        refresh();
    });
}

function process_workout(arr) {
    // Dataset order: 0=Target Speed, 1=Target Incline, 2=Speed (actual), 3=Incline (actual).
    // The target datasets are owned by rebuildTrainingProgramTarget()/syncTrainingProgramTimeline()
    // (whole plan, pre-rendered); here we only append the actual live values, aligned to the same
    // workoutElapsed baseline the plan uses.
    const elapsed = arr.elapsed_s + (arr.elapsed_m * 60) + (arr.elapsed_h * 3600);

    if (arr.target_speed !== undefined && arr.target_speed !== -1 && firstElapsedTargetSpeed === 0) {
        firstElapsedTargetSpeed = elapsed;
        treadmillChart.data.datasets[2].data = [];
        treadmillChart.data.datasets[3].data = [];
    }

    if (firstElapsedTargetSpeed === 0) {
        refresh();
        return;
    }

    const workoutElapsed = elapsed - firstElapsedTargetSpeed;

    syncTrainingProgramTimeline(arr, workoutElapsed);

    // Update speed data (actual)
    treadmillChart.data.datasets[2].data.push({x: workoutElapsed, y: arr.speed * miles});
    if (speed_max < arr.speed * miles) {
        speed_max = Math.ceil(arr.speed * miles * 1.1);
        treadmillChart.options.scales['y-speed'].max = speed_max;
    }

    // Update inclination data (actual)
    treadmillChart.data.datasets[3].data.push({x: workoutElapsed, y: arr.inclination});
    if (incline_max < arr.inclination) {
        incline_max = Math.ceil(arr.inclination * 1.1);
        treadmillChart.options.scales['y-incline'].max = incline_max;
    }

    if (workoutElapsed > treadmillChart.options.scales.x.max) {
        treadmillChart.options.scales.x.max = workoutElapsed;
    }

    // 'none' skips the animated transition since a new point arrives every tick anyway.
    treadmillChart.update('none');
    refresh();
}

function loadSessionArrayAndBuildChart() {
    let el = new MainWSQueueElement({
        msg: 'getsessionarray'
    }, function(msg) {
        if (msg.msg === 'R_getsessionarray') {
            return msg.content;
        }
        return null;
    }, 15000, 3);
    el.enqueue().then(function(arr) {
        process_arr(arr);
        loadTrainingProgram();
    }).catch(function(err) {
        console.error('Error is ' + err);
    });
}

function loadTrainingProgram() {
    let el = new MainWSQueueElement({
        msg: 'gettrainingprogram'
    }, function(msg) {
        if (msg.msg === 'R_gettrainingprogram') {
            return msg.content;
        }
        return null;
    }, 15000, 3);
    el.enqueue().then(process_trainprogram).catch(function(err) {
        console.error('Error is ' + err);
    });
}

function dochart_init() {
    // Fetch miles_unit and the user's race pace settings first, since paceZones/miles need to be
    // resolved before the chart (and its zone backgrounds/labels) is built.
    const keys_arr = ['miles_unit', 'pacef_1mile', 'pacef_5km', 'pacef_10km', 'pacef_halfmarathon', 'pacef_marathon'];
    let el = new MainWSQueueElement({
        msg: 'getsettings',
        content: {
            keys: keys_arr
        }
    }, function(msg) {
        if (msg.msg === 'R_getsettings') {
            for (const key of keys_arr) {
                if (msg.content[key] === undefined) return null;
            }
            return msg.content;
        }
        return null;
    }, 5000, 3);
    el.enqueue().then(function(settings) {
        if (settings['miles_unit'] === true || settings['miles_unit'] === 'true') {
            miles = 0.621371;
        }
        const paceToSpeed = pacef => (3600 / Number(pacef)) * miles;
        paceZones = [
            paceToSpeed(settings['pacef_marathon']),
            paceToSpeed(settings['pacef_halfmarathon']),
            paceToSpeed(settings['pacef_10km']),
            paceToSpeed(settings['pacef_5km']),
            paceToSpeed(settings['pacef_1mile']),
        ].sort((a, b) => a - b);
    }).catch(function(err) {
        console.error('Error getting settings: ' + err);
    }).finally(loadSessionArrayAndBuildChart);
}

$(window).on('load', function() {
    dochart_init();
});

$(document).ready(function () {
    $('#loading').hide();
});
