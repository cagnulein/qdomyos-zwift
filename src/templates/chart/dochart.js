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
};

var ftp = 200;
var ftpZones = [];

function process_arr(arr) {
    let watts = [];
    let reqpower = [];
    for (let el of arr) {
        let wattel = {};
        let reqpowerel = {};
        wattel.x = el.elapsed_s + el.elapsed_m * 60 + el.elapsed_h * 3600;
        wattel.y = el.watts;
        watts.push(wattel);
        reqpowerel.x = el.elapsed_s + el.elapsed_m * 60 + el.elapsed_h * 3600;
        reqpowerel.y = el.req_power;
        reqpower.push(reqpowerel);
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
                segment: {
                   borderColor: ctx => ctx.p0.parsed.y < ftpZones[0] ? window.chartColors.white :
                                                                       ctx.p0.parsed.y < ftpZones[1] ? window.chartColors.limegreen :
                                                                       ctx.p0.parsed.y < ftpZones[2] ? window.chartColors.gold :
                                                                       ctx.p0.parsed.y < ftpZones[3] ? window.chartColors.orange :
                                                                       ctx.p0.parsed.y < ftpZones[4] ? window.chartColors.darkorange :
                                                                       ctx.p0.parsed.y < ftpZones[5] ? window.chartColors.orangered :
                                                                       window.chartColors.red,
                }
            }, {
                label: 'Req. Watts',
                backgroundColor: window.chartColors.red,
                borderColor: window.chartColors.black,
                borderWidth: 5,
                cubicInterpolationMode: 'monotone',
                data: reqpower,
                fill: false,
            },
            ]
        },
        options: {
            responsive: true,
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
                        callback: value => [ftpZones[0], ftpZones[1], ftpZones[2], ftpZones[3], ftpZones[4], ftpZones[5]].includes(value) ?
                            value === ftpZones[0] ? 'zone 1' :
                            value === ftpZones[1] ? 'zone 2' :
                            value === ftpZones[2] ? 'zone 3' :
                            value === ftpZones[3] ? 'zone 4' :
                            value === ftpZones[4] ? 'zone 5' : undefined : undefined,
                        color: 'black',
                    }
                }
            }
        }
    };

    let ctx = document.getElementById('canvas').getContext('2d');
    new Chart(ctx, config);
}

function dochart_init() {
    onSettingsOK = true;
    keys_arr = ['ftp', 'heart_rate_zone1', 'heart_rate_zone2', 'heart_rate_zone3', 'heart_rate_zone4']
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
    dochart_init();
});
