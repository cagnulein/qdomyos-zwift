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
    blue: 'rgb(54, 162, 235)',
    purple: 'rgb(153, 102, 255)',
    grey: 'rgb(201, 203, 207)',
    greyt: 'rgb(201, 203, 207, 0.55)',
    white: 'rgb(255, 255, 255)',
    whitet: 'rgb(255, 255, 255, 0.55)',
    limegreen: 'rgb(50, 205, 50)',
    limegreent: 'rgb(50, 205, 50, 0.55)',
    gold: 'rgb(255, 215, 0)',
    goldt: 'rgb(255, 215, 0, 0.55)',
    black: 'rgb(0, 0, 0)',
    blackt: 'rgb(0, 0, 0, 0.55)',
    lightsteelblue: 'rgb(176,192,222)',
    lightsteelbluet: 'rgb(176,192,222, 0.55)',
};

var ftp = 200;
var ftpZones = [];
var maxHeartRate = 190;
var heartZones = [];
var miles = 1;

// live update during a workout: the new points of the session are asked for once a second and
// the charts are updated in place, only while a workout is running (see setLiveActive)
var LIVE_REFRESH_MS = 1000;
var liveTimer = null;
var liveLoading = false;
var liveActive = false;
var liveStateKnown = false; // a 'workout' push has come
var liveFinalPending = false;
var lastSessionLength = -1;
var sessionCache = null; // the session so far, grown by the parts the live update asks for
var liveRefresh = false;
var pendingLive = {}; // canvas id -> the config of a live refresh skipped while off screen
var pendingLiveTimer = null;

// the polling goes on: a chart off screen waits for the next refresh or for a scroll to it
function livePolling() {
    return liveActive;
}

function offScreen(canvas) {
    var rect = canvas.getBoundingClientRect();
    return rect.bottom < 0 || rect.top > window.innerHeight;
}

function isPointRow(row) {
    return Array.isArray(row) && row.length > 0 && typeof row[0] === 'object' && row[0] !== null;
}

// the lines keep the lowest and the highest point of each slice of the time axis, one slice for
// every 2 px of the screen: a long workout draws as fast as a short one, and the peaks stay.
// The lines of a chart keep the same points (the union of their lowest and highest ones), so
// the tooltip, which takes the point of one index from every line, shows them at one time.
// Averages, maximums and zones are counted on the whole session before this
function thinLines(datasets, slices) {
    let rows = [], length = 0;
    datasets.forEach(function (d) {
        if (isPointRow(d.data))
            length = Math.max(length, d.data.length);
    });
    datasets.forEach(function (d) {
        if (isPointRow(d.data) && d.data.length === length)
            rows.push(d.data);
    });
    if (length <= slices * 2)
        return;
    let keep = [], size = length / slices;
    for (let b = 0; b < slices; b++) {
        let from = Math.floor(b * size), to = Math.min(length, Math.floor((b + 1) * size));
        let picked = [];
        rows.forEach(function (row) {
            let low = -1, high = -1, gap = -1;
            for (let i = from; i < to; i++) {
                let y = row[i].y;
                if (y === null || y === undefined || y !== y) { // y !== y: NaN
                    if (gap < 0)
                        gap = i; // a gap in the slice (a point without a value) stays a break
                    continue;
                }
                if (low < 0 || y < row[low].y)
                    low = i;
                if (high < 0 || y > row[high].y)
                    high = i;
            }
            [low, high, gap].forEach(function (i) {
                if (i >= 0 && picked.indexOf(i) < 0)
                    picked.push(i);
            });
        });
        if (picked.length === 0)
            picked.push(from);
        picked.sort(function (a, c) { return a - c; });
        keep = keep.concat(picked);
    }
    datasets.forEach(function (d) {
        if (rows.indexOf(d.data) >= 0)
            d.data = keep.map(function (i) { return d.data[i]; });
    });
}

function updateInPlace(old, config) {
    config.options.animation = false;
    if (lastTimeRepeats(config)) {
        var ctx = old.ctx;
        old.destroy();
        return qzChartTheme.create(ctx, config);
    }
    // keep the dataset objects: the chart's per-dataset state is bound to them
    for (var i = 0; i < config.data.datasets.length; i++)
        old.data.datasets[i].data = config.data.datasets[i].data;
    if (config.data.labels)
        old.data.labels = config.data.labels;
    qzChartTheme.restyle(old, config); // the new options in the app theme, like create()
    old.options = config.options;
    old.update('none');
    return old;
}

// with the animation off Chart.js 3.4.1 updates only the points up to the first one at the x
// axis max; when the last points share one time (a session that stands still) the points after
// it keep no values, and drawing the line throws. Such a chart is made anew
function lastTimeRepeats(config) {
    return config.data.datasets.some(function (d) {
        var n = d.data.length;
        return n > 1 && d.data[n - 1] !== null && typeof d.data[n - 1] === 'object' && d.data[n - 2] !== null
            && typeof d.data[n - 2] === 'object' && d.data[n - 1].x === d.data[n - 2].x;
    });
}

// creates a chart on ctx; on a live refresh the existing chart gets the new data and options
// in place instead, so an open tooltip stays and nothing flickers; the animation is off there,
// so its onComplete (saving screenshots) does not run again. While the polling goes on, a chart
// off screen is not drawn: drawing is most of the cost of a refresh, and only one or two of the
// charts are on screen
function makeChart(ctx, config) {
    // by the longer side of the screen, not by the chart: the page turned to landscape keeps
    // the same points
    thinLines(config.data.datasets, Math.max(100, Math.round(Math.max(window.innerWidth, window.innerHeight) / 2)));
    var old = Chart.getChart(ctx.canvas);
    delete pendingLive[ctx.canvas.id];
    if (old && liveRefresh && old.data.datasets.length === config.data.datasets.length) {
        if (livePolling() && offScreen(ctx.canvas)) {
            pendingLive[ctx.canvas.id] = config;
            return old;
        }
        return updateInPlace(old, config);
    }
    if (old)
        old.destroy();
    if (liveRefresh)
        config.options.animation = false;
    return qzChartTheme.create(ctx, config);
}

// draws the skipped charts that came on screen, or all of them when the polling stops
function drawPendingLive() {
    for (var id in pendingLive) {
        var canvas = document.getElementById(id);
        var old = canvas && Chart.getChart(canvas);
        if (old && livePolling() && offScreen(canvas))
            continue;
        if (old)
            updateInPlace(old, pendingLive[id]);
        delete pendingLive[id];
    }
}

function schedulePendingLive() {
    if (pendingLiveTimer === null)
        pendingLiveTimer = setTimeout(function () {
            pendingLiveTimer = null;
            drawPendingLive();
        }, 100);
}
window.addEventListener('scroll', schedulePendingLive);
window.addEventListener('resize', schedulePendingLive);

