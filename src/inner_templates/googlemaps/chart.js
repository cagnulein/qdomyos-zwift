window.chartColors = {
    red: 'rgb(255, 29, 0)',
    redt: 'rgb(255, 29, 0, 0.25)',
    orange: 'rgb(255, 159, 64)',
    oranget: 'rgb(255, 159, 64, 0.25)',
    darkorange: 'rgb(255, 140, 0)',
    darkoranget: 'rgb(255, 140, 0, 0.25)',
    orangered: 'rgb(255, 69, 0)',
    orangeredt: 'rgb(255, 69, 0, 0.25)',
    yellow: 'rgb(255, 205, 86)',
    yellowt: 'rgb(255, 205, 86, 0.25)',
    green: 'rgb(75, 192, 192)',
    greent: 'rgb(75, 192, 192, 0.25)',
    blue: 'rgb(54, 162, 235)',
    purple: 'rgb(153, 102, 255)',
    grey: 'rgb(201, 203, 207)',
    greyt: 'rgb(201, 203, 207, 0.25)',
    white: 'rgb(255, 255, 255)',
    whitet: 'rgb(255, 255, 255, 0.25)',
    limegreen: 'rgb(50, 205, 50)',
    limegreent: 'rgb(50, 205, 50, 0.25)',
    gold: 'rgb(255, 215, 0)',
    goldt: 'rgb(255, 215, 0, 0.25)',
    black: 'rgb(0, 0, 0)',
    blackt: 'rgb(0, 0, 0, 0.25)',
    lightsteelblue: 'rgb(176,192,222)',
    lightsteelbluet: 'rgb(176,192,222, 0.25)',
};

var inclinationArray = []

const range = ({from = 0, to, step = 1, length = Math.ceil((to - from) / step)}) =>
  Array.from({length}, (_, i) => from + i * step)

let canvasChart = {}

$(window).on('load', function () {
const data = {
  labels: range({to: 300}),
  datasets: [{
    label: '',
    data: inclinationArray,
    fill: true,
    tension: 0.2,
    pointRadius: 0,
                segment: {
                    borderColor: ctx => ctx.p1.parsed.y - ctx.p0.parsed.y < 0 ? window.chartColors.green:
                                                                        ctx.p1.parsed.y - ctx.p0.parsed.y < 3 ? window.chartColors.limegreen :
                                                                        ctx.p1.parsed.y - ctx.p0.parsed.y< 5 ? window.chartColors.gold :
                                                                        ctx.p1.parsed.y - ctx.p0.parsed.y < 7 ? window.chartColors.orange :
                                                                        ctx.p1.parsed.y - ctx.p0.parsed.y < 9 ? window.chartColors.darkorange :
                                                                        ctx.p1.parsed.y - ctx.p0.parsed.y < 10 ? window.chartColors.orangered :
                                                                        window.chartColors.red,
                    backgroundColor: ctx => ctx.p1.parsed.y - ctx.p0.parsed.y < 0 ? window.chartColors.green:
                                                                        ctx.p1.parsed.y - ctx.p0.parsed.y < 3 ? window.chartColors.limegreen :
                                                                        ctx.p1.parsed.y - ctx.p0.parsed.y< 5 ? window.chartColors.gold :
                                                                        ctx.p1.parsed.y - ctx.p0.parsed.y < 7 ? window.chartColors.orange :
                                                                        ctx.p1.parsed.y - ctx.p0.parsed.y < 9 ? window.chartColors.darkorange :
                                                                        ctx.p1.parsed.y - ctx.p0.parsed.y < 10 ? window.chartColors.orangered :
                                                                        window.chartColors.red,
                }
  }]
};

let config = {
    type: 'line',
    data: data,
        options: {
            plugins: {
                legend : {
                    display: false,
                }
            },

            maintainAspectRatio: false,
            responsive: true,
            scales: {
              x: {                    
                ticks: {
                  callback: () => ('')
                },
                    grid: {
                               display: false,
                               drawOnChartArea:false,
                               drawBorder: false, // <-- this removes y-axis line
                             }
              },
                y: {
                    min: -3000,
                    max: 3000,
                    ticks: {
                    callback: () => ('')
                  },
                    grid: {
                               display: false,
                               drawOnChartArea:false,
                               drawBorder: false, // <-- this removes y-axis line
                             }
                },
            }
          }
};

let ctx = document.getElementById('canvasChart').getContext('2d');
canvasChart = new Chart(ctx, config);
    canvasChart.canvas.parentNode.style.height = '150px';
    canvasChart.canvas.parentNode.style.width = '150px';
    setTimeout(chartRefresh, 500);
});

function chartRefresh() {
onWorkout = false;
let el = new MainWSQueueElement({
    msg: 'getnextinclination'
}, function(msg) {
    if (msg.msg === 'R_getnextinclination') {
        return msg.content;
    }
    return null;
}, 15000, 3);
el.enqueue().then(process_nextinclination).catch(function(err) {
    console.error('Error is ' + err);
});
}

function process_nextinclination(msg) {
    let arr = msg.split(",");
    let lastValue = 0
    inclinationArray = [];
    for(var i=0; i<arr.length/2.0;i++) {
        for(var ii=0; ii<parseFloat(arr[i*2]);ii++) {
            lastValue += parseFloat(arr[(i*2)+1])
            inclinationArray.push(lastValue);
        }
    }
    canvasChart.data.datasets[0].data = inclinationArray;
    canvasChart.update();
    setTimeout(chartRefresh, 500);
}
