let socket = new WebSocket("ws://" + location.host);
socket.onopen = function (event) {
    console.log("Upgrade HTTP connection OK");
};
let pad = function(num, size) {
    num = num.toString();
    while (num.length < size) num = "0" + num;
    return num;
}
let getClass = function(workout, field) {
    let v1 = Math.floor(workout[field] * 10);
    let v2 = Math.floor(workout[field+'_avg'] * 10);
    let diff = v1-v2;
    let absdiff = Math.abs(diff);
    if (absdiff< 10) {
        return "normal";
    }
    else if (diff > 0) {
        return "ok";
    }
    else {
        return "min";
    }
}
socket.onmessage = function (event) {
    console.log(event.data);
    let msg = JSON.parse(event.data);
    if (msg.msg === "workout") {
        let workout = msg.content;
        if (!msg.content.deviceId) {
            $("#spntimev").attr('class', 'err').text('--:--:--');
            $("#spndistancev").attr('class', 'err').text('---');
            $("#spnwattv").attr('class', 'err').text('---');
            $("#spnwattavgv").attr('class', 'err').text('---');
            $("#spncaloriev").attr('class', 'err').text('---');
            $("#spnrpmv").attr('class', 'err').text('---');
            $("#spnrpmavgv").attr('class', 'err').text('---');
            $("#spnheartv").attr('class', 'err').text('---');
            $("#spnheartavgv").attr('class', 'err').text('---');
            $("#spnspeedv").attr('class', 'err').text('---');
            $("#spnspeedavgv").attr('class', 'err').text('---');
        }
        else {
            $("#spntimev").attr('class', 'normal').text(workout.elapsed_h + ':' + pad(workout.elapsed_m, 2) + ':'  + pad(workout.elapsed_s, 2));
            $("#spndistancev").attr('class', 'normal').text(workout.distance.toFixed(2));
            $("#spnwattv").attr('class', getClass(workout, 'watts')).text(workout.watts.toFixed(0));
            $("#spnwattavgv").attr('class', 'normal').text(workout.watts_avg.toFixed(0));
            $("#spncaloriev").attr('class', 'normal').text(workout.calories + '');
            if (workout['cadence']) {
                $("#spnrpmv").attr('class', getClass(workout, 'cadence')).text(workout.cadence.toFixed(0));
                $("#spnrpmavgv").attr('class', 'normal').text(workout.cadence_avg.toFixed(0));
            }
            else {
                $("#spnrpmv").attr('class', 'err').text('---');
                $("#spnrpmavgv").attr('class', 'err').text('---');
            }

            $("#spnheartv").attr('class', 'normal').text(workout.heart);
            $("#spnheartavgv").attr('class', 'normal').text(workout.heart_avg);
            $("#spnspeedv").attr('class', getClass(workout, 'speed')).text(workout.speed.toFixed(1));
            $("#spnspeedavgv").attr('class', 'normal').text(workout.speed_avg.toFixed(1));
        }
    }
}