function t(key, fallback) {
    return window.qzTranslate ? window.qzTranslate(key, fallback) : fallback;
}

function zoneLabel(number) {
    return t('chart.zoneNumber', 'zone {number}').replace('{number}', number);
}

function isTrueSetting(value) {
    return value === true || value === 'true' || value === 1 || value === '1';
}

function ensurePowerZones() {
    ftp = Number(ftp);
    if (!Number.isFinite(ftp) || ftp <= 0) {
        ftp = 200;
    }

    const zoneRatios = [0.55, 0.75, 0.90, 1.05, 1.20, 1.50];
    for (let i = 0; i < zoneRatios.length; i++) {
        ftpZones[i] = Number(ftpZones[i]);
        if (!Number.isFinite(ftpZones[i]) || ftpZones[i] <= 0) {
            ftpZones[i] = Math.round(ftp * zoneRatios[i]);
        }
    }
}

function ensureHeartZones() {
    maxHeartRate = Number(maxHeartRate);
    if (!Number.isFinite(maxHeartRate) || maxHeartRate <= 0) {
        maxHeartRate = 190;
    }

    const defaultZoneRatios = [0.70, 0.80, 0.90, 1.00];
    for (let i = 0; i < defaultZoneRatios.length; i++) {
        heartZones[i] = Number(heartZones[i]);
        if (!Number.isFinite(heartZones[i]) || heartZones[i] <= 0) {
            heartZones[i] = Math.round(maxHeartRate * defaultZoneRatios[i]);
        }
    }
}

