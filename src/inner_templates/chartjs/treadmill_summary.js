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

    function formatElapsedTick(value) {
        const totalSeconds = Math.max(0, Math.round(finiteNumber(value, 0)));
        const hours = Math.floor(totalSeconds / 3600);
        const minutes = Math.floor((totalSeconds % 3600) / 60);
        const seconds = totalSeconds % 60;

        if (hours > 0) {
            return hours + ':' + String(minutes).padStart(2, '0') + ':' + String(seconds).padStart(2, '0');
        }
        return String(minutes).padStart(2, '0') + ':' + String(seconds).padStart(2, '0');
    }

    function isTreadmillWorkout(arr) {
        return Array.isArray(arr) && arr.length > 0 && Number(arr[arr.length - 1].deviceType) === 1;
    }

    function deriveTreadmillSpeedPoints(arr) {
        let lastSpeed = 0;
        const points = [];
        const unitFactor = Number.isFinite(Number(window.miles)) ? Number(window.miles) : 1;

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
                y: lastSpeed * unitFactor
            });
        }

        return points;
    }

    function buildTreadmillChartData(arr) {
        return {
            speedPoints: deriveTreadmillSpeedPoints(arr),
            inclinationPoints: arr.map(function(sample) {
                return {
                    x: elapsedSeconds(sample),
                    y: finiteNumber(sample.inclination, 0)
                };
            }),
            lastElapsed: arr.length ? elapsedSeconds(arr[arr.length - 1]) : 0
        };
    }

    function getRawXScaleOptions(chart) {
        if (chart && chart.config && chart.config.options && chart.config.options.scales && chart.config.options.scales.x) {
            return chart.config.options.scales.x;
        }
        if (chart && chart.options && chart.options.scales && chart.options.scales.x) {
            return chart.options.scales.x;
        }
        return null;
    }

    function updateTreadmillSpeedInclinationChart(arr) {
        const diagnostics = {
            treadmill: isTreadmillWorkout(arr),
            canvasFound: false,
            chartFound: false,
            durationSeconds: arr && arr.length ? elapsedSeconds(arr[arr.length - 1]) : 0,
            speedPoints: [],
            inclinationPoints: [],
            xMax: null,
            tick10: null,
            tick20: null,
            valid: false
        };

        if (!diagnostics.treadmill || typeof Chart === 'undefined' || typeof Chart.getChart !== 'function') {
            window.qzTreadmillSummaryDiagnostics = diagnostics;
            return diagnostics;
        }

        const canvas = document.getElementById('canvasSpeedInclination');
        diagnostics.canvasFound = Boolean(canvas);
        if (!canvas) {
            window.qzTreadmillSummaryDiagnostics = diagnostics;
            return diagnostics;
        }

        const chart = Chart.getChart(canvas);
        diagnostics.chartFound = Boolean(chart);
        if (!chart || !chart.data || !chart.data.datasets || chart.data.datasets.length < 2) {
            window.qzTreadmillSummaryDiagnostics = diagnostics;
            return diagnostics;
        }

        const chartData = buildTreadmillChartData(arr);
        chart.data.datasets[0].data = chartData.speedPoints;
        chart.data.datasets[1].data = chartData.inclinationPoints;

        // Chart.js 3 exposes chart.options through a resolver Proxy. Reassigning a
        // nested resolver (for example ticks = chart.options.scales.x.ticks) can
        // recursively call the Proxy setter in modern Chromium/WebKit. Mutate the
        // raw config object instead; Chart.update() will resolve it afterwards.
        const xScaleOptions = getRawXScaleOptions(chart);
        if (xScaleOptions) {
            xScaleOptions.max = chartData.lastElapsed;
            if (!xScaleOptions.ticks) {
                xScaleOptions.ticks = {};
            }
            xScaleOptions.ticks.callback = formatElapsedTick;
        }

        // The canvas is moved to its final treadmill location before Chart.js is
        // created. resize() here is only a final guard for WebView layout changes.
        if (typeof chart.resize === 'function') {
            chart.resize();
        }
        chart.update('none');

        diagnostics.speedPoints = chartData.speedPoints;
        diagnostics.inclinationPoints = chartData.inclinationPoints;
        diagnostics.xMax = xScaleOptions ? xScaleOptions.max : null;
        diagnostics.tick10 = formatElapsedTick(10);
        diagnostics.tick20 = formatElapsedTick(20);
        diagnostics.valid = diagnostics.durationSeconds === diagnostics.xMax &&
            diagnostics.speedPoints.some(function(point) { return finiteNumber(point.y, 0) > 0; }) &&
            diagnostics.inclinationPoints.some(function(point) { return finiteNumber(point.y, 0) !== 0; });

        window.qzTreadmillSummaryDiagnostics = diagnostics;
        if (diagnostics.valid) {
            console.info('treadmill_summary.js diagnostics: ' + JSON.stringify(diagnostics));
        } else {
            console.error('treadmill_summary.js invalid chart diagnostics: ' + JSON.stringify(diagnostics));
        }
        return diagnostics;
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

            // Do this BEFORE originalProcessArr() creates the responsive Chart.js
            // instance. Moving a live responsive canvas between parents can reset its
            // render state in Qt WebView/WebKit.
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
        updateTreadmillSpeedInclinationChart(arr);

        if (treadmillThumbnailTimer !== null) {
            clearTimeout(treadmillThumbnailTimer);
        }
        treadmillThumbnailTimer = setTimeout(saveTreadmillThumbnails, 1800);
    }

    window.process_arr = function (arr) {
        const treadmill = isTreadmillWorkout(arr);

        // Establish the final DOM location before Chart.js measures/creates canvases.
        if (treadmill) {
            configureTreadmillCharts();
        }

        // Keep the original session samples untouched, preserving bike behaviour.
        originalProcessArr(arr);

        if (treadmill) {
            adaptTreadmillPresentation(arr);
        }
    };
})();