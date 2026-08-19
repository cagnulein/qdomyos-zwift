(function () {
    const originalProcessArr = window.process_arr;
    let treadmillThumbnailTimer = null;

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

    function isTreadmillWorkout(arr) {
        return Array.isArray(arr) && arr.length > 0 && Number(arr[arr.length - 1].deviceType) === 1;
    }

    function deriveTreadmillSpeedPoints(arr) {
        let lastSpeed = 0;
        const points = [];

        for (let i = 0; i < arr.length; i++) {
            const sample = arr[i];
            const time = elapsedSeconds(sample);
            const reportedSpeed = finiteNumber(sample.speed, 0);

            if (reportedSpeed > 0) {
                lastSpeed = reportedSpeed;
            } else if (i > 0) {
                const previous = arr[i - 1];
                const deltaSeconds = time - elapsedSeconds(previous);
                const deltaDistanceKm = finiteNumber(sample.distance, 0) - finiteNumber(previous.distance, 0);

                if (deltaSeconds > 0 && deltaDistanceKm > 0) {
                    const derivedSpeed = deltaDistanceKm * 3600 / deltaSeconds;
                    if (Number.isFinite(derivedSpeed) && derivedSpeed >= 0 && derivedSpeed < 100) {
                        lastSpeed = derivedSpeed;
                    }
                }
            }

            points.push({
                x: time,
                y: lastSpeed * window.miles
            });
        }

        return points;
    }

    function updateTreadmillSpeedInclinationChart(arr) {
        if (!isTreadmillWorkout(arr) || typeof Chart === 'undefined' || typeof Chart.getChart !== 'function') {
            return;
        }

        const canvas = document.getElementById('canvasSpeedInclination');
        if (!canvas) {
            return;
        }

        const chart = Chart.getChart(canvas);
        if (!chart || !chart.data || !chart.data.datasets || chart.data.datasets.length < 2) {
            return;
        }

        const speedPoints = deriveTreadmillSpeedPoints(arr);
        const inclinationPoints = arr.map(function(sample) {
            return {
                x: elapsedSeconds(sample),
                y: finiteNumber(sample.inclination, 0)
            };
        });

        chart.data.datasets[0].data = speedPoints;
        chart.data.datasets[1].data = inclinationPoints;

        const lastElapsed = elapsedSeconds(arr[arr.length - 1]);
        if (chart.options && chart.options.scales && chart.options.scales.x) {
            chart.options.scales.x.max = lastElapsed;
        }

        chart.update('none');
    }

    function setSummaryLabel(valueSelector, column, text) {
        const valueCell = $(valueSelector);
        const labelCell = valueCell.closest('tr').prev('tr').find('td').eq(column);
        labelCell.removeAttr('data-i18n').text(text);
    }

    function updateTreadmillSummary(arr) {
        if (!isTreadmillWorkout(arr)) {
            return;
        }

        const last = arr[arr.length - 1];
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

    function hideFollowingBreaks(element) {
        let next = element.next();
        while (next.length && next.is('br')) {
            next.hide();
            next = next.next();
        }
    }

    function hideChartSection(canvasSelector) {
        const canvas = $(canvasSelector);
        if (!canvas.length) {
            return;
        }

        const container = canvas.parent();
        container.hide();
        hideFollowingBreaks(container);
    }

    function configureTreadmillCharts() {
        const powerContainer = $('#canvas').parent();
        const wattsStats = $('.watts_avg').parent();

        powerContainer.hide();
        wattsStats.hide();
        hideFollowingBreaks(wattsStats);

        hideChartSection('#canvasResistance');
        hideChartSection('#canvasPelotonResistance');
        hideChartSection('#canvasCadence');
        hideChartSection('#canvasPowerDistribution');

        const speedContainer = $('#canvasSpeedInclination').parent();
        if (speedContainer.length) {
            let previous = speedContainer.prev();
            while (previous.length && previous.is('br')) {
                previous.hide();
                previous = previous.prev();
            }

            speedContainer.show().appendTo('#watt_badge');
        }

        $('#canvasHeart').parent().show();
        $('.heart_avg').parent().show();
    }

    function saveChartImage(name, image) {
        if (!image) {
            return;
        }

        const element = new MainWSQueueElement({
            msg: 'savechart',
            content: {
                name: name,
                image: image
            }
        }, function(msg) {
            if (msg.msg === 'R_savechart') {
                return msg.content;
            }
            return null;
        }, 15000, 3);

        element.enqueue().catch(function(err) {
            console.error('treadmill_summary.js: error saving ' + name + ': ' + err);
        });
    }

    function saveTreadmillThumbnails() {
        const speedCanvas = document.getElementById('canvasSpeedInclination');
        if (speedCanvas && typeof speedCanvas.toDataURL === 'function') {
            try {
                saveChartImage('power', speedCanvas.toDataURL('image/png'));
            } catch (err) {
                console.error('treadmill_summary.js: unable to export speed/inclination chart: ' + err);
            }
        }

        const badge = document.getElementById('watt_badge');
        if (badge && typeof html2canvas === 'function') {
            html2canvas(badge).then(function(canvas) {
                saveChartImage('power_badge', canvas.toDataURL('image/png'));
            }).catch(function(err) {
                console.error('treadmill_summary.js: unable to export treadmill badge: ' + err);
            });
        }
    }

    function adaptTreadmillPresentation(arr) {
        if (!isTreadmillWorkout(arr)) {
            return;
        }

        updateTreadmillSummary(arr);
        configureTreadmillCharts();
        updateTreadmillSpeedInclinationChart(arr);

        if (treadmillThumbnailTimer !== null) {
            clearTimeout(treadmillThumbnailTimer);
        }
        treadmillThumbnailTimer = setTimeout(saveTreadmillThumbnails, 1800);
    }

    window.process_arr = function (arr) {
        // The stock chart already uses elapsed seconds on a linear x-axis. Keep the
        // original session data untouched, then patch only the treadmill chart data.
        originalProcessArr(arr);
        adaptTreadmillPresentation(arr);
    };
})();
