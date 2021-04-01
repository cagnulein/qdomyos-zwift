import QtQuick 2.4

ChartsEndWorkoutForm {
    Component.onCompleted: {
        //console.log("ChartsEndWorkoutForm completed " + rootItem.workout_sample_points)
        for(var i=0;i<rootItem.workout_sample_points;i++)
        {
            //console.log("ChartsEndWorkoutForm completed " + i + " " + rootItem.workout_watt_points[i])
            powerSeries.append(i, rootItem.workout_watt_points[i]);
            heartSeries.append(i, rootItem.workout_heart_points[i]);
            cadenceSeries.append(i, rootItem.workout_cadence_points[i]);
        }
        rootItem.update_chart(powerChart);
        //rootItem.update_axes(valueAxisX, valueAxisY);
        rootItem.update_chart(heartChart);
        //rootItem.update_axes(valueAxisXHR, valueAxisYHR);
        rootItem.update_chart(cadenceChart);
        //rootItem.update_axes(valueAxisXCadence, valueAxisYCadence);
    }
}
