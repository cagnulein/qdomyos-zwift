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

// Define speed zones
const speedZones = [6, 8, 10, 12, 14, 16]; // km/h

function process_arr(arr) {    
    let ctx = document.getElementById('canvas').getContext('2d');
    let div = document.getElementById('divcanvas');

    let speed = [];
    let targetSpeed = [];
    let inclination = [];
    let targetInclination = [];
    let maxEl = 0;

    for (let el of arr) {
        let time = el.elapsed_s + el.elapsed_m * 60 + el.elapsed_h * 3600;
        maxEl = time;

        if (el.speed !== undefined) {
            speed.push({x: time, y: el.speed * miles});
            if (speed_max < el.speed * miles) speed_max = el.speed * miles;
        }

        if (el.target_speed !== undefined && el.target_speed !== -1) {
            targetSpeed.push({x: time, y: el.target_speed * miles});
            if (speed_max < el.target_speed * miles) speed_max = el.target_speed * miles;
        }

        if (el.inclination !== undefined) {
            inclination.push({x: time, y: el.inclination});
            if (incline_max < el.inclination) incline_max = el.inclination;
        }

        if (el.target_inclination !== undefined && el.target_inclination !== -200) {
            targetInclination.push({x: time, y: el.target_inclination});
            if (incline_max < el.target_inclination) incline_max = el.target_inclination;
        }
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
            datasets: [{
                label: 'Speed (' + (miles === 1 ? 'km/h' : 'mph') + ')',
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
                        if (y < speedZones[0]) return window.chartColors.grey;
                        if (y < speedZones[1]) return window.chartColors.limegreen;
                        if (y < speedZones[2]) return window.chartColors.gold;
                        if (y < speedZones[3]) return window.chartColors.orange;
                        if (y < speedZones[4]) return window.chartColors.darkorange;
                        if (y < speedZones[5]) return window.chartColors.orangered;
                        return window.chartColors.red;
                    }
                }
            }, {
                label: 'Target Speed (' + (miles === 1 ? 'km/h' : 'mph') + ')',
                backgroundColor: window.chartColors.black,
                borderColor: window.chartColors.black,
                data: targetSpeed,
                fill: false,
                pointRadius: 0,
                borderWidth: 2,
                yAxisID: 'y-speed',
                borderDash: [5, 5]
            }, {
                label: 'Incline',
                backgroundColor: window.chartColors.orange,
                borderColor: window.chartColors.orange,
                data: inclination,
                fill: false,
                pointRadius: 0,
                borderWidth: 2,
                yAxisID: 'y-incline'
            }, {
                label: 'Target Incline',
                backgroundColor: window.chartColors.grey,
                borderColor: window.chartColors.grey,
                data: targetInclination,
                fill: false,
                pointRadius: 0,
                borderWidth: 2,
                yAxisID: 'y-incline',
                borderDash: [5, 5]
            }]
        },
        options: {           
            responsive: true,
            aspectRatio: div.width / div.height,
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
                    annotations: {
                        box1: {
                            type: 'box',
                            xMin: 0,
                            yMin: 0,
                            yMax: speedZones[0],
                            backgroundColor: "#d6d6d620",
                            yScaleID: 'y-speed',
                        },
                        box2: {
                            type: 'box',
                            xMin: 0,
                            yMin: speedZones[0],
                            yMax: speedZones[1],
                            backgroundColor: window.chartColors.limegreent,
                            yScaleID: 'y-speed',
                        },
                        box3: {
                            type: 'box',
                            xMin: 0,
                            yMin: speedZones[1],
                            yMax: speedZones[2],
                            backgroundColor: window.chartColors.goldt,
                            yScaleID: 'y-speed',
                        },
                        box4: {
                            type: 'box',
                            xMin: 0,
                            yMin: speedZones[2],
                            yMax: speedZones[3],
                            backgroundColor: window.chartColors.oranget,
                            yScaleID: 'y-speed',
                        },
                        box5: {
                            type: 'box',
                            xMin: 0,
                            yMin: speedZones[3],
                            yMax: speedZones[4],
                            backgroundColor: window.chartColors.darkoranget,
                            yScaleID: 'y-speed',
                        },
                        box6: {
                            type: 'box',
                            xMin: 0,
                            yMin: speedZones[4],
                            yMax: speedZones[5],
                            backgroundColor: window.chartColors.orangeredt,
                            yScaleID: 'y-speed',
                        },
                        box7: {
                            type: 'box',
                            xMin: 0,
                            yMin: speedZones[5],
                            yMax: speed_max,
                            backgroundColor: window.chartColors.redt,
                            yScaleID: 'y-speed',
                        }
                    }
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
                        display: false
                    },
                    min: 0,
                    max: speed_max,
                    ticks: {
                        stepSize: 1,
                        autoSkip: false,
                        callback: value => speedZones.includes(value) ?
                            'Speed z' + (speedZones.indexOf(value) + 1) : undefined,
                        color: 'black',
                        padding: -70,
                        align: 'end',
                    }
                },
                'y-incline': {
                    type: 'linear',
                    display: true,
                    position: 'right',
                    title: {
                        display: false
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
    // Update speed data
    treadmillChart.data.datasets[0].data.push({
        x: arr.elapsed_s + (arr.elapsed_m * 60) + (arr.elapsed_h * 3600),
        y: arr.speed * miles
    });
    if (speed_max < arr.speed * miles) {
        speed_max = Math.ceil(arr.speed * miles * 1.1);
        treadmillChart.options.scales['y-speed'].max = speed_max;
    }

    // Update target speed
    if (arr.target_speed !== undefined && arr.target_speed !== -1) {
        treadmillChart.data.datasets[1].data.push({
            x: arr.elapsed_s + (arr.elapsed_m * 60) + (arr.elapsed_h * 3600),
            y: arr.target_speed * miles
        });
        if (speed_max < arr.target_speed * miles) {
            speed_max = Math.ceil(arr.target_speed * miles * 1.1);
            treadmillChart.options.scales['y-speed'].max = speed_max;
        }
    }

    // Update inclination data
    treadmillChart.data.datasets[2].data.push({
        x: arr.elapsed_s + (arr.elapsed_m * 60) + (arr.elapsed_h * 3600),
        y: arr.inclination
    });
    if (incline_max < arr.inclination) {
        incline_max = Math.ceil(arr.inclination * 1.1);
        treadmillChart.options.scales['y-incline'].max = incline_max;
    }

    // Update target inclination
    if (arr.target_inclination !== undefined && arr.target_inclination !== -200) {
        treadmillChart.data.datasets[3].data.push({
            x: arr.elapsed_s + (arr.elapsed_m * 60) + (arr.elapsed_h * 3600),
            y: arr.target_inclination
        });
        if (incline_max < arr.target_inclination) {
            incline_max = Math.ceil(arr.target_inclination * 1.1);
            treadmillChart.options.scales['y-incline'].max = incline_max;
        }
    }

    treadmillChart.update();
    refresh();
}

function dochart_init() {
    // Get miles_unit setting first
    let el = new MainWSQueueElement({
        msg: 'getsettings',
        content: {
            keys: ['miles_unit']
        }
    }, function(msg) {
        if (msg.msg === 'R_getsettings') {
            if (msg.content['miles_unit'] === true || msg.content['miles_unit'] === 'true') {
                miles = 0.621371;
            }
            return msg.content;
        }
        return null;
    }, 5000, 3);
    el.enqueue().catch(function(err) {
        console.error('Error getting settings: ' + err);
    });

    // Get session array
    el = new MainWSQueueElement({
        msg: 'getsessionarray'
    }, function(msg) {
        if (msg.msg === 'R_getsessionarray') {
            return msg.content;
        }
        return null;
    }, 15000, 3);
    el.enqueue().then(process_arr).catch(function(err) {
        console.error('Error is ' + err);
    });
}

$(window).on('load', function() {
    dochart_init();

    /*
    let testData = [
        {'speed': 5, 'target_speed': 5, 'inclination': 1, 'target_inclination': 1, 'elapsed_s': 0, 'elapsed_m': 0, 'elapsed_h': 0},
        {'speed': 8, 'target_speed': 3, 'inclination': 2, 'target_inclination': 2, 'elapsed_s': 0, 'elapsed_m': 5, 'elapsed_h': 0},
        {'speed': 10, 'target_speed': 5, 'inclination': 4, 'target_inclination': 4, 'elapsed_s': 0, 'elapsed_m': 10, 'elapsed_h': 0},
        {'speed': 12, 'target_speed': 7, 'inclination': 6, 'target_inclination': 8, 'elapsed_s': 0, 'elapsed_m': 15, 'elapsed_h': 0},
        {'speed': 14, 'target_speed': 14, 'inclination': 8, 'target_inclination': 10, 'elapsed_s': 0, 'elapsed_m': 20, 'elapsed_h': 0},
        {'speed': 16, 'target_speed': 16, 'inclination': 10, 'target_inclination': 12, 'elapsed_s': 0, 'elapsed_m': 25, 'elapsed_h': 0},
        {'speed': 8, 'target_speed': 8, 'inclination': 2, 'target_inclination': 4, 'elapsed_s': 0, 'elapsed_m': 30, 'elapsed_h': 0}
    ];
    process_arr(testData);
    */
});

$(document).ready(function () {
    $('#loading').hide();
});
