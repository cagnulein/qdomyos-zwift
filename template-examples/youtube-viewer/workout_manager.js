let getClass = function(workout, field) {
    let v1 = Math.floor(workout[field] * 10);
    let v2 = Math.floor(workout[field+'_avg'] * 10);
    let diff = v1-v2;
    let absdiff = Math.abs(diff);
    if (absdiff == 0) {
        return 'normal';
    }
    else if (diff > 0) {
        return 'ok';
    }
    else {
        return 'min';
    }
};
function workout_queue_element_build() {
    return new MainWSQueueElement(null, function(msg) {
        return msg.msg === 'workout'?msg.content:null;
    }, 5000, 1);
}

function workout_msg_process(workout) {
    if (!workout.deviceId) {
        $('#spntimev').attr('class', 'err').text('--:--:--');
        $('#spndistancev').attr('class', 'err').text('---');
        $('#spnwattv').attr('class', 'err').text('---');
        $('#spnwattavgv').attr('class', 'err').text('---');
        $('#spncaloriev').attr('class', 'err').text('---');
        $('#spnrpmv').attr('class', 'err').text('---');
        $('#spnrpmavgv').attr('class', 'err').text('---');
        $('#spnheartv').attr('class', 'err').text('---');
        $('#spnheartavgv').attr('class', 'err').text('---');
        $('#spnspeedv').attr('class', 'err').text('---');
        $('#spnspeedavgv').attr('class', 'err').text('---');
        $('#spninclinev').attr('class', 'err').text('---');
    }
    else {
        $('#spntimev').attr('class', 'normal').text(workout.elapsed_h + ':' + pad(workout.elapsed_m, 2) + ':'  + pad(workout.elapsed_s, 2));
        $('#spndistancev').attr('class', 'normal').text(workout.distance.toFixed(2));
        $('#spnwattv').attr('class', getClass(workout, 'watts')).text(workout.watts.toFixed(0));
        $('#spnwattavgv').attr('class', 'normalavg').text(workout.watts_avg.toFixed(0));
        $('#spncaloriev').attr('class', 'normal').text(workout.calories.toFixed(0));
        $('#spninclinev').attr('class', 'normal').text(workout.resistance.toFixed(0));
        if (workout.cadence) {
            $('#spnrpmv').attr('class', getClass(workout, 'cadence')).text(workout.cadence.toFixed(0));
            $('#spnrpmavgv').attr('class', 'normalavg').text(workout.cadence_avg.toFixed(0));
        }
        else {
            $('#spnrpmv').attr('class', 'err').text('---');
            $('#spnrpmavgv').attr('class', 'err').text('---');
        }

        $('#spnheartv').attr('class', 'normal').text(workout.heart);
        $('#spnheartavgv').attr('class', 'normalavg').text(workout.heart_avg.toFixed(0));
        $('#spnspeedv').attr('class', getClass(workout, 'speed')).text(workout.speed.toFixed(1));
        $('#spnspeedavgv').attr('class', 'normalavg').text(workout.speed_avg.toFixed(1));
    }
}

function workout_manager_init() {
    let el = workout_queue_element_build();
    el.enqueue().then(function(content) {
        workout_msg_process(content);
        workout_manager_init();
    })
        .catch(function(err) {
            console.error(err);
            workout_msg_process({});
            workout_manager_init();
        });
}

workout_manager_init();
