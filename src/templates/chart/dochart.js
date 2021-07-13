
function process_arr(arr) {
    let watts = [];
    for (let el of arr) {
        let wattel = {};
        wattel.x = el.elapsed_s + el.elapsed_m * 60 + el.elapsed_h * 3600;
        wattel.y = el.watts;
        watts.push(wattel);
    }
    var config = {
        type: 'line',
        data: {
            datasets: [{
                label: 'Watts',
                backgroundColor: window.chartColors.red,
                borderColor: window.chartColors.red,
                data: watts,
                fill: false,
            }]
        },
        options: {
            responsive: true,
            gridLines: {
                zeroLineColor: 'rgba(0,255,0,1)'
            },
            title:{
                display:true,
                text:'Chart.js Line Chart'
            },
            tooltips: {
                mode: 'index',
                intersect: false,
            },
            hover: {
                mode: 'nearest',
                intersect: true
            },
            scales: {
                xAxes: [{
                    display: true,
                    scaleLabel: {
                        display: true,
                        labelString: 'Seconds'
                    }
                }],
                yAxes: [{
                    display: true,
                    scaleLabel: {
                        display: true,
                        labelString: 'Watt'
                    }
                }]
            }
        }
    };

    let ctx = document.getElementById('canvas').getContext('2d');
    new Chart(ctx, config);
}

function dochart_init() {
    let el = new MainWSQueueElement({
        msg: 'getsessionarray'
    }, function(msg) {
        if (msg.msg === 'R_getsessionarray') {
            return msg.content;
        }
        return null;
    }, 5000, 3);
    el.enqueue().then(process_arr).catch(function(err) {
        console.error('Error is ' + err);
    });
}


$(window).on('load', function () {
    dochart_init();
});