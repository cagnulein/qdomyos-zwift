(function () {
    const originalProcessArr = window.process_arr;

    if (typeof originalProcessArr !== 'function') {
        console.error('treadmill_summary.js: process_arr is not available');
        return;
    }

    function elapsedSeconds(sample) {
        return Number(sample.elapsed_s || 0) + Number(sample.elapsed_m || 0) * 60 + Number(sample.elapsed_h || 0) * 3600;
    }

    function finiteNumber(value, fallback) {
        const parsed = Number(value);
        return Number.isFinite(parsed) ? parsed : fallback;
    }

    function formatPace(secondsPerUnit, unit) {
        if (!Number.isFinite(secondsPerUnit) || secondsPerUnit <= 0) {
            return '--:-- /' + unit;
        }

        const rounded = Math.round(secondsPerUnit);
        const minutes = Math.floor(rounded / 60);
        const seconds = rounded % 60;
        return minutes + ':' + String(seconds).padStart(2, '0') + ' /' + unit;
    }

    function setSummaryLabel(valueSelector, column, text) {
        const valueCell = $(valueSelector);
        const labelCell = valueCell.closest('tr').prev('tr').find('td').eq(column);
        labelCell.removeAttr('data-i18n').text(text);
    }

    function updateTreadmillSummary(arr) {
        if (!Array.isArray(arr) || arr.length === 0) {
            return;
        }

        const last = arr[arr.length - 1];
        if (Number(last.deviceType) !== 1) {
            return;
        }

        const distanceKm = Math.max(0, finiteNumber(last.distance, 0));
        const unitFactor = Number.isFinite(Number(window.miles)) ? Number(window.miles) : 1;
        const displayDistance = distanceKm * unitFactor;
        const paceUnit = unitFactor === 1 ? 'km' : 'mi';
        const elevationUnit = unitFactor === 1 ? 'm' : 'ft';
        const lastElapsed = elapsedSeconds(last);
        const paceSeconds = displayDistance > 0 ? lastElapsed / displayDistance : 0;

        let weightedIncline = 0;
        let inclineWeight = 0;
        let elevationGainMeters = 0;

        for (let i = 1; i < arr.length; i++) {
            const previous = arr[i - 1];
            const current = arr[i];
            const previousDistance = finiteNumber(previous.distance, 0);
            const currentDistance = finiteNumber(current.distance, previousDistance);
            const deltaDistanceKm = currentDistance - previousDistance;

            if (!Number.isFinite(deltaDistanceKm) || deltaDistanceKm <= 0) {
                continue;
            }

            const previousIncline = finiteNumber(previous.inclination, 0);
            const currentIncline = finiteNumber(current.inclination, previousIncline);
            const averageIncline = (previousIncline + currentIncline) / 2;

            weightedIncline += averageIncline * deltaDistanceKm;
            inclineWeight += deltaDistanceKm;

            if (averageIncline > 0) {
                const grade = averageIncline / 100;
                const segmentMeters = deltaDistanceKm * 1000;
                elevationGainMeters += segmentMeters * Math.sin(Math.atan(grade));
            }
        }

        let averageIncline = 0;
        if (inclineWeight > 0) {
            averageIncline = weightedIncline / inclineWeight;
        } else {
            let weightedByTime = 0;
            let timeWeight = 0;
            for (let i = 1; i < arr.length; i++) {
                const previous = arr[i - 1];
                const current = arr[i];
                const deltaTime = elapsedSeconds(current) - elapsedSeconds(previous);
                if (!Number.isFinite(deltaTime) || deltaTime <= 0) {
                    continue;
                }
                const previousIncline = finiteNumber(previous.inclination, 0);
                const currentIncline = finiteNumber(current.inclination, previousIncline);
                weightedByTime += ((previousIncline + currentIncline) / 2) * deltaTime;
                timeWeight += deltaTime;
            }
            if (timeWeight > 0) {
                averageIncline = weightedByTime / timeWeight;
            }
        }

        const heartAverage = Math.max(0, finiteNumber(last.heart_avg, 0));
        const displayElevation = unitFactor === 1 ? elevationGainMeters : elevationGainMeters * 3.28084;

        setSummaryLabel('.summary_watts_avg', 0, t('chart.avgPace', 'Avg Pace'));
        setSummaryLabel('.summary_jouls', 1, t('chart.avgIncline', 'Avg Incline'));
        setSummaryLabel('.summary_cadence_avg', 0, t('chart.elevationGain', 'Elevation Gain'));
        setSummaryLabel('.summary_resistance_avg', 1, t('chart.avgHeartRate', 'Avg Heart Rate'));

        $('.summary_watts_avg').text(formatPace(paceSeconds, paceUnit));
        $('.summary_jouls').text(averageIncline.toFixed(1) + ' %');
        $('.summary_cadence_avg').text(Math.round(displayElevation) + ' ' + elevationUnit);
        $('.summary_resistance_avg').text(Math.round(heartAverage) + ' bpm');
    }

    window.process_arr = function (arr) {
        originalProcessArr(arr);
        updateTreadmillSummary(arr);
    };
})();