function process_arr(arr) {
    let watts = [];
    let reqpower = [];
    let reqcadence = [];
    let heart = [];
    let cadence = [];
    let speed = [];
    let inclination = [];
    let resistance = [];
    let reqresistance = [];
    let pelotonresistance = [];
    let pelotonreqresistance = [];
    let distributionPowerZones = [];
    let maxEl = 0;
    let saveScreenshot = [];
    let workoutName = '';
    let workoutStartDate = '';
    let instructorName = '';
    let watts_avg = 0;
    let watts_max = 0;
    let heart_avg = 0;
    let heart_max = 0;
    let jouls = 0;
    let deviceType = 0;
    let cadence_avg = 0;
    let peloton_resistance_avg = 0;
    let resistance_avg = 0;
    let calories = 0;
    let distance = 0;
    saveScreenshot[0] = false;
    saveScreenshot[1] = false;
    saveScreenshot[2] = false;
    saveScreenshot[3] = false;
    saveScreenshot[4] = false;
    saveScreenshot[5] = false;
    saveScreenshot[6] = false;
    saveScreenshot[7] = false;
    distributionPowerZones[0] = 0;
    distributionPowerZones[1] = 0;
    distributionPowerZones[2] = 0;
    distributionPowerZones[3] = 0;
    distributionPowerZones[4] = 0;
    distributionPowerZones[5] = 0;
    distributionPowerZones[6] = 0;

    for (let el of arr) {
        let wattel = {};
        let reqpowerel = {};
        let reqcadenceel = {};
        let heartel = {};
        let cadenceel = {};
        let resistanceel = {};
        let reqresistanceel = {};
        let pelotonresistanceel = {};
        let pelotonreqresistanceel = {};
        let speedel = {};
        let inclinationel = {};
        let time = el.elapsed_s + el.elapsed_m * 60 + el.elapsed_h * 3600;
        workoutName = el.workoutName;
        workoutStartDate = el.workoutStartDate;
        instructorName = el.instructorName;
        watts_avg = el.watts_avg;
        watts_max = el.watts_max;
        heart_avg = el.heart_avg;
        heart_max = el.heart_max;
        jouls = el.jouls;
        deviceType = el.deviceType;
        peloton_resistance_avg = el.peloton_resistance_avg;
        resistance_avg = el.resistance_avg;
        cadence_avg = el.cadence_avg;
        distance = el.distance;
        calories = el.calories;
        maxEl = time;
        wattel.x = time;
        wattel.y = el.watts;
        watts.push(wattel);
        if(el.watts < ftpZones[0])
            distributionPowerZones[0]++;
        else if(el.watts < ftpZones[1])
            distributionPowerZones[1]++;
        else if(el.watts < ftpZones[2])
            distributionPowerZones[2]++;
        else if(el.watts < ftpZones[3])
            distributionPowerZones[3]++;
        else if(el.watts < ftpZones[4])
            distributionPowerZones[4]++;
        else if(el.watts < ftpZones[5])
            distributionPowerZones[5]++;
        else
            distributionPowerZones[6]++;
        reqpowerel.x = time;
        reqpowerel.y = el.req_power;
        reqpower.push(reqpowerel);
        reqcadenceel.x = time;
        reqcadenceel.y = el.req_cadence;
        reqcadence.push(reqcadenceel);
        heartel.x = time;
        heartel.y = el.heart;
        heart.push(heartel);
        cadenceel.x = time;
        cadenceel.y = el.cadence;
        cadence.push(cadenceel);
        resistanceel.x = time;
        resistanceel.y = el.resistance;
        resistance.push(resistanceel);
        reqresistanceel.x = time;
        reqresistanceel.y = el.req_resistance;
        reqresistance.push(reqresistanceel);

        pelotonresistanceel.x = time;
        pelotonresistanceel.y = el.peloton_resistance;
        pelotonresistance.push(pelotonresistanceel);
        pelotonreqresistanceel.x = time;
        pelotonreqresistanceel.y = el.peloton_req_resistance;
        pelotonreqresistance.push(pelotonreqresistanceel);

        speedel.x = time;
        speedel.y = el.speed * miles; // Convert to user's preferred unit (km/h or mph)
        speed.push(speedel);
        inclinationel.x = time;
        inclinationel.y = el.inclination;
        inclination.push(inclinationel);
    }

    $('.workoutName').text(workoutName);
    $('.workoutStartDate').text(workoutStartDate);
    $('.instructorName').text((instructorName));
    if(instructorName.length === 0) {
        if(deviceType === 1)
            $('.workout_image').attr("src","run.png");
        else if(deviceType === 3) 
            $('.workout_image').attr("src","row.png");
        else if(deviceType === 4) 
            $('.workout_image').attr("src","elliptical.png");
        else
            $('.workout_image').attr("src","bike.png");
        // Modern look: the picture as an icon in the theme colours (the saved badge keeps the PNG)
        qzChartTheme.sportIcon($('.workout_image')[0]);
    }
    $('.workout_image').attr("crossOrigin","anonymous");
    $('.watts_avg').text(t('chart.wattAvgValue', 'Watt AVG: {value}').replace('{value}', Math.floor(watts_avg)));
    $('.watts_max').text(t('chart.wattMaxValue', 'Watt MAX: {value}').replace('{value}', watts_max));
    $('.heart_avg').text(t('chart.heartRateAvgValue', 'Heart Rate AVG: {value}').replace('{value}', Math.floor(heart_avg)));
    $('.heart_max').text(t('chart.heartRateMaxValue', 'Heart Rate MAX: {value}').replace('{value}', heart_max));

    $('.summary_watts_avg').text(Math.floor(watts_avg) + ' ' + t('unit.w', 'W'));
    $('.summary_jouls').text(Math.floor(jouls / 1000.0) + ' ' + t('unit.kj', 'kJ'));
    $('.summary_calories').text(Math.floor(calories) + ' ' + t('unit.kcal', 'kcal'));
    $('.summary_distance').text((distance * miles).toFixed(1) + ' ' + (miles === 1 ? t('unit.km', 'km') : t('unit.mi', 'mi')));
    $('.summary_cadence_avg').text(Math.floor(cadence_avg) + ' ' + t('unit.rpm', 'rpm'));
    // Use resistance_avg for ellipticals (deviceType 4), peloton_resistance_avg for bikes/rowers
    $('.summary_resistance_avg').text(Math.floor(deviceType === 4 ? resistance_avg : peloton_resistance_avg) + ' ' + t('unit.level', 'lvl'));

    const backgroundFill = {
      id: 'custom_canvas_background_color',
      beforeDraw: (chart) => {
        const ctx = chart.canvas.getContext('2d');
        ctx.save();
        ctx.globalCompositeOperation = 'destination-over';
        ctx.fillStyle = qzChartTheme.background(chart);
        ctx.fillRect(0, 0, chart.width, chart.height);
        ctx.restore();
      }
    };

    let config = {
        type: 'line',
        plugins: [backgroundFill],
        data: {
            datasets: [{
                label: t('chart.watts', 'Watts'),
                backgroundColor: window.chartColors.red,
                borderColor: window.chartColors.red,
                cubicInterpolationMode: 'monotone',
                data: watts,
                fill: false,
                pointRadius: 0,
                borderWidth: 2,
                segment: {
                   borderColor: ctx => ctx.p0.parsed.y < ftpZones[0] && ctx.p1.parsed.y < ftpZones[0] ? window.chartColors.grey :
                                                                       ctx.p0.parsed.y < ftpZones[1] && ctx.p1.parsed.y < ftpZones[1] ? window.chartColors.limegreen :
                                                                       ctx.p0.parsed.y < ftpZones[2] && ctx.p1.parsed.y < ftpZones[2] ? window.chartColors.gold :
                                                                       ctx.p0.parsed.y < ftpZones[3] && ctx.p1.parsed.y < ftpZones[3] ? window.chartColors.orange :
                                                                       ctx.p0.parsed.y < ftpZones[4] && ctx.p1.parsed.y < ftpZones[4] ? window.chartColors.darkorange :
                                                                       ctx.p0.parsed.y < ftpZones[5] && ctx.p1.parsed.y < ftpZones[5] ? window.chartColors.orangered :
                                                                       window.chartColors.red,
                }
            }, {
                label: t('chart.requestedWatts', 'Req. Watts'),
                backgroundColor: window.chartColors.black,
                borderColor: window.chartColors.black,
                //cubicInterpolationMode: 'monotone',
                data: reqpower,
                fill: false,
                pointRadius: 0,
                borderWidth: 2,
            },
            ]
        },
        options: {
            animation: {
              onComplete: function() {
                if(saveScreenshot[7] === false) {
                    // before withLight(): its redraw calls this onComplete again
                    saveScreenshot[7] = true;
                    var watt_badge = document.getElementById('watt_badge');

                    // Capture the containers using html2canvas; in the modern look the badge and
                    // the power chart inside it are drawn light for the copy
                    qzChartTheme.withLight(powerChart, function () {
                        return html2canvas(watt_badge, qzChartTheme.snapshotOptions());
                    }).then(function(canvas1) {
            
                        // Convert the merged canvas to a PNG image
                        var image = canvas1.toDataURL('image/png');
                
                        let el = new MainWSQueueElement({
                            msg: 'savechart',
                            content: {
                                name: 'power_badge',
                                image: image
                            }
                        }, function(msg) {
                            if (msg.msg === 'R_savechart') {
                                return msg.content;
                            }
                            return null;
                        }, 15000, 3);
                        el.enqueue().catch(function(err) {
                            console.error('Error is ' + err);
                        });
                    });
                }
                saveScreenshot[7] = true;

                  if(saveScreenshot[0])
                      return;
                  saveScreenshot[0] = true;
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'power',
                          image: qzChartTheme.image(powerChart)
                      }
                  }, function(msg) {
                      if (msg.msg === 'R_savechart') {
                          return msg.content;
                      }
                      return null;
                  }, 15000, 3);
                  el.enqueue().catch(function(err) {
                      console.error('Error is ' + err);
                  });
              }
            },
            responsive: true,
            aspectRatio: 1.5,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                title:{
                    display:true,
                    backgroundColor: "#1d2330",
                    padding: {
                        top: 2,
                        bottom: 2
                    },
                    text:t('metric.watt', 'Watt')
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false
                },
                annotation: {
                        annotations: {
                            box1: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: 0,
                            yMax: ftpZones[0],
                            backgroundColor: "#d6d6d620"
                            },
                            box2: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: ftpZones[0],
                            yMax: ftpZones[1],
                            backgroundColor: window.chartColors.limegreent,
                            },
                            box3: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: ftpZones[1],
                            yMax: ftpZones[2],
                            backgroundColor: window.chartColors.goldt,
                            },
                            box4: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: ftpZones[2],
                            yMax: ftpZones[3],
                            backgroundColor: window.chartColors.oranget,
                            },
                            box5: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: ftpZones[3],
                            yMax: ftpZones[4],
                            backgroundColor: window.chartColors.darkoranget,
                            },
                            box6: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: ftpZones[4],
                            yMax: ftpZones[5],
                            backgroundColor: window.chartColors.orangeredt,
                            },
                            box7: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: ftpZones[5],
                            yMax: (watts_max > ftpZones[3] * 2 ? watts_max + 10 : ftpZones[3] * 2),
                            backgroundColor: window.chartColors.redt,
                            },
                        }
                      }
            },
            hover: {
                mode: 'nearest',
                intersect: true
            },
            scales: {
                x: {
                    type: 'linear',
                    display: true,
                    title: {
                        display: false,
                        text: 'Time'
                    },
                    ticks: {
                        // Include a dollar sign in the ticks
                        callback: function(value, index, values) {
                            return value !== 0 ? Math.floor(value / 3600).toString().padStart(2, "0") + ":" + Math.floor((value / 60) - (Math.floor(value / 3600) * 60)).toString().padStart(2, "0")  : "";
                        },
                        padding: -20,
                        //stepSize: 300,
                        align: "end",
                    },
                    max: maxEl,
                },
                y: {
                    display: true,
                    title: {
                        display: false,
                        text: 'Watt'
                    },
                    min: 0,
                    max: (watts_max > ftpZones[4] + 10 ? watts_max + 10 : ftpZones[4] + 10),
                    ticks: {
                        stepSize: 1,
                        autoSkip: false,
                        callback: value => [ftpZones[0] * 0.8, ftpZones[0], ftpZones[1], ftpZones[2], ftpZones[3], ftpZones[4], ftpZones[5]].includes(value) ?
                            value === ftpZones[0] * 0.8 ? zoneLabel(1) :
                            value === ftpZones[0] ? zoneLabel(2) :
                            value === ftpZones[1] ? zoneLabel(3) :
                            value === ftpZones[2] ? zoneLabel(4) :
                            value === ftpZones[3] ? zoneLabel(5) :
                            value === ftpZones[4] ? zoneLabel(6) :
                            value === ftpZones[5] ? zoneLabel(7) : undefined : undefined,
                        color: 'black',
                        padding: -50,
                        align: 'end',
                        z: 1,
                    }
                }
            }
        }
    };

    let ctx = document.getElementById('canvas').getContext('2d');
    var powerChart = makeChart(ctx, config);

    const minRecordedHeart = heart.reduce(function(minValue, point) {
        return point.y > 0 ? Math.min(minValue, point.y) : minValue;
    }, Number.POSITIVE_INFINITY);
    const heartTrainingFloor = Math.round(maxHeartRate * 0.5);
    const heartChartBottom = Math.max(
        0,
        Math.min(heartTrainingFloor, Number.isFinite(minRecordedHeart) ? minRecordedHeart - 5 : heartTrainingFloor)
    );
    const heartChartTop = Math.max(
        maxHeartRate,
        heart_max > maxHeartRate ? heart_max + 5 : maxHeartRate,
        heartChartBottom + 50
    );
    const heartZoneLabelPositions = [
        Math.round((heartChartBottom + heartZones[0]) / 2),
        Math.round((heartZones[0] + heartZones[1]) / 2),
        Math.round((heartZones[1] + heartZones[2]) / 2),
        Math.round((heartZones[2] + heartZones[3]) / 2),
        Math.round((heartZones[3] + heartChartTop) / 2),
    ];
    config = {
        type: 'line',
        plugins: [backgroundFill],
        data: {
            datasets: [{
                label: t('metric.heart', 'Heart'),
                backgroundColor: window.chartColors.red,
                borderColor: window.chartColors.red,
                //cubicInterpolationMode: 'monotone',
                data: heart,
                fill: false,
                pointRadius: 0,
                borderWidth: 2,
                segment: {
                   borderColor: ctx => ctx.p0.parsed.y < heartZones[0] && ctx.p1.parsed.y < heartZones[0] ? window.chartColors.lightsteelblue :
                                                                       ctx.p0.parsed.y < heartZones[1] && ctx.p1.parsed.y < heartZones[1] ? window.chartColors.green :
                                                                       ctx.p0.parsed.y < heartZones[2] && ctx.p1.parsed.y < heartZones[2] ? window.chartColors.yellow :
                                                                       ctx.p0.parsed.y < heartZones[3] && ctx.p1.parsed.y < heartZones[3] ? window.chartColors.orange :
                                                                       window.chartColors.red,
                }
            }]
        },
        options: {
            animation: {
              onComplete: function() {
                  if(saveScreenshot[1])
                      return;
                  saveScreenshot[1] = true;
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'heart',
                          image: qzChartTheme.image(heartChart)
                      }
                  }, function(msg) {
                      if (msg.msg === 'R_savechart') {
                          return msg.content;
                      }
                      return null;
                  }, 15000, 3);
                  el.enqueue().catch(function(err) {
                      console.error('Error is ' + err);
                  });
              }
            },
            responsive: true,
            aspectRatio: 1.5,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                title:{
                    display:true,
                    text:t('chart.heartRate', 'Heart Rate')
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false
                },
                annotation: {
                        annotations: {
                            box1: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: heartChartBottom,
                            yMax: heartZones[0],
                            backgroundColor: window.chartColors.lightsteelbluet,
                            },
                            box2: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: heartZones[0],
                            yMax: heartZones[1],
                            backgroundColor: window.chartColors.greent,
                            },
                            box3: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: heartZones[1],
                            yMax: heartZones[2],
                            backgroundColor: window.chartColors.yellowt,
                            },
                            box4: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: heartZones[2],
                            yMax: heartZones[3],
                            backgroundColor: window.chartColors.oranget,
                            },
                            box5: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            xMax: maxEl,
                            yMin: heartZones[3],
                            yMax: heartChartTop,
                            backgroundColor: window.chartColors.redt,
                            },
                    }
                },
            },
            hover: {
                mode: 'nearest',
                intersect: true
            },
            scales: {
                x: {
                    type: 'linear',
                    display: true,
                    title: {
                        display: false,
                        text: 'Time'
                    },
                    ticks: {
                        // Include a dollar sign in the ticks
                        callback: function(value, index, values) {
                            return value !== 0 ? Math.floor(value / 3600).toString().padStart(2, "0") + ":" + Math.floor((value / 60) - (Math.floor(value / 3600) * 60)).toString().padStart(2, "0")  : "";
                        },
                        padding: -20,
                        //stepSize: 300,
                        align: "end",
                    },
                    max: maxEl,
                },
                y: {
                    display: true,
                    title: {
                        display: false,
                        text: 'Heart rate'
                    },
                    min: heartChartBottom,
                    max: heartChartTop,
                    ticks: {
                        stepSize: 1,
                        autoSkip: false,
                        callback: value => heartZoneLabelPositions.includes(value) ?
                            value === heartZoneLabelPositions[0] ? zoneLabel(1) :
                            value === heartZoneLabelPositions[1] ? zoneLabel(2) :
                            value === heartZoneLabelPositions[2] ? zoneLabel(3) :
                            value === heartZoneLabelPositions[3] ? zoneLabel(4) :
                            value === heartZoneLabelPositions[4] ? zoneLabel(5) : undefined : undefined,
                        color: 'black',
                        padding: -50,
                        align: 'end',
                        z: 1,
                    }
                }
            }
        }
    };

    ctx = document.getElementById('canvasHeart').getContext('2d');
    var heartChart = makeChart(ctx, config);

    config = {
        type: 'line',
        plugins: [backgroundFill],
        data: {
            datasets: [
                {
                    label: t('workoutEditor.resistance', 'Resistance'),
                    //cubicInterpolationMode: 'monotone',
                    data: resistance,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                    backgroundColor: window.chartColors.red,
                    borderColor: window.chartColors.red,
                },
                {
                    label: t('chart.targetResistanceShort', 'Target R.'),
                    //cubicInterpolationMode: 'monotone',
                    data: reqresistance,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                    backgroundColor: window.chartColors.black,
                    borderColor: window.chartColors.black,
                },
            ]
        },
        options: {
            animation: {
              onComplete: function() {
                  if(saveScreenshot[2])
                      return;
                  saveScreenshot[2] = true;
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'resistance',
                          image: qzChartTheme.image(resistanceChart)
                      }
                  }, function(msg) {
                      if (msg.msg === 'R_savechart') {
                          return msg.content;
                      }
                      return null;
                  }, 15000, 3);
                  el.enqueue().catch(function(err) {
                      console.error('Error is ' + err);
                  });
              }
            },
            responsive: true,
            aspectRatio: 1.5,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                title:{
                    display:true,
                    text:t('chart.resistanceVsTarget', 'Resistance vs Target Resistance')
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false,
                    position: 'bottom',
                },
            },
            hover: {
                mode: 'nearest',
                intersect: true
            },
            scales: {
                x: {
                    type: 'linear',
                    display: true,
                    title: {
                        display: false,
                        text: 'Time'
                    },
                    ticks: {
                        // Include a dollar sign in the ticks
                        callback: function(value, index, values) {
                            return value !== 0 ? Math.floor(value / 3600).toString().padStart(2, "0") + ":" + Math.floor((value / 60) - (Math.floor(value / 3600) * 60)).toString().padStart(2, "0")  : "";
                        },
                        padding: -20,
                        //stepSize: 300,
                        align: "end",
                    },
                    max: maxEl,
                },
                y: {
                    display: true,
                    title: {
                        display: false,
                    },
                    min: 0,
                    ticks: {
                        stepSize: 10,
                        autoSkip: false,
                        color: 'black',
                        padding: -30,
                        align: 'end',
                        z: 1,
                        callback: function(value, index, values) {
                            return value !== 0 ? value  : "";
                        },
                    }
                }
            }
        }
    };

    ctx = document.getElementById('canvasResistance').getContext('2d');
    var resistanceChart = makeChart(ctx, config);

    config = {
        type: 'line',
        plugins: [backgroundFill],
        data: {
            datasets: [
                {
                    label: t('workoutEditor.resistance', 'Resistance'),
                    //cubicInterpolationMode: 'monotone',
                    data: pelotonresistance,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                    backgroundColor: window.chartColors.red,
                    borderColor: window.chartColors.red,
                },
                {
                    label: t('chart.targetResistanceShort', 'Target R.'),
                    //cubicInterpolationMode: 'monotone',
                    data: pelotonreqresistance,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                    backgroundColor: window.chartColors.black,
                    borderColor: window.chartColors.black,
                },
            ]
        },
        options: {
            animation: {
              onComplete: function() {
                  if(saveScreenshot[3])
                      return;
                  saveScreenshot[3] = true;
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'pelotonresistance',
                          image: qzChartTheme.image(pelotonresistanceChart)
                      }
                  }, function(msg) {
                      if (msg.msg === 'R_savechart') {
                          return msg.content;
                      }
                      return null;
                  }, 15000, 3);
                  el.enqueue().catch(function(err) {
                      console.error('Error is ' + err);
                  });
              }
            },
            responsive: true,
            aspectRatio: 1.5,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                title:{
                    display:true,
                    text:t('chart.pelotonResistanceVsTarget', 'Peloton Resistance vs Target Peloton Resistance')
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false,
                    position: 'bottom',
                },
            },
            hover: {
                mode: 'nearest',
                intersect: true
            },
            scales: {
                x: {
                    type: 'linear',
                    display: true,
                    title: {
                        display: false,
                        text: 'Time'
                    },
                    ticks: {
                        // Include a dollar sign in the ticks
                        callback: function(value, index, values) {
                            return value !== 0 ? Math.floor(value / 3600).toString().padStart(2, "0") + ":" + Math.floor((value / 60) - (Math.floor(value / 3600) * 60)).toString().padStart(2, "0")  : "";
                        },
                        padding: -20,
                        //stepSize: 300,
                        align: "end",
                    },
                    max: maxEl,
                },
                y: {
                    display: true,
                    title: {
                        display: false,
                    },
                    min: 0,
                    ticks: {
                        stepSize: 10,
                        autoSkip: false,
                        color: 'black',
                        padding: -30,
                        align: 'end',
                        z: 1,
                        callback: function(value, index, values) {
                            return value !== 0 ? value  : "";
                        },
                    }
                }
            }
        }
    };

    ctx = document.getElementById('canvasPelotonResistance').getContext('2d');
    var pelotonresistanceChart = makeChart(ctx, config);

    config = {
        type: 'line',
        plugins: [backgroundFill],
        data: {
            datasets: [
                {
                    backgroundColor: window.chartColors.blue,
                    borderColor: window.chartColors.blue,
                    label: t('workoutEditor.cadence', 'Cadence'),
                    //cubicInterpolationMode: 'monotone',
                    data: cadence,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                },
                {
                    backgroundColor: window.chartColors.black,
                    borderColor: window.chartColors.black,
                    label: t('chart.targetCadenceShort', 'Target C.'),
                    //cubicInterpolationMode: 'monotone',
                    data: reqcadence,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                },
            ]
        },
        options: {
            animation: {
              onComplete: function() {
                  if(saveScreenshot[4])
                      return;
                  saveScreenshot[4] = true;
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'cadence',
                          image: qzChartTheme.image(cadenceChart)
                      }
                  }, function(msg) {
                      if (msg.msg === 'R_savechart') {
                          return msg.content;
                      }
                      return null;
                  }, 15000, 3);
                  el.enqueue().catch(function(err) {
                      console.error('Error is ' + err);
                  });
              }
            },
            responsive: true,
            aspectRatio: 1.5,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                title:{
                    display:true,
                    text:t('chart.cadenceVsTarget', 'Cadence vs Target Cadence')
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false,
                    position: 'bottom',
                },
            },
            hover: {
                mode: 'nearest',
                intersect: true
            },
            scales: {
                x: {
                    type: 'linear',
                    display: true,
                    title: {
                        display: false,
                        text: 'Time'
                    },
                    ticks: {
                        // Include a dollar sign in the ticks
                        callback: function(value, index, values) {
                            return value !== 0 ? Math.floor(value / 3600).toString().padStart(2, "0") + ":" + Math.floor((value / 60) - (Math.floor(value / 3600) * 60)).toString().padStart(2, "0")  : "";
                        },
                        padding: -20,
                        //stepSize: 300,
                        align: "end",
                    },
                    max: maxEl,
                },
                y: {
                    display: true,
                    title: {
                        display: false,
                    },
                    min: 0,
                    ticks: {
                        stepSize: 10,
                        autoSkip: false,
                        color: 'black',
                        padding: -30,
                        align: 'end',
                        z: 1,
                        callback: function(value, index, values) {
                            return value !== 0 ? value  : "";
                        },
                    }
                }
            }
        }
    };

    ctx = document.getElementById('canvasCadence').getContext('2d');
    var cadenceChart = makeChart(ctx, config);

    config = {
        type: 'bar',
        plugins: [backgroundFill],
        data: {
            labels: [zoneLabel(1), zoneLabel(2), zoneLabel(3), zoneLabel(4), zoneLabel(5), zoneLabel(6), zoneLabel(7) ],
            datasets: [
                {
                    data: distributionPowerZones,
                    backgroundColor: [
                      window.chartColors.lightsteelbluet, window.chartColors.limegreent,window.chartColors.goldt,window.chartColors.oranget,window.chartColors.darkoranget,window.chartColors.orangeredt, window.chartColors.redt
                    ],
                },
            ]
        },
        options: {
            animation: {
              onComplete: function() {
                  if(saveScreenshot[5])
                      return;
                  saveScreenshot[5] = true;
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'powerDistribution',
                          image: qzChartTheme.image(powerDistributionChart)
                      }
                  }, function(msg) {
                      if (msg.msg === 'R_savechart') {
                          return msg.content;
                      }
                      return null;
                  }, 15000, 3);
                  el.enqueue().catch(function(err) {
                      console.error('Error is ' + err);
                  });
              }
            },
            responsive: true,
            aspectRatio: 1.5,
            plugins: {
                title:{
                    display:true,
                    text:t('chart.powerDistribution', 'Power Distribution')
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false,
                    position: 'right',
                },
            },
            hover: {
                mode: 'nearest',
                intersect: true
            },
            scales: {
                y: {
                    type: 'linear',
                    display: true,
                    title: {
                        display: false,
                        text: 'Time'
                    },
                    ticks: {
                        // Include a dollar sign in the ticks
                        callback: function(value, index, values) {
                            return value !== 0 ? Math.floor(value / 60).toString().padStart(2, "0") + ":" + Math.floor(value % 60).toString().padStart(2, "0")  : "";
                        },
                        align: "end",
                    },
                },
            }
        }
    };

    ctx = document.getElementById('canvasPowerDistribution').getContext('2d');
    var powerDistributionChart = makeChart(ctx, config);

    config = {
        type: 'line',
        plugins: [backgroundFill],
        data: {
            datasets: [
                {
                    backgroundColor: window.chartColors.blue,
                    borderColor: window.chartColors.blue,
                    label: miles === 1 ? t('workoutEditor.speedKmh', 'Speed (km/h)') : t('workoutEditor.speedMph', 'Speed (mph)'),
                    //cubicInterpolationMode: 'monotone',
                    data: speed,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                },
                {
                    backgroundColor: window.chartColors.green,
                    borderColor: window.chartColors.green,
                    label: t('metric.inclinationTitle', 'Inclination'),
                    //cubicInterpolationMode: 'monotone',
                    data: inclination,
                    fill: false,
                    pointRadius: 0,
                    borderWidth: 2,
                },
            ]
        },
        options: {
            animation: {
              onComplete: function() {
                  if(saveScreenshot[6])
                      return;
                  saveScreenshot[6] = true;
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'speedInclination',
                          image: qzChartTheme.image(speedInclinationChart)
                      }
                  }, function(msg) {
                      if (msg.msg === 'R_savechart') {
                          return msg.content;
                      }
                      return null;
                  }, 15000, 3);
                  el.enqueue().catch(function(err) {
                      console.error('Error is ' + err);
                  });
              }
            },
            responsive: true,
            aspectRatio: 1.5,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                title:{
                    display:true,
                    text:t('chart.speedAndInclination', 'Speed and Inclination')
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false,
                    position: 'bottom',
                },
            },
            hover: {
                mode: 'nearest',
                intersect: true
            },
            scales: {
                x: {
                    type: 'linear',
                    display: true,
                    title: {
                        display: false,
                        text: 'Time'
                    },
                    ticks: {
                        // Include a dollar sign in the ticks
                        callback: function(value, index, values) {
                            return value !== 0 ? Math.floor(value / 3600).toString().padStart(2, "0") + ":" + Math.floor((value / 60) - (Math.floor(value / 3600) * 60)).toString().padStart(2, "0")  : "";
                        },
                        padding: -20,
                        //stepSize: 300,
                        align: "end",
                    },
                    max: maxEl,
                },
                y: {
                    display: true,
                    title: {
                        display: false,
                    },
                    min: 0,
                    ticks: {
                        stepSize: 5,
                        autoSkip: false,
                        color: 'black',
                        padding: -30,
                        align: 'end',
                        z: 1,
                        callback: function(value, index, values) {
                            return value !== 0 ? value  : "";
                        },
                    }
                }
            }
        }
    };

    ctx = document.getElementById('canvasSpeedInclination').getContext('2d');
    var speedInclinationChart = makeChart(ctx, config);
}

