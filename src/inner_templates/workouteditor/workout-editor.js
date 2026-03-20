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
    const state = {
        chart: null,
        axisTemplate: {
            type: 'linear',
            grid: {
                color: 'rgba(255,255,255,0.04)'
            },
            ticks: {
                color: 'rgba(255,255,255,0.65)'
            },
            border: {
                color: 'rgba(255,255,255,0.16)'
            }
        }
    };

    function formatSeconds(total) {
        if (!isFinite(total)) {
            return '0:00';
        }
        const minutes = Math.floor(total / 60);
        const seconds = Math.floor(total % 60);
        return minutes + ':' + seconds.toString().padStart(2, '0');
    }

    function ensureChart() {
        if (state.chart) {
            return state.chart;
        }
        const canvas = document.getElementById('workoutChart');
        if (!canvas) {
            return null;
        }
        const ctx = canvas.getContext('2d');
        state.chart = new Chart(ctx, {
            type: 'line',
            data: { datasets: [] },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                animation: false,
                plugins: {
                    legend: { display: false },
                    tooltip: {
                        intersect: false,
                        mode: 'index',
                        backgroundColor: 'rgba(15,21,30,0.92)',
                        borderWidth: 0,
                        callbacks: {
                            title: (items) => items.length ? formatSeconds(items[0].parsed.x) : '',
                            label: (item) => {
                                const dataset = item.dataset;
                                const unit = dataset.unit ? ' ' + dataset.unit : '';
                                return dataset.label + ': ' + item.formattedValue + unit;
                            }
                        }
                    },
                    title: {
                        display: false
                    }
                },
                scales: {
                    x: {
                        type: 'linear',
                        grid: {
                            color: 'rgba(255,255,255,0.04)'
                        },
                        border: {
                            color: 'rgba(255,255,255,0.16)'
                        },
                        ticks: {
                            color: 'rgba(255,255,255,0.65)',
                            callback: (value) => formatSeconds(value)
                        },
                        title: {
                            display: true,
                            text: 'Time',
                            color: 'rgba(255,255,255,0.75)'
                        }
                    }
                },
                elements: {
                    point: {
                        radius: 0
                    },
                    line: {
                        tension: 0,
                        borderWidth: 2
                    }
                }
            }
        });
        return state.chart;
    }

    function buildAxes(seriesList) {
        const axes = {};
        seriesList.forEach((series) => {
            const axisId = series.axis || 'y';
            if (!axes[axisId]) {
                axes[axisId] = JSON.parse(JSON.stringify(state.axisTemplate));
                axes[axisId].position = series.axisPosition || 'left';
                axes[axisId].id = axisId;
                axes[axisId].title = {
                    display: !!series.axisLabel,
                    text: series.axisLabel || '',
                    color: 'rgba(255,255,255,0.75)'
                };
                if (typeof series.min === 'number') {
                    axes[axisId].min = series.min;
                }
                if (typeof series.max === 'number') {
                    axes[axisId].max = series.max;
                }
                if (series.stacked) {
                    axes[axisId].stacked = true;
                }
            }
        });
        return axes;
    }

    function updateLegend(seriesList) {
        const legendRoot = document.getElementById('legend');
        if (!legendRoot) {
            return;
        }
        legendRoot.innerHTML = '';
        seriesList.forEach((series) => {
            const item = document.createElement('div');
            item.className = 'legend-item';
            const swatch = document.createElement('div');
            swatch.className = 'legend-swatch';
            swatch.style.backgroundColor = series.color || '#35baf6';
            const label = document.createElement('div');
            label.textContent = series.label + (series.unit ? ' (' + series.unit + ')' : '');
            item.appendChild(swatch);
            item.appendChild(label);
            legendRoot.appendChild(item);
        });
    }

    function updateMeta(payload) {
        const title = document.getElementById('chartTitle');
        const meta = document.getElementById('chartMeta');
        if (title) {
            title.textContent = payload.title || 'Workout Preview';
        }
        if (meta) {
            const parts = [];
            if (payload.subtitle) {
                parts.push(payload.subtitle);
            }
            if (typeof payload.totalSeconds === 'number') {
                parts.push('Duration ' + formatSeconds(payload.totalSeconds));
            }
            if (Array.isArray(payload.rows)) {
                parts.push(payload.rows.length + ' intervals');
            }
            meta.textContent = parts.join(' • ');
        }
    }

    function updateChart(payload) {
        const chart = ensureChart();
        if (!chart) {
            return;
        }
        const seriesList = Array.isArray(payload.series) ? payload.series : [];
        const axes = buildAxes(seriesList);
        chart.options.scales = Object.assign({ x: chart.options.scales.x }, axes);
        chart.data.datasets = seriesList.map((series) => {
            const color = String(series.color || '#35baf6');
            const fillColor = String(series.fillColor || color) + '33';
            return ({
                label: series.label || series.key || 'Series',
                data: Array.isArray(series.points) ? series.points : [],
                borderColor: color,
                backgroundColor: fillColor,
                unit: series.unit || '',
                yAxisID: series.axis || 'y',
                stepped: true,
                borderWidth: series.lineWidth || 2,
                fill: Boolean(series.fill),
                tension: 0,
                spanGaps: true
            });
        });
        if (typeof payload.totalSeconds === 'number') {
            chart.options.scales.x.suggestedMax = payload.totalSeconds;
        }
        chart.update();
        updateLegend(seriesList);
        updateMeta(payload);
    }

    function reset() {
        const chart = ensureChart();
        if (!chart) {
            return;
        }
        chart.data.datasets = [];
        chart.update();
        updateLegend([]);
    }

    window.WorkoutEditorApp = {
        update: updateChart,
        reset
    };

    window.addEventListener('DOMContentLoaded', ensureChart);
})();
