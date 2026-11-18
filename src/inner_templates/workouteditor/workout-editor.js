(function () {
    if (typeof Object.prototype.startsWith !== 'function') {
        Object.defineProperty(Object.prototype, 'startsWith', {
            value: function(search) {
                try {
                    return String(this).startsWith(search);
                } catch (e) {
                    return false;
                }
            },
            configurable: true,
            enumerable: false,
            writable: true
        });
    }
})();

(function () {
    function t(key, fallback) {
        return window.qzTranslate ? window.qzTranslate(key, fallback) : fallback;
    }

    const state = {
        charts: null
    };

    // Axis colours over the dark page of the editor (the same as the single chart had before),
    // and the chips: a light outline, the accent of the editor as the fill of a selected one
    const AXIS_COLORS = {
        tick: 'rgba(255,255,255,0.65)',
        grid: 'rgba(255,255,255,0.04)',
        title: 'rgba(255,255,255,0.75)',
        outline: 'rgba(255,255,255,0.38)',
        tonal: 'rgba(53,186,246,0.24)'
    };

    function formatSeconds(total) {
        if (!isFinite(total)) {
            return '0:00';
        }
        const minutes = Math.floor(total / 60);
        const seconds = Math.floor(total % 60);
        return minutes + ':' + seconds.toString().padStart(2, '0');
    }

    // Each metric on its own chart, the ticked ones together (qzmetriccharts.js, shared with
    // the workout preview); before, all of them were on one chart with an axis each
    function ensureCharts() {
        if (state.charts) {
            return state.charts;
        }
        const host = document.querySelector('.chart-wrapper');
        if (!host) {
            return null;
        }
        state.charts = QzMetricCharts.create(host, {
            storageKey: 'qz.workouteditor.together',
            colors: () => AXIS_COLORS,
            timeTitle: () => t('workoutEditor.time', 'Time'),
            caption: () => t('chart.onOneChart', 'On one chart:')
        });
        return state.charts;
    }

    function updateMeta(payload) {
        const title = document.getElementById('chartTitle');
        const meta = document.getElementById('chartMeta');
        if (title) {
            title.textContent = payload.title || t('workoutEditor.workoutPreview', 'Workout Preview');
        }
        if (meta) {
            const parts = [];
            if (payload.subtitle) {
                parts.push(payload.subtitle);
            }
            if (typeof payload.totalSeconds === 'number') {
                parts.push(t('workoutEditor.durationValue', 'Duration {value}').replace('{value}', formatSeconds(payload.totalSeconds)));
            }
            if (Array.isArray(payload.rows)) {
                parts.push(t('workoutEditor.intervalsCount', '{count} intervals').replace('{count}', payload.rows.length));
            }
            meta.textContent = parts.join(' • ');
        }
    }

    function updateChart(payload) {
        const charts = ensureCharts();
        if (!charts) {
            return;
        }
        const seriesList = Array.isArray(payload.series) ? payload.series : [];
        charts.update({
            totalSeconds: payload.totalSeconds,
            series: seriesList.map((series) => ({
                key: series.key,
                label: series.label || series.key || 'Series',
                unit: series.unit || '',
                color: String(series.color || '#35baf6'),
                points: Array.isArray(series.points) ? series.points : [],
                stepped: series.stepped !== false,
                floor: series.key === 'inclination' ? -50 : 0,
                // The scale titles and ranges as the single chart had them: the translated axis
                // label of the series and a scale fitted to the values, not from zero
                axisTitle: series.axisLabel || '',
                beginAtZero: false
            }))
        });
        updateMeta(payload);
    }

    function reset() {
        const charts = ensureCharts();
        if (!charts) {
            return;
        }
        charts.update({ series: [] });
    }

    window.WorkoutEditorApp = {
        update: updateChart,
        reset
    };

    window.addEventListener('DOMContentLoaded', ensureCharts);
})();