function dochart_init() {
    keys_arr = ['ftp', 'miles_unit', 'age', 'heart_rate_zone1', 'heart_rate_zone2', 'heart_rate_zone3', 'heart_rate_zone4', 'heart_max_override_enable', 'heart_max_override_value']

    function scheduleLiveRefresh() {
        clearTimeout(liveTimer);
        liveTimer = null;
        // not before the first load: it runs after getsettings and draws with the right zones
        if (livePolling() && !liveLoading && lastSessionLength >= 0 && !document.hidden)
            liveTimer = setTimeout(load_workout_data, LIVE_REFRESH_MS);
        else
            drawPendingLive();
    }

    // the polling works only while a workout is running: device connected, not paused and not
    // stopped (stop pauses the device); state comes with the 'workout' push
    function setLiveActive(active) {
        let firstState = !liveStateKnown;
        liveStateKnown = true;
        if (active === liveActive)
            return;
        liveActive = active;
        if (!active) {
            clearTimeout(liveTimer);
            liveTimer = null;
            // pick up the last points before going idle; a load in flight may predate the stop
            if (liveLoading)
                liveFinalPending = true;
            else if (lastSessionLength >= 0)
                load_workout_data();
        } else {
            // a start or a resume: the session may be a new one of any length (the app clears
            // it on start only), so the next load takes it whole instead of appending a part.
            // Not on the first state the page gets: the page has just loaded the session whole
            if (!firstState)
                sessionCache = null;
            scheduleLiveRefresh();
        }
    }

    function watchWorkoutState() {
        let el = new MainWSQueueElement({
            msg: null
        }, function(msg) {
            if (msg.msg === 'workout') {
                return msg.content;
            }
            return null;
        });
        el.enqueue().then(function(w) {
            setLiveActive(!!(w && w.deviceId && w.deviceConnected && !w.devicePaused));
        }).catch(function(err) {
            // {msg: null} is still sent, so the element times out (5 s) when no push comes;
            // the state stays as it was and the watch goes on: without this catch it stopped
        }).then(function() {
            setTimeout(watchWorkoutState, 500);
        });
    }
    watchWorkoutState();

    // a page kept out of sight (the app may hide it instead of closing) does not poll
    document.addEventListener('visibilitychange', scheduleLiveRefresh);

    function load_workout_data() {
        liveLoading = true;
        let request = {
            msg: 'getsessionarray'
        };
        // after the first load only the new points are asked for; an app that does not know
        // "from" (or a session started anew) answers with the whole array, without "from"
        if (sessionCache)
            request.content = { from: sessionCache.length };
        let el = new MainWSQueueElement(request, function(msg) {
            if (msg.msg === 'R_getsessionarray' || msg.msg === 'R_getsessionarraypart') {
                return msg;
            }
            return null;
        }, 15000, 3);
        el.enqueue().then(function(msg) {
            let arr = msg && msg.content;
            // a part fits only the cache it was asked for; one asked before the cache was
            // dropped (a new start) is thrown away, the next load takes the whole session
            if (arr && typeof msg.from === 'number')
                arr = sessionCache && msg.from === sessionCache.length ? sessionCache.concat(arr) : null;
            if (!arr && msg && msg.content)
                liveFinalPending = true; // load again at once, whole
            if (arr)
                sessionCache = arr;
            // redraw only when the session grew: after stop the array stays the same,
            // so the end-of-workout screenshots are taken just once
            if (arr && arr.length !== lastSessionLength) {
                liveRefresh = lastSessionLength >= 0;
                lastSessionLength = arr.length;
                process_arr(arr);
            }
        }).catch(function(err) {
            console.error('Error is ' + err);
        }).then(function() {
            liveLoading = false;
            if (liveFinalPending) {
                liveFinalPending = false;
                load_workout_data();
            } else
                scheduleLiveRefresh();
        });
    }

    let el = new MainWSQueueElement({
            msg: 'getsettings',
            content: {
                keys: keys_arr
            }
        }, function(msg) {
            if (msg.msg === 'R_getsettings') {
                var heart_max_override_enable = false;
                var heart_max_override_value = 195;
                var heart_rate_zone1 = 0;
                var heart_rate_zone2 = 0;
                var heart_rate_zone3 = 0;
                var heart_rate_zone4 = 0;

                for (let key of keys_arr) {
                    if (msg.content[key] === undefined)
                        return null;
                    if (key === 'ftp') {
                        ftp = msg.content[key];
                        ftpZones[0] = Math.round(ftp * 0.55);
                        ftpZones[1] = Math.round(ftp * 0.75);
                        ftpZones[2] = Math.round(ftp * 0.90);
                        ftpZones[3] = Math.round(ftp * 1.05);
                        ftpZones[4] = Math.round(ftp * 1.20);
                        ftpZones[5] = Math.round(ftp * 1.50);
                    } else if (key === 'age') {
                        age = msg.content[key];
                        maxHeartRate = 220 - age;
                    } else if (key === 'heart_max_override_enable') {
                        heart_max_override_enable = isTrueSetting(msg.content[key]);
                    } else if (key === 'heart_max_override_value') {
                        heart_max_override_value = msg.content[key];
                    } else if (key === 'heart_rate_zone1') {
                        heart_rate_zone1 = msg.content[key];
                        heartZones[0] = Math.round(maxHeartRate * (msg.content[key] / 100));
                    } else if (key === 'heart_rate_zone2') {
                        heart_rate_zone2 = msg.content[key];
                        heartZones[1] = Math.round(maxHeartRate * (msg.content[key] / 100));
                    } else if (key === 'heart_rate_zone3') {
                        heart_rate_zone3 = msg.content[key];
                        heartZones[2] = Math.round(maxHeartRate * (msg.content[key] / 100));
                    } else if (key === 'heart_rate_zone4') {
                        heart_rate_zone4 = msg.content[key];
                        heartZones[3] = Math.round(maxHeartRate * (msg.content[key] / 100));
                    } else if (key === 'miles_unit') {
                        if(msg.content[key] === true || msg.content[key] === 'true')
                            miles = 0.621371;
                    }
                }
                if(heart_max_override_enable) {
                    maxHeartRate = heart_max_override_value;
                    heartZones[0] = Math.round(maxHeartRate * (heart_rate_zone1 / 100));
                    heartZones[1] = Math.round(maxHeartRate * (heart_rate_zone2 / 100));
                    heartZones[2] = Math.round(maxHeartRate * (heart_rate_zone3 / 100));
                    heartZones[3] = Math.round(maxHeartRate * (heart_rate_zone4 / 100));
                }
                return msg.content;
            }
            return null;
        }, 5000, 3);
    el.enqueue().then(function() {
        ensurePowerZones();
        ensureHeartZones();
        load_workout_data();
    }).catch(function(err) {
        console.error('Error is ' + err);
        ensurePowerZones();
        ensureHeartZones();
        load_workout_data();
    })

    let getPelotonImage = new MainWSQueueElement({
        msg: 'getpelotonimage',
    }, function(msg) {
        if (msg.msg === 'R_getpelotonimage' && msg.content.length > 0) {
            // the Peloton picture of the class: not a sport icon to restyle (qzChartTheme.sportIcon)
            $('.workout_image').removeAttr("data-classic-src").attr("src","data:image/png;base64," + msg.content);
        }
        return null;
    }, 15000, 1);
    getPelotonImage.enqueue().catch(function(err) {
        console.error('Error is ' + err);
    });

}


