// The metrics of a workout - power, speed, incline, resistance,
// cadence - each on its own small chart with its own scale, one under another. The chips at
// the top right ("On one chart:") put the selected metrics (two or more) together on one shared
// chart above the others; the choice is kept per page. The workout preview and the workout editor give it
// their prepared metrics:
//
//   var charts = QzMetricCharts.create(host, { storageKey: 'qz.workoutpreview.together' });
//   charts.update({ totalSeconds: 1800, series: [{ key: 'power', label: 'Power', unit: 'W',
//       color: '#3b82f6', points: [{ x: 0, y: 150 }, ...] }, ...] });
//
// A series: key, label, unit, color ('#rrggbb'), points ({x seconds, y; openEnded and
// segmentLabel of an open ended step}), stepped, floor - values below it mean "not set" and
// leave a gap (0 by default, the incline goes down to -50); axisTitle - the title of its scale
// ("label (unit)" by default); beginAtZero - false for a scale fitted to the values (by default
// a scale starts at zero unless the floor is below it). A series without a set value is not
// drawn. Options: storageKey (localStorage - per page origin, and the app gives its inner
// server a new port at every start, so the choice lasts while the app runs); colors() - {tick,
// grid, title} of the axes and {outline, tonal} of the chips (neutral greys for
// a light page by default; the workout editor gives light ones for its dark page); timeTitle() -
// the title of the time axis under the last chart; caption() - the words before the chips.
(function () {
    var STYLE = ''
        + '.qzmc-host{display:flex;flex-direction:column;gap:8px;min-height:0}'
        + '.qzmc-bar{display:flex;flex-wrap:wrap;justify-content:flex-end;align-items:center;gap:8px}'
        + '.qzmc-bar:empty{display:none}'
        + '.qzmc-caption{font-size:13px;opacity:.7;margin-right:2px}'
        // Material 3 filter chips: 32px high, corner 8px, an outline when off, a tonal fill and a
        // check when on; the touch area reaches 8px above and below the chip
        + '.qzmc-chip{position:relative;display:inline-flex;align-items:center;gap:6px;height:32px;padding:0 12px 0 8px;'
        + 'border:1px solid var(--qzmc-outline);border-radius:8px;background:transparent;color:inherit;font-family:inherit;font-size:14px;font-weight:500;line-height:1;'
        + 'cursor:pointer;white-space:nowrap;-webkit-tap-highlight-color:transparent}'
        + '.qzmc-chip:hover{border-color:var(--qzmc-outline)}'
        + '.qzmc-chip::after{content:"";position:absolute;left:0;right:0;top:-8px;bottom:-8px}'
        + '.qzmc-dot{width:10px;height:10px;border-radius:50%;margin:0 2px}'
        + '.qzmc-check{display:none;width:18px;height:18px}'
        + '.qzmc-chip[aria-pressed="true"]{border-color:transparent;background:var(--qzmc-tonal)}'
        + '.qzmc-chip[aria-pressed="true"] .qzmc-dot{display:none}'
        + '.qzmc-chip[aria-pressed="true"] .qzmc-check{display:block}'
        + '.qzmc-list{display:flex;flex-direction:column;gap:8px;flex:1 1 auto;min-height:0}'
        + '.qzmc-box{position:relative;flex:1 1 0;min-height:140px}'
        + '.qzmc-box canvas{width:100%!important;height:100%!important}';

    function injectStyle() {
        if (document.getElementById('qzmc-style')) {
            return;
        }
        var style = document.createElement('style');
        style.id = 'qzmc-style';
        style.textContent = STYLE;
        document.head.appendChild(style);
    }

    function clock(seconds) {
        var s = Math.round(seconds), h = Math.floor(s / 3600), m = Math.floor(s % 3600 / 60);
        function two(n) { return (n < 10 ? '0' : '') + n; }
        return (h ? h + ':' + two(m) : m) + ':' + two(s % 60);
    }

    function openEndedDash(ctx) {
        return (ctx.p0.raw && ctx.p0.raw.openEnded) || (ctx.p1.raw && ctx.p1.raw.openEnded) ? [6, 5] : undefined;
    }

    function axisTitle(series) {
        if (series.axisTitle) {
            return series.axisTitle;
        }
        return series.unit ? series.label + ' (' + series.unit + ')' : series.label;
    }

    function floorOf(series) {
        return typeof series.floor === 'number' ? series.floor : 0;
    }

    // "Not set" values become gaps; a series with no set value at all is dropped
    function clean(seriesList) {
        var out = [];
        (seriesList || []).forEach(function (series) {
            var floor = floorOf(series);
            var any = false;
            var points = (series.points || []).map(function (p) {
                var y = p && typeof p === 'object' ? p.y : p;
                if (y === null || y === undefined || y === '' || isNaN(Number(y)) || Number(y) < floor) {
                    return Object.assign({}, p, { y: null });
                }
                any = true;
                return p;
            });
            if (any) {
                out.push(Object.assign({}, series, { points: points }));
            }
        });
        return out;
    }

    function readTogether(key) {
        try {
            var v = key && window.localStorage.getItem(key);
            var list = v ? JSON.parse(v) : [];
            return Array.isArray(list) ? list : [];
        } catch (e) {
            return [];
        }
    }

    function writeTogether(key, list) {
        try {
            if (key) {
                window.localStorage.setItem(key, JSON.stringify(list));
            }
        } catch (e) {
            // no storage (private window, preview): the choice lasts while the page is open
        }
    }

    window.QzMetricCharts = {
        create: function (host, options) {
            options = options || {};
            injectStyle();
            host.classList.add('qzmc-host');
            var bar = document.createElement('div');
            bar.className = 'qzmc-bar';
            var list = document.createElement('div');
            list.className = 'qzmc-list';
            host.appendChild(bar);
            host.appendChild(list);

            var together = readTogether(options.storageKey);
            var payload = null;
            var charts = []; // {group: [keys], chart, box}
            var layout = '';

            function colors() {
                return Object.assign({ tick: '#666', grid: 'rgba(0, 0, 0, 0.05)', title: '#666',
                                       outline: '#79747e', tonal: '#e8def8' },
                                     options.colors ? options.colors() : {});
            }

            function timeTitle() {
                return options.timeTitle ? options.timeTitle() : 'Time';
            }

            // The axes of a shared chart: the first keeps the usual id "y", the next ones y1, y2...
            function axisId(i) {
                return i ? 'y' + i : 'y';
            }

            function scales(group, last) {
                var c = colors();
                var result = {
                    x: {
                        type: 'linear',
                        min: 0,
                        grid: { color: c.grid },
                        ticks: { color: c.tick, maxRotation: 0, includeBounds: false, callback: function (v) { return clock(v); } },
                        title: { display: last, text: timeTitle(), color: c.title }
                    }
                };
                // The axis ends with the workout; its end gets no tick of its own (includeBounds),
                // which ran into the last round one
                if (payload && typeof payload.totalSeconds === 'number' && payload.totalSeconds > 0) {
                    result.x.max = payload.totalSeconds;
                }
                group.forEach(function (series, i) {
                    var shared = group.length > 1;
                    result[axisId(i)] = {
                        type: 'linear',
                        position: i % 2 ? 'right' : 'left',
                        beginAtZero: series.beginAtZero !== false && floorOf(series) >= 0,
                        grid: i ? { drawOnChartArea: false, color: c.grid } : { color: c.grid },
                        ticks: { color: shared ? series.color : c.tick },
                        title: { display: true, text: axisTitle(series), color: series.color }
                    };
                });
                return result;
            }

            function datasets(group) {
                var shared = group.length > 1;
                return group.map(function (series, i) {
                    var color = series.color || '#3b82f6';
                    return {
                        label: series.label,
                        unit: series.unit || '',
                        data: series.points,
                        yAxisID: axisId(i),
                        borderColor: color,
                        backgroundColor: color + (shared ? '00' : '22'),
                        fill: !shared,
                        borderWidth: 2,
                        stepped: !!series.stepped,
                        tension: 0,
                        pointRadius: 0,
                        spanGaps: false,
                        segment: { borderDash: openEndedDash }
                    };
                });
            }

            function config(group, last) {
                return {
                    type: 'line',
                    data: { datasets: datasets(group) },
                    options: {
                        responsive: true,
                        maintainAspectRatio: false,
                        animation: false,
                        interaction: { mode: 'index', intersect: false },
                        plugins: {
                            legend: { display: false },
                            tooltip: {
                                backgroundColor: 'rgba(0, 0, 0, 0.8)',
                                titleColor: '#fff',
                                bodyColor: '#fff',
                                callbacks: {
                                    title: function (items) { return items.length ? clock(items[0].parsed.x) : ''; },
                                    label: function (item) {
                                        var ds = item.dataset;
                                        // The number as the axis writes it (Chart.js formats ticks
                                        // by the page locale): "9,7", not "9.7", where comma is the
                                        // decimal mark
                                        var value = Math.round(item.parsed.y * 10) / 10;
                                        var text = ds.label + ': ' + value.toLocaleString(item.chart.options.locale);
                                        if (ds.unit) {
                                            text += ' ' + ds.unit;
                                        }
                                        if (item.raw && item.raw.segmentLabel) {
                                            text += ' (' + item.raw.segmentLabel + ')';
                                        }
                                        return text;
                                    }
                                }
                            }
                        },
                        scales: scales(group, last)
                    }
                };
            }

            // The shared chart first (the ticked metrics, when two or more are drawn), then
            // each other metric alone
            function groups(seriesList) {
                var ticked = seriesList.filter(function (s) { return together.indexOf(s.key) >= 0; });
                var result = [];
                if (ticked.length > 1) {
                    result.push(ticked);
                }
                seriesList.forEach(function (s) {
                    if (ticked.length < 2 || ticked.indexOf(s) < 0) {
                        result.push([s]);
                    }
                });
                return result;
            }

            var CHECK = '<svg class="qzmc-check" viewBox="0 0 24 24" aria-hidden="true">'
                + '<path fill="currentColor" d="M9 16.2 4.8 12l-1.4 1.4L9 19 21 7l-1.4-1.4z"/></svg>';

            function drawBar(seriesList) {
                bar.innerHTML = '';
                if (seriesList.length < 2) {
                    return;
                }
                var c = colors();
                host.style.setProperty('--qzmc-outline', c.outline);
                host.style.setProperty('--qzmc-tonal', c.tonal);
                var caption = document.createElement('span');
                caption.className = 'qzmc-caption';
                caption.textContent = options.caption ? options.caption() : 'On one chart:';
                bar.appendChild(caption);
                seriesList.forEach(function (series) {
                    var on = together.indexOf(series.key) >= 0;
                    var chip = document.createElement('button');
                    chip.type = 'button';
                    chip.className = 'qzmc-chip';
                    chip.setAttribute('aria-pressed', on ? 'true' : 'false');
                    var dot = document.createElement('span');
                    dot.className = 'qzmc-dot';
                    dot.style.backgroundColor = series.color;
                    var text = document.createElement('span');
                    text.textContent = series.label;
                    chip.appendChild(dot);
                    chip.insertAdjacentHTML('beforeend', CHECK);
                    chip.lastChild.style.color = series.color;
                    chip.appendChild(text);
                    chip.addEventListener('click', function () {
                        together = together.filter(function (k) { return k !== series.key; });
                        if (!on) {
                            together.push(series.key);
                        }
                        writeTogether(options.storageKey, together);
                        draw(true);
                    });
                    bar.appendChild(chip);
                });
            }

            function clear() {
                charts.forEach(function (c) { c.chart.destroy(); });
                charts = [];
                list.innerHTML = '';
            }

            // Same metrics in the same charts: the data and scales change in place (the editor
            // redraws on every keystroke); otherwise the charts are made anew
            function draw(force) {
                var seriesList = clean(payload ? payload.series : []);
                var plan = groups(seriesList);
                var key = plan.map(function (g) { return g.map(function (s) { return s.key; }).join('+'); }).join('|')
                          + '#' + seriesList.map(function (s) { return s.label + s.unit + s.color; }).join(',');
                host.classList.toggle('qzmc-single', plan.length === 1);
                if (force || key !== layout) {
                    layout = key;
                    clear();
                    drawBar(seriesList);
                    plan.forEach(function (group, i) {
                        var box = document.createElement('div');
                        box.className = 'qzmc-box';
                        var canvas = document.createElement('canvas');
                        box.appendChild(canvas);
                        list.appendChild(box);
                        var chart = new Chart(canvas.getContext('2d'), config(group, i === plan.length - 1));
                        charts.push({ chart: chart, box: box });
                    });
                    return;
                }
                plan.forEach(function (group, i) {
                    var chart = charts[i].chart;
                    var fresh = config(group, i === plan.length - 1);
                    chart.config.data.datasets = fresh.data.datasets;
                    chart.config.options.scales = fresh.options.scales;
                    chart.update('none');
                });
            }

            return {
                update: function (next) {
                    payload = next;
                    draw(false);
                }
            };
        }
    };
})();
