import QtQuick 2.4

ChartsEndWorkoutForm {
    Timer {
        id: timer

        // Start the timer and execute the provided callback on every X milliseconds
        function startTimer(callback, milliseconds) {
            timer.interval = milliseconds;
            timer.repeat = false;
            timer.triggered.connect(callback);
            timer.start();
        }

        // Stop the timer and unregister the callback
        function stopTimer(callback) {
            timer.stop();
            timer.triggered.disconnect(callback);
        }
    }

    function saveScreenshot()
    {
        rootItem.save_screenshot_chart(powerChart, "powerChart");
        rootItem.save_screenshot_chart(heartChart, "heartChart");
        rootItem.save_screenshot_chart(cadenceChart, "cadenceChart");
        timer.stopTimer(saveScreenshot)
        timer.startTimer(sendMail, 100);
    }

    function sendMail()
    {
        rootItem.sendMail()
        timer.stopTimer(sendMail)
    }

    Component.onCompleted: {
        headerToolbar.visible = true;

        //console.log("ChartsEndWorkoutForm completed " + rootItem.workout_sample_points)
        for(var i=0;i<rootItem.workout_sample_points;i+=10)
        {
            //console.log("ChartsEndWorkoutForm completed " + i + " " + rootItem.workout_watt_points[i])
            powerSeries.append(i * 1000, rootItem.workout_watt_points[i]);
            heartSeries.append(i * 1000, rootItem.workout_heart_points[i]);
            cadenceSeries.append(i * 1000, rootItem.workout_cadence_points[i]);
            resistanceSeries.append(i * 1000, rootItem.workout_resistance_points[i]);
            pelotonResistanceSeries.append(i * 1000, rootItem.workout_peloton_resistance_points[i]);
        }
        rootItem.update_chart_power(powerChart);
        //rootItem.update_axes(valueAxisX, valueAxisY);
        rootItem.update_chart_heart(heartChart);
        //rootItem.update_axes(valueAxisXHR, valueAxisYHR);
        //rootItem.update_chart(cadenceChart);
        //rootItem.update_axes(valueAxisXCadence, valueAxisYCadence);

        timer.startTimer(saveScreenshot, 100);
    }
}
