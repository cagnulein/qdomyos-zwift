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
var powerChart = null;
var watts_max = 0;

function process_trainprogram(arr) {
    let powerWorkout = false;
    let elapsed = 0;

    for (let el of arr.list) {
        // if the power is ok or it was a power zone workout but this segment is a freeride tag...
        if(el.power !== -1 || powerWorkout) {
            powerWorkout = true;
            for (i=0; i<el.duration_s; i++) {
                powerChart.data.datasets[1].data.push({x: elapsed++, y: el.power});
                if(watts_max < el.power)
                    watts_max = el.power;
            }
        }
    }
    powerChart.options.scales.x.max = elapsed;
    powerChart.update();
}

function process_arr(arr) {    
    let ctx = document.getElementById('canvas').getContext('2d');
    let div = document.getElementById('divcanvas');

    let reqpower = [];
    let reqcadence = [];
    let heart = [];
    let cadence = [];
    let speed = [];
    let inclination = [];
    let resistance = [];
    let watts = [];
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
    let heart_avg = 0;
    let heart_max = 0;
    let jouls = 0;
    let deviceType = 0;
    let cadence_avg = 0;
    let peloton_resistance_avg = 0;
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
        if(watts_max < el.watts)
            watts_max = el.watts;
        heart_avg = el.heart_avg;
        heart_max = el.heart_max;
        jouls = el.jouls;
        deviceType = el.deviceType;
        peloton_resistance_avg = el.peloton_resistance_avg;
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
        // they are added from the process_trainprogram()
        //reqpower.push(reqpowerel);

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
        speedel.y = el.speed;
        speed.push(speedel);
        inclinationel.x = time;
        inclinationel.y = el.inclination;
        inclination.push(inclinationel);
    }

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
                label: 'Watts',
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
                label: 'Req. Watts',
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
            responsive: true,
            aspectRatio: div.width / div.height,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                /*
                title:{
                    display:true,
                    backgroundColor: "#1d2330",
                    padding: {
                        top: 2,
                        bottom: 2
                    },
                    text:'Watt'
                },*/
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
                            //xMax: maxEl,
                            yMin: 0,
                            yMax: ftpZones[0],
                            backgroundColor: "#d6d6d620"
                            },
                            box2: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            //xMax: maxEl,
                            yMin: ftpZones[0],
                            yMax: ftpZones[1],
                            backgroundColor: window.chartColors.limegreent,
                            },
                            box3: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            //xMax: maxEl,
                            yMin: ftpZones[1],
                            yMax: ftpZones[2],
                            backgroundColor: window.chartColors.goldt,
                            },
                            box4: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            //xMax: maxEl,
                            yMin: ftpZones[2],
                            yMax: ftpZones[3],
                            backgroundColor: window.chartColors.oranget,
                            },
                            box5: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            //xMax: maxEl,
                            yMin: ftpZones[3],
                            yMax: ftpZones[4],
                            backgroundColor: window.chartColors.darkoranget,
                            },
                            box6: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            //xMax: maxEl,
                            yMin: ftpZones[4],
                            yMax: ftpZones[5],
                            backgroundColor: window.chartColors.orangeredt,
                            },
                            box7: {
                            // Indicates the type of annotation
                            type: 'box',
                            xMin: 0,
                            //xMax: maxEl,
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
                    //max: maxEl,
                },
                y: {
                    display: true,
                    title: {
                        display: false,
                        text: 'Watt'
                    },
                    min: 0,
                    suggestedMax: (watts_max > ftpZones[4] + 10 ? watts_max + 10 : ftpZones[4] + 10),
                    ticks: {
                        stepSize: 1,
                        autoSkip: false,
                        callback: value => [ftpZones[0] * 0.8, ftpZones[0], ftpZones[1], ftpZones[2], ftpZones[3], ftpZones[4], ftpZones[5]].includes(value) ?
                            value === ftpZones[0] * 0.8 ? 'power z1' :
                            value === ftpZones[0] ? 'power z2' :
                            value === ftpZones[1] ? 'power z3' :
                            value === ftpZones[2] ? 'power z4' :
                            value === ftpZones[3] ? 'power z5' :
                            value === ftpZones[4] ? 'power z6' :
                            value === ftpZones[5] ? 'power z7' : undefined : undefined,
                        color: 'black',
                        padding: -70,
                        align: 'end',
                        z: 1,
                    }
                }
            }
        }
    };    
    powerChart = new Chart(ctx, config);

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
    powerChart.data.datasets[0].data.push({x: arr.elapsed_s + (arr.elapsed_m * 60) + (arr.elapsed_h * 3600), y: arr.watts});
    if(watts_max < arr.watts)
        watts_max = arr.watts;
    powerChart.update();
    refresh();
}

function dochart_init() {
    onSettingsOK = true;
    keys_arr = ['ftp', 'miles_unit', 'age', 'heart_rate_zone1', 'heart_rate_zone2', 'heart_rate_zone3', 'heart_rate_zone4', 'heart_max_override_enable', 'heart_max_override_value']
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
                        heart_max_override_enable = msg.content[key];
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
    el.enqueue().then(onSettingsOK).catch(function(err) {
            console.error('Error is ' + err);
    })

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

    el = new MainWSQueueElement({
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
