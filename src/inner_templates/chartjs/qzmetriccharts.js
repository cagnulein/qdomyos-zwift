// Modern look (fork only): the metrics of a workout - power, speed, incline, resistance,
// cadence - each on its own small chart with its own scale, one under another. The chips at
// the top right put the ticked metrics (two or more) together on one shared chart above the
// others; the choice is kept per page. The workout preview and the workout editor give it
// their prepared metrics:
//
//   var charts = QzMetricCharts.create(host, { storageKey: 'qz.preview.together' });
//   charts.update({ totalSeconds: 1800, series: [{ key: 'power', label: 'Power', unit: 'W',
//       color: '#3b82f6', points: [{ x: 0, y: 150 }, ...] }, ...] });
//
// A series: key, label, unit, color ('#rrggbb'), points ({x seconds, y; openEnded and
// segmentLabel of an open ended step}), stepped, floor - values below it mean "not set" and
// leave a gap (0 by default, the incline goes down to -50). A series without a set value is
// not drawn. Options: storageKey; make(ctx, config) and restyle(chart, config) - how the page
// makes and redraws a chart (qzChartTheme on the preview, new Chart by default); colors() -
// {tick, grid, title} of the axes (neutral greys by default, which qzChartTheme repaints for
// the dark theme); timeTitle() - the title of the time axis under the last chart.
(function () {
    var STYLE = ''
        + '.qzmc-host{display:flex;flex-direction:column;gap:8px;min-height:0}'
        + '.qzmc-bar{display:flex;flex-wrap:wrap;justify-content:flex-end;gap:6px 12px;font-size:13px;line-height:1.2}'
        + '.qzmc-bar:empty{display:none}'
        + '.qzmc-chip{display:inline-flex;align-items:center;gap:6px;cursor:pointer;user-select:none;-webkit-user-select:none;white-space:nowrap}'
        + '.qzmc-chip input{margin:0;width:16px;height:16px}'
        + '.qzmc-swatch{display:inline-block;width:12px;height:3px;border-radius:2px}'
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
            return v ? JSON.parse(v) : [];
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

            function make(ctx, config) {
                return options.make ? options.make(ctx, config) : new Chart(ctx, config);
            }

            function colors() {
                return Object.assign({ tick: '#666', grid: 'rgba(0, 0, 0, 0.05)', title: '#666' },
                                     options.colors ? options.colors() : {});
            }

            function timeTitle() {
                return options.timeTitle ? options.timeTitle() : 'Time';
            }

            // The first axis is "y": qzChartTheme styles the x and y scales of every chart and
            // would add a stray default y axis to a chart without one
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
                        beginAtZero: floorOf(series) >= 0,
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
                                        var text = ds.label + ': ' + Math.round(item.parsed.y * 10) / 10;
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

            function drawBar(seriesList) {
                bar.innerHTML = '';
                if (seriesList.length < 2) {
                    return;
                }
                seriesList.forEach(function (series) {
                    var chip = document.createElement('label');
                    chip.className = 'qzmc-chip';
                    var box = document.createElement('input');
                    box.type = 'checkbox';
                    box.checked = together.indexOf(series.key) >= 0;
                    box.addEventListener('change', function () {
                        together = together.filter(function (k) { return k !== series.key; });
                        if (box.checked) {
                            together.push(series.key);
                        }
                        writeTogether(options.storageKey, together);
                        draw(true);
                    });
                    var swatch = document.createElement('span');
                    swatch.className = 'qzmc-swatch';
                    swatch.style.backgroundColor = series.color;
                    var text = document.createElement('span');
                    text.textContent = series.label;
                    chip.appendChild(box);
                    chip.appendChild(swatch);
                    chip.appendChild(text);
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
                        var chart = make(canvas.getContext('2d'), config(group, i === plan.length - 1));
                        charts.push({ chart: chart, box: box });
                    });
                    return;
                }
                plan.forEach(function (group, i) {
                    var chart = charts[i].chart;
                    var fresh = config(group, i === plan.length - 1);
                    chart.config.data.datasets = fresh.data.datasets;
                    chart.config.options.scales = fresh.options.scales;
                    if (options.restyle) {
                        options.restyle(chart, { options: chart.config.options, data: chart.config.data });
                    }
                    chart.update('none');
                });
            }

            return {
                update: function (next) {
                    payload = next;
                    draw(false);
                },
                // Redraw in the current colours (the page theme changed)
                refresh: function () {
                    draw(true);
                },
                // How many charts are drawn (the page sizes itself by it)
                count: function () {
                    return charts.length;
                }
            };
        }
    };
})();
