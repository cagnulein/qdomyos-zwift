window.chartColors = {
    red: 'rgb(255, 99, 132)',
    orange: 'rgb(255, 159, 64)',
    yellow: 'rgb(255, 205, 86)',
    green: 'rgb(75, 192, 192)',
    blue: 'rgb(54, 162, 235)',
    purple: 'rgb(153, 102, 255)',
    grey: 'rgb(201, 203, 207)'
};

function process_arr(arr) {
    let watts = [];
    for (let el of arr) {
        let wattel = {};
        wattel.x = el.elapsed_s + el.elapsed_m * 60 + el.elapsed_h * 3600;
        wattel.y = el.watts;
        watts.push(wattel);
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
                   borderColor: ctx => ctx.p0.parsed.y < 30 ? 'rgb(192,75,75)' : '',
                }
            }]
        },
        options: {
            responsive: true,
            grid: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            plugins: {
                title:{
                    display:true,
                    text:'Chart.js Line Chart'
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
