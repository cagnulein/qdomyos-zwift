window.chartColors = {
    red: 'rgb(255, 99, 132)',
    orange: 'rgb(255, 159, 64)',
    darkorange: 'rgb(255, 140, 0)',
    orangered: 'rgb(255, 69, 0)',
    yellow: 'rgb(255, 205, 86)',
    green: 'rgb(75, 192, 192)',
    blue: 'rgb(54, 162, 235)',
    purple: 'rgb(153, 102, 255)',
    grey: 'rgb(201, 203, 207)',
    white: 'rgb(255, 255, 255)',
    limegreen: 'rgb(50, 205, 50)',
    gold: 'rgb(255, 215, 0)',
    black: 'rgb(0, 0, 0)',
};

var ftp = 200;
var ftpZones = [];
var maxHeartRate = 190;
var heartZones = [];

function process_arr(arr) {
    let watts = [];
    let reqpower = [];
    let heart = [];
    let cadence = [];
    let resistance = [];
    let peloton_resistance = [];
    for (let el of arr) {
        let wattel = {};
        let reqpowerel = {};
        let heartel = {};
        let cadenceel = {};
        let resistanceel = {};
        let pelotonresistanceel = {};
        let time = el.elapsed_s + el.elapsed_m * 60 + el.elapsed_h * 3600;
        wattel.x = time;
        wattel.y = el.watts;
        watts.push(wattel);
        reqpowerel.x = time;
        reqpowerel.y = el.req_power;
        reqpower.push(reqpowerel);
        heartel.x = time;
        heartel.y = el.heart;
        heart.push(heartel);
        cadenceel.x = time;
        cadenceel.y = el.cadence;
        cadence.push(cadenceel);
        resistanceel.x = time;
        resistanceel.y = el.resistance;
        resistance.push(resistanceel);
        pelotonresistanceel.x = time;
        pelotonresistanceel.y = el.peloton_resistance;
        peloton_resistance.push(pelotonresistanceel);
    }
    let config = {
        type: 'line',
        data: {
            datasets: [{
                label: 'Watts',
                backgroundColor: window.chartColors.red,
                borderColor: window.chartColors.red,
                cubicInterpolationMode: 'monotone',
                data: watts,
                fill: false,
                pointRadius: 0,
                segment: {
                   borderColor: ctx => ctx.p0.parsed.y < ftpZones[0] && ctx.p1.parsed.y < ftpZones[0] ? window.chartColors.white :
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
                borderWidth: 5,
                cubicInterpolationMode: 'monotone',
                data: reqpower,
                fill: false,
                pointRadius: 0,
            },
            ]
        },
        options: {
            animation: {
              onComplete: function() {
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'power',
                          image: powerChart.toBase64Image()
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
                    text:'Watt'
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false
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
                        display: true,
                        text: 'Time'
                    },
                    ticks: {
                        // Include a dollar sign in the ticks
                        callback: function(value, index, values) {
                            return Math.floor(value / 3600).toString().padStart(2, "0") + ":" + Math.floor(value / 60).toString().padStart(2, "0");
                        }
                    }
                },
                y: {
                    display: true,
                    title: {
                        display: true,
                        text: 'Watt'
                    },
                    ticks: {
                        stepSize: 1,
                        autoSkip: false,
                        callback: value => [ftpZones[0] * 0.8, ftpZones[0], ftpZones[1], ftpZones[2], ftpZones[3], ftpZones[4], ftpZones[5]].includes(value) ?
                            value === ftpZones[0] * 0.8 ? 'zone 1' :
                            value === ftpZones[0] ? 'zone 2' :
                            value === ftpZones[1] ? 'zone 3' :
                            value === ftpZones[2] ? 'zone 4' :
                            value === ftpZones[3] ? 'zone 5' :
                            value === ftpZones[4] ? 'zone 6' : undefined : undefined,
                        color: 'black',
                    }
                }
            }
        }
    };

    let ctx = document.getElementById('canvas').getContext('2d');
    var powerChart = new Chart(ctx, config);

    config = {
        type: 'line',
        data: {
            datasets: [{
                label: 'Heart',
                backgroundColor: window.chartColors.red,
                borderColor: window.chartColors.red,
                cubicInterpolationMode: 'monotone',
                data: heart,
                fill: true,
                borderWidth: 5,
                pointRadius: 0,
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
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'heart',
                          image: powerChart.toBase64Image()
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
                    text:'Heart Rate'
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: false
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
                        display: true,
                        text: 'Time'
                    },
                    ticks: {
                        // Include a dollar sign in the ticks
                        callback: function(value, index, values) {
                            return Math.floor(value / 3600).toString().padStart(2, "0") + ":" + Math.floor(value / 60).toString().padStart(2, "0");
                        }
                    }
                },
                y: {
                    display: true,
                    title: {
                        display: true,
                        text: 'Heart rate'
                    },
                    ticks: {
                        stepSize: 1,
                        autoSkip: false,
                        callback: value => [heartZones[0] * 0.8, heartZones[0], heartZones[1], heartZones[2], heartZones[3], heartZones[4]].includes(value) ?
                            value === heartZones[0] * 0.8 ? 'zone 1' :
                            value === heartZones[0] ? 'zone 2' :
                            value === heartZones[1] ? 'zone 3' :
                            value === heartZones[2] ? 'zone 4' :
                            value === heartZones[3] ? 'zone 5' : undefined : undefined,
                        color: 'black',
                    }
                }
            }
        }
    };

    ctx = document.getElementById('canvasHeart').getContext('2d');
    var heartChart = new Chart(ctx, config);

    config = {
        type: 'line',
        data: {
            datasets: [
                {
                    label: 'Resistance',
                    cubicInterpolationMode: 'monotone',
                    data: resistance,
                    fill: false,
                    pointRadius: 0,
                    backgroundColor: window.chartColors.red,
                    borderColor: window.chartColors.red,
                },
                {
                    label: 'Peloton R.',
                    cubicInterpolationMode: 'monotone',
                    data: peloton_resistance,
                    fill: false,
                    pointRadius: 0,
                    backgroundColor: window.chartColors.black,
                    borderColor: window.chartColors.black,
                },
                {
                    backgroundColor: window.chartColors.blue,
                    borderColor: window.chartColors.blue,
                    label: 'Cadence',
                    cubicInterpolationMode: 'monotone',
                    data: cadence,
                    fill: false,
                    pointRadius: 0,
                },
            ]
        },
        options: {
            animation: {
              onComplete: function() {
                  let el = new MainWSQueueElement({
                      msg: 'savechart',
                      content: {
                          name: 'resistance',
                          image: powerChart.toBase64Image()
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
                    text:'Resistance vs Peloton Resistance'
                },
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                legend: {
                    display: true,
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
                        display: true,
                        text: 'Time'
                    },
                    ticks: {
                        // Include a dollar sign in the ticks
                        callback: function(value, index, values) {
                            return Math.floor(value / 3600).toString().padStart(2, "0") + ":" + Math.floor(value / 60).toString().padStart(2, "0");
                        }
                    }
                },
                y: {
                    display: true,
                    title: {
                        display: true,
                        text: 'Heart rate'
                    },
                }
            }
        }
    };

    ctx = document.getElementById('canvasResistance').getContext('2d');
    var resistanceChart = new Chart(ctx, config);
}

function dochart_init() {
    onSettingsOK = true;
    keys_arr = ['ftp', 'age', 'heart_rate_zone1', 'heart_rate_zone2', 'heart_rate_zone3', 'heart_rate_zone4']
    let el = new MainWSQueueElement({
            msg: 'getsettings',
            content: {
                keys: keys_arr
            }
        }, function(msg) {
            if (msg.msg === 'R_getsettings') {
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
                    } else if (key === 'heart_rate_zone1') {
                        heartZones[0] = Math.round(maxHeartRate * msg.content[key]);
                    } else if (key === 'heart_rate_zone2') {
                        heartZones[1] = Math.round(maxHeartRate * msg.content[key]);
                    } else if (key === 'heart_rate_zone3') {
                        heartZones[2] = Math.round(maxHeartRate * msg.content[key]);
                    } else if (key === 'heart_rate_zone4') {
                        heartZones[3] = Math.round(maxHeartRate * msg.content[key]);
                    }
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
}


$(window).on('load', function () {
    dochart_init();return;

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

    arr = [{'watts': 100, 'req_power': 150, 'elapsed_s':0,'elapsed_m':0,'elapsed_h':0, 'heart':90, 'resistance': 10, 'peloton_resistance': 15, 'cadence': 80},
           {'watts': 120, 'req_power': 150, 'elapsed_s':1,'elapsed_m':1,'elapsed_h':0, 'heart':92, 'resistance': 11, 'peloton_resistance': 30, 'cadence': 90},
           {'watts': 130, 'req_power': 170, 'elapsed_s':2,'elapsed_m':2,'elapsed_h':0, 'heart':110, 'resistance': 12, 'peloton_resistance': 40, 'cadence': 100},
           {'watts': 140, 'req_power': 170, 'elapsed_s':3,'elapsed_m':3,'elapsed_h':0, 'heart':115, 'resistance': 16, 'peloton_resistance': 41, 'cadence': 90},
           {'watts': 130, 'req_power': 170, 'elapsed_s':4,'elapsed_m':4,'elapsed_h':0, 'heart':130, 'resistance': 18, 'peloton_resistance': 43, 'cadence': 95},
           {'watts': 160, 'req_power': 170, 'elapsed_s':5,'elapsed_m':5,'elapsed_h':0, 'heart':135, 'resistance': 22, 'peloton_resistance': 43, 'cadence': 95},
           {'watts': 180, 'req_power': 130, 'elapsed_s':6,'elapsed_m':6,'elapsed_h':0, 'heart':140, 'resistance': 31, 'peloton_resistance': 43, 'cadence': 95},
           {'watts': 120, 'req_power': 130, 'elapsed_s':7,'elapsed_m':7,'elapsed_h':0, 'heart':150, 'resistance': 18, 'peloton_resistance': 35, 'cadence': 95},
           {'watts': 190, 'req_power': 150, 'elapsed_s':1,'elapsed_m':8,'elapsed_h':0, 'heart':155, 'resistance': 17, 'peloton_resistance': 35, 'cadence': 95},
           {'watts': 195, 'req_power': 170, 'elapsed_s':2,'elapsed_m':9,'elapsed_h':0, 'heart':165, 'resistance': 19, 'peloton_resistance': 30, 'cadence': 80},
           {'watts': 200, 'req_power': 170, 'elapsed_s':3,'elapsed_m':10,'elapsed_h':0, 'heart':153, 'resistance': 20, 'peloton_resistance': 25, 'cadence': 90},
           {'watts': 206, 'req_power': 170, 'elapsed_s':4,'elapsed_m':11,'elapsed_h':0, 'heart':152, 'resistance': 21, 'peloton_resistance': 35, 'cadence': 90},
           {'watts': 211, 'req_power': 170, 'elapsed_s':5,'elapsed_m':12,'elapsed_h':0, 'heart':180, 'resistance': 25, 'peloton_resistance': 35, 'cadence': 90},
           {'watts': 222, 'req_power': 130, 'elapsed_s':6,'elapsed_m':13,'elapsed_h':0, 'heart':182, 'resistance': 31, 'peloton_resistance': 35, 'cadence': 80},
           {'watts': 237, 'req_power': 130, 'elapsed_s':7,'elapsed_m':14,'elapsed_h':0, 'heart':160, 'resistance': 20, 'peloton_resistance': 50, 'cadence': 90},
           {'watts': 250, 'req_power': 170, 'elapsed_s':3,'elapsed_m':15,'elapsed_h':0, 'heart':115, 'resistance': 20, 'peloton_resistance': 50, 'cadence': 90},
           {'watts': 266, 'req_power': 170, 'elapsed_s':4,'elapsed_m':16,'elapsed_h':0, 'heart':120, 'resistance': 11, 'peloton_resistance': 35, 'cadence': 80},
           {'watts': 271, 'req_power': 170, 'elapsed_s':5,'elapsed_m':17,'elapsed_h':0, 'heart':112, 'resistance': 22, 'peloton_resistance': 23, 'cadence': 80},
           {'watts': 262, 'req_power': 130, 'elapsed_s':6,'elapsed_m':18,'elapsed_h':0, 'heart':90, 'resistance': 25, 'peloton_resistance': 23, 'cadence': 80},
           {'watts': 257, 'req_power': 130, 'elapsed_s':7,'elapsed_m':19,'elapsed_h':0, 'heart':120, 'resistance': 10, 'peloton_resistance': 23, 'cadence': 80},
            ]
    process_arr(arr);
});