$(window).on('load', function () {
    dochart_init(); return;

    // DEBUG
    ftpZones[0] = Math.round(ftp * 0.55);
    ftpZones[1] = Math.round(ftp * 0.75);
    ftpZones[2] = Math.round(ftp * 0.90);
    ftpZones[3] = Math.round(ftp * 1.05);
    ftpZones[4] = Math.round(ftp * 1.20);
    ftpZones[5] = Math.round(ftp * 1.50);

    heartZones[0] = 110;
    heartZones[1] = 130;
    heartZones[2] = 150;
    heartZones[3] = 170;    

    arr = [{'watts': 50, 'req_power': 150, 'elapsed_s':0,'elapsed_m':0,'elapsed_h':0, 'heart':90, 'resistance': 10, 'req_resistance': 15, 'cadence': 80, 'req_cadence': 90, 'speed': 10, 'inclination': 1, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 60, 'req_power': 150, 'elapsed_s':1,'elapsed_m':1,'elapsed_h':0, 'heart':92, 'resistance': 11, 'req_resistance': 30, 'cadence': 90, 'req_cadence': 100, 'speed': 8, 'inclination': 2, 'peloton_resistance': 20, 'peloton_req_resistance': 25},
           {'watts': 70, 'req_power': 170, 'elapsed_s':2,'elapsed_m':2,'elapsed_h':0, 'heart':110, 'resistance': 12, 'req_resistance': 40, 'cadence': 100, 'req_cadence': 90, 'speed': 9, 'inclination': 2.5, 'peloton_resistance': 30, 'peloton_req_resistance': 35},
           {'watts': 140, 'req_power': 170, 'elapsed_s':3,'elapsed_m':3,'elapsed_h':0, 'heart':115, 'resistance': 16, 'req_resistance': 41, 'cadence': 90, 'req_cadence': 95, 'speed': 11, 'inclination': 1, 'peloton_resistance': 40, 'peloton_req_resistance': 45},
           {'watts': 130, 'req_power': 170, 'elapsed_s':4,'elapsed_m':4,'elapsed_h':0, 'heart':130, 'resistance': 18, 'req_resistance': 43, 'cadence': 95, 'req_cadence': 95, 'speed': 10, 'inclination': 4, 'peloton_resistance': 50, 'peloton_req_resistance': 55},
           {'watts': 160, 'req_power': 170, 'elapsed_s':5,'elapsed_m':5,'elapsed_h':0, 'heart':135, 'resistance': 22, 'req_resistance': 43, 'cadence': 95, 'req_cadence': 95, 'speed': 12, 'inclination': 1, 'peloton_resistance': 60, 'peloton_req_resistance': 15},
           {'watts': 180, 'req_power': 130, 'elapsed_s':6,'elapsed_m':6,'elapsed_h':0, 'heart':140, 'resistance': 31, 'req_resistance': 43, 'cadence': 95, 'req_cadence': 90, 'speed': 10, 'inclination': 3, 'peloton_resistance': 70, 'peloton_req_resistance': 15},
           {'watts': 120, 'req_power': 130, 'elapsed_s':7,'elapsed_m':7,'elapsed_h':0, 'heart':150, 'resistance': 18, 'req_resistance': 35, 'cadence': 95, 'req_cadence': 80, 'speed': 10, 'inclination': 4, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 190, 'req_power': 150, 'elapsed_s':1,'elapsed_m':8,'elapsed_h':0, 'heart':155, 'resistance': 17, 'req_resistance': 35, 'cadence': 95, 'req_cadence': 80, 'speed': 13, 'inclination': 1, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 195, 'req_power': 170, 'elapsed_s':2,'elapsed_m':9,'elapsed_h':0, 'heart':165, 'resistance': 19, 'req_resistance': 30, 'cadence': 80, 'req_cadence': 80, 'speed': 12, 'inclination': 3, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 200, 'req_power': 170, 'elapsed_s':3,'elapsed_m':10,'elapsed_h':0, 'heart':153, 'resistance': 20, 'req_resistance': 25, 'cadence': 90, 'req_cadence': 90, 'speed': 10, 'inclination': 2, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 206, 'req_power': 170, 'elapsed_s':4,'elapsed_m':11,'elapsed_h':0, 'heart':152, 'resistance': 21, 'req_resistance': 35, 'cadence': 90, 'req_cadence': 90, 'speed': 12, 'inclination': 7, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 211, 'req_power': 170, 'elapsed_s':5,'elapsed_m':12,'elapsed_h':0, 'heart':180, 'resistance': 25, 'req_resistance': 35, 'cadence': 90, 'req_cadence': 70, 'speed': 10, 'inclination': 10, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 222, 'req_power': 130, 'elapsed_s':6,'elapsed_m':13,'elapsed_h':0, 'heart':182, 'resistance': 31, 'req_resistance': 35, 'cadence': 80, 'req_cadence': 70, 'speed': 7, 'inclination': 12, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 237, 'req_power': 130, 'elapsed_s':7,'elapsed_m':14,'elapsed_h':0, 'heart':160, 'resistance': 20, 'req_resistance': 50, 'cadence': 90, 'req_cadence': 70, 'speed': 6, 'inclination': 1, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 250, 'req_power': 170, 'elapsed_s':3,'elapsed_m':15,'elapsed_h':0, 'heart':115, 'resistance': 20, 'req_resistance': 50, 'cadence': 90, 'req_cadence': 90, 'speed': 10, 'inclination': 14, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 266, 'req_power': 170, 'elapsed_s':4,'elapsed_m':16,'elapsed_h':0, 'heart':120, 'resistance': 11, 'req_resistance': 35, 'cadence': 80, 'req_cadence': 60, 'speed': 10, 'inclination': 10, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 351, 'req_power': 170, 'elapsed_s':5,'elapsed_m':17,'elapsed_h':0, 'heart':112, 'resistance': 22, 'req_resistance': 23, 'cadence': 80, 'req_cadence': 60, 'speed': 5, 'inclination': 9, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 322, 'req_power': 130, 'elapsed_s':6,'elapsed_m':18,'elapsed_h':0, 'heart':90, 'resistance': 25, 'req_resistance': 23, 'cadence': 80, 'req_cadence': 96, 'speed': 10, 'inclination': 5, 'peloton_resistance': 10, 'peloton_req_resistance': 15},
           {'watts': 257, 'req_power': 130, 'elapsed_s':7,'elapsed_m':19,'elapsed_h':0, 'heart':120, 'resistance': 10, 'req_resistance': 23, 'cadence': 80, 'req_cadence': 97, 'speed': 10, 'inclination': 1, 'workoutName': '45min Power Zone Ride', 'workoutStartDate': '20/12/2021', 'instructorName': "Robin Arzon", 'watts_avg': 200, 'watts_max' : 351, 'heart_avg': 120, 'heart_max' : 150, 'jouls': 138000, 'calories': 950, 'distance': 11, 'cadence_avg': 65, 'peloton_resistance_avg': 22, 'deviceType': 1},
            ]
    process_arr(arr);
});

$(document).ready(function () {
    $('#loading').hide();
});
