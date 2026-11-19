// Modern look: the Chart.js charts of the web pages follow the app theme.
// A page creates its charts with qzChartTheme.create(ctx, config) instead of new Chart().
// Without html.modern (the classic look, or a page opened without the theme fragment) the
// config is left as it is. The canvas fill takes the card colour of the theme; in the dark
// theme the neutral colours - black and grey text, grid lines drawn for white, the black
// target line - take the theme colours too; the colours that carry meaning (zones, metrics)
// stay. The chart images saved for the workout mail (qzChartTheme.image) are always drawn in
// the original light look.
(function () {
    // Colours drawn for a white background, replaced in the dark theme (spaces dropped,
    // lower case); rgba(0, 0, 0, any alpha) as well
    var NEUTRAL = ['black', '#000', '#000000', '#666', '#666666', 'rgb(0,0,0)'];
    var charts = [];

    // Sport pictures of the summary (a lime drawing on a grey disc) and the icons of the
    // app's set (Material Symbols Rounded, the paths of ShapeIcon.qml) drawn instead in the modern look
    var SPORT_ICONS = {
        'bike.png': 'M200-160q-85 0-142.5-57.5T0-360q0-85 58.5-142.5T200-560q77 0 129.5 46T396-400h26l-72-200h-30q-17 0-28.5-11.5T280-640q0-17 11.5-28.5T320-680h120q17 0 28.5 11.5T480-640q0 17-11.5 28.5T440-600h-4l14 40h192l-58-160h-64q-17 0-28.5-11.5T480-760q0-17 11.5-28.5T520-800h64q26 0 46.5 14t29.5 38l68 186h32q83 0 141.5 58.5T960-362q0 84-58 143t-142 59q-72 0-126.5-45T564-320H396q-14 69-68 114.5T200-160Zm0-80q41 0 70.5-22.5T312-320h-72q-17 0-28.5-11.5T200-360q0-17 11.5-28.5T240-400h72q-12-36-41.5-58T200-480q-51 0-85.5 34.5T80-360q0 50 34.5 85t85.5 35Zm308-160h56q5-23 13.5-43t22.5-37H478l30 80Zm252 160q51 0 85.5-35t34.5-85q0-51-34.5-85.5T760-480h-4l26 69q6 16-1 30.5T758-360q-16 6-31-1t-21-23l-24-68q-20 17-31 40t-11 52q0 50 34.5 85t85.5 35ZM196-360Zm564 0Z',
        'run.png': 'M520-80v-200l-84-80-31 138q-4 16-17.5 24.5T358-192l-198-40q-17-3-26-17t-6-31q3-17 17-26.5t31-5.5l152 32 64-324-72 28v96q0 17-11.5 28.5T280-440q-17 0-28.5-11.5T240-480v-122q0-12 6.5-21.5T264-638l134-58q35-15 51.5-19.5T480-720q21 0 39 11t29 29l40 64q21 34 54.5 59t77.5 33q17 3 28.5 15t11.5 29q0 17-11.5 28t-27.5 9q-54-8-101-33.5T540-540l-24 120 72 68q6 6 9 13.5t3 15.5v243q0 17-11.5 28.5T560-40q-17 0-28.5-11.5T520-80Zm20-660q-33 0-56.5-23.5T460-820q0-33 23.5-56.5T540-900q33 0 56.5 23.5T620-820q0 33-23.5 56.5T540-740Z',
        'row.png': 'm692-28-80-80q-6-6-9-13.5t-3-15.5v-43L316-464q-9 2-18 3t-18 1v-88q50 2 102-21.5t84-58.5l56-62q13-15 30.5-22.5T590-720q38 0 64 26t26 64v230q0 26-9.5 47.5T644-314L500-456v-92q-20 17-43 31t-49 25l252 252h43q8 0 15.5 3t13.5 9l80 80q12 12 12 28t-12 28l-64 64q-12 12-28 12t-28-12ZM360-280 250-170q-13 13-30 13t-30-13q-13-13-13-30t13-30l150-150 100 100h-80Zm240-480q-33 0-56.5-23.5T520-840q0-33 23.5-56.5T600-920q33 0 56.5 23.5T680-840q0 33-23.5 56.5T600-760ZM280-460q-18 0-31-13t-13-31q0-18 13-31t31-13q18 0 31 13t13 31q0 18-13 31t-31 13Z',
        'elliptical.png': 'M282-622 168-508q-11 11-27.5 11.5T112-508q-11-11-11.5-27.5T111-564l29-30-28-28q-12-12-12-28t12-28l56-56-29-30q-11-11-11-27.5t12-28.5q11-11 27.5-11.5T196-821l30 29 56-56q12-12 28-12t28 12l28 28 30-29q11-11 27.5-11t28.5 12q11 11 11 28t-11 28L338-678l340 340 114-114q11-11 27.5-11.5T848-452q11 11 11.5 27.5T849-396l-29 30 28 28q12 12 12 28t-12 28l-56 56 29 30q11 11 11 27.5T820-140q-11 11-27.5 11.5T764-139l-30-29-56 56q-12 12-28 12t-28-12l-28-28-30 29q-11 11-27.5 11T508-112q-11-11-11-28t11-28l114-114-340-340Z'
    };

    // The sport picture in the theme colours: the accent icon on a disc one step lighter than
    // the card. The PNG name stays in data-classic-src - for the classic look and the saved
    // badge (snapshotOptions puts it back in the copy that html2canvas draws)
    function applySportIcons() {
        var root = document.documentElement;
        var imgs = document.querySelectorAll('img[data-classic-src]');
        for (var i = 0; i < imgs.length; i++) {
            var img = imgs[i];
            var png = img.getAttribute('data-classic-src');
            if (!root.classList.contains('modern') || !SPORT_ICONS[png]) {
                img.setAttribute('src', png);
                continue;
            }
            var css = getComputedStyle(root);
            var accent = css.getPropertyValue('--qz-accent').trim() || '#b69df8';
            var disc = css.getPropertyValue('--qz-surface-high').trim() || '#23272e';
            var svg = '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 120 120">'
                + '<circle cx="60" cy="60" r="60" fill="' + disc + '"/>'
                + '<path transform="translate(28 92) scale(0.0667)" fill="' + accent + '" d="' + SPORT_ICONS[png] + '"/>'
                + '</svg>';
            img.setAttribute('src', 'data:image/svg+xml;charset=utf-8,' + encodeURIComponent(svg));
        }
    }

    function palette() {
        var root = document.documentElement;
        if (!root.classList.contains('modern')) {
            return null;
        }
        var css = getComputedStyle(root);
        function v(name, fallback) {
            var s = css.getPropertyValue(name).trim();
            return s || fallback;
        }
        return {
            dark: !root.classList.contains('light'),
            bg: v('--qz-surface', '#1b1f27'),
            text: v('--qz-text', '#e6e8ee'),
            muted: v('--qz-muted', '#9aa1ad'),
            grid: 'rgba(255, 255, 255, 0.07)',
            axis: 'rgba(255, 255, 255, 0.16)'
        };
    }

    function isNeutral(c) {
        if (c === undefined) {
            return true;
        }
        if (typeof c !== 'string') {
            return false;
        }
        var s = c.split(' ').join('').toLowerCase();
        return NEUTRAL.indexOf(s) >= 0
            || (s.indexOf('rgba(0,0,0,') === 0 && s.charAt(s.length - 1) === ')');
    }

    // 'rgba(r, g, b, 0.55)' -> 'rgba(r, g, b, 0.3)'; any other colour as it is
    function fainterBand(c) {
        var i = c.lastIndexOf(',');
        if (i < 0 || c.substring(i + 1).split(' ').join('') !== '0.55)') {
            return c;
        }
        return c.substring(0, i) + ', 0.3)';
    }

    // Paths, not object references: Chart.js rebuilds options.scales on every update
    function get(root, path) {
        var o = root;
        for (var i = 0; i < path.length; i++) {
            if (o === undefined || o === null) {
                return undefined;
            }
            o = o[path[i]];
        }
        return o;
    }

    function set(root, path, value) {
        var o = root;
        for (var i = 0; i < path.length - 1; i++) {
            if (o[path[i]] === undefined || o[path[i]] === null) {
                if (value === undefined) {
                    return;
                }
                o[path[i]] = {};
            }
            o = o[path[i]];
        }
        if (value === undefined) {
            delete o[path[path.length - 1]];
        } else {
            o[path[path.length - 1]] = value;
        }
    }

    // Each patch: a path, the original value and the value for a theme palette
    function patches(config) {
        var list = [];
        function add(path, themed) {
            var light = get(config, path);
            list.push({ path: path, light: light, themed: function (pal) { return themed(pal, light); } });
        }
        function neutral(pal, light, darkValue) {
            return pal.dark && isNeutral(light) ? darkValue : light;
        }

        var scales = (config.options && config.options.scales) || {};
        var ids = Object.keys(scales);
        ['x', 'y'].forEach(function (id) {
            if (ids.indexOf(id) < 0) {
                ids.push(id);
            }
        });
        ids.forEach(function (id) {
            var s = ['options', 'scales', id];
            add(s.concat(['ticks', 'color']), function (pal, light) {
                return neutral(pal, light, light === 'black' ? pal.text : pal.muted);
            });
            add(s.concat(['title', 'color']), function (pal, light) { return neutral(pal, light, pal.muted); });
            add(s.concat(['grid', 'color']), function (pal, light) { return neutral(pal, light, pal.grid); });
            add(s.concat(['grid', 'borderColor']), function (pal, light) { return neutral(pal, light, pal.axis); });
        });

        add(['options', 'plugins', 'title', 'color'], function (pal, light) { return neutral(pal, light, pal.text); });
        add(['options', 'plugins', 'title', 'font', 'weight'], function () { return '600'; });
        add(['options', 'plugins', 'legend', 'labels', 'color'], function (pal, light) { return neutral(pal, light, pal.muted); });

        // The zone bands: lighter on a dark card, so the line over them stands out, and with
        // the borders of the grid instead of the plugin's grey
        var annotations = get(config, ['options', 'plugins', 'annotation', 'annotations']) || {};
        Object.keys(annotations).forEach(function (key) {
            var a = ['options', 'plugins', 'annotation', 'annotations', key];
            add(a.concat(['backgroundColor']), function (pal, light) {
                return pal.dark && typeof light === 'string' ? fainterBand(light) : light;
            });
            add(a.concat(['borderColor']), function (pal, light) { return neutral(pal, light, pal.axis); });
        });

        // The black target line (Req. Watts, requested resistance and cadence)
        var datasets = get(config, ['data', 'datasets']) || [];
        datasets.forEach(function (ds, i) {
            ['borderColor', 'backgroundColor'].forEach(function (key) {
                if (typeof ds[key] === 'string' && isNeutral(ds[key])) {
                    add(['data', 'datasets', i, key], function (pal, light) { return pal.dark ? pal.text : light; });
                }
            });
        });
        return list;
    }

    function apply(root, list, pal) {
        list.forEach(function (p) {
            set(root, p.path, pal ? p.themed(pal) : p.light);
        });
    }

    function rootOf(chart) {
        return { options: chart.config.options, data: chart.config.data };
    }

    window.qzChartTheme = {
        create: function (ctx, config) {
            var list = patches(config);
            var pal = palette();
            apply(config, list, pal);
            var chart = new Chart(ctx, config);
            chart.$qzPatches = list;
            chart.$qzPalette = pal;
            // Pages that recreate their chart drop the destroyed ones here
            charts = charts.filter(function (c) { return !!c.canvas; });
            charts.push(chart);
            return chart;
        },

        // The fill of the canvas, drawn by dochart.js under each chart: the card colour of the
        // theme, white in the classic look and in the images for the mail (withLight)
        background: function (chart) {
            var pal = chart && chart.$qzPalette;
            return pal ? pal.bg : 'white';
        },

        // Marks a sport picture (img with a PNG of SPORT_ICONS) to follow the theme
        sportIcon: function (img) {
            if (!img) {
                return;
            }
            var png = img.getAttribute('data-classic-src') || img.getAttribute('src');
            if (!SPORT_ICONS[png]) {
                return;
            }
            img.setAttribute('data-classic-src', png);
            applySportIcons();
        },

        // After the page theme changes (qzApplyTheme)
        refresh: function () {
            applySportIcons();
            var pal = palette();
            charts = charts.filter(function (chart) { return !!chart.canvas; });
            charts.forEach(function (chart) {
                chart.$qzPalette = pal;
                apply(rootOf(chart), chart.$qzPatches, pal);
                chart.update('none');
            });
        },

        // Runs fn with the chart drawn in the original light look and returns its result; the
        // screen gets the theme back in the same task, so nothing flashes. Redrawing calls the
        // chart's animation.onComplete again: the callers set their "saved" flag first.
        withLight: function (chart, fn) {
            var pal = chart && chart.$qzPalette;
            if (!pal || !chart.$qzPatches) {
                return fn();
            }
            chart.$qzPalette = null;
            apply(rootOf(chart), chart.$qzPatches, null);
            chart.update('none');
            try {
                return fn();
            } finally {
                chart.$qzPalette = pal;
                apply(rootOf(chart), chart.$qzPatches, pal);
                chart.update('none');
            }
        },

        // The chart as a PNG for the workout mail, in the original light look
        image: function (chart) {
            return this.withLight(chart, function () { return chart.toBase64Image(); });
        },

        // html2canvas options: the saved summary badge keeps the classic look. html2canvas
        // copies the canvases synchronously when called, so the call goes inside withLight()
        // for the chart in the badge. html2canvas draws a picture by its currentSrc, which follows
        // a new src only once the browser starts loading it, and waits for the promise onclone
        // returns: so the copy gets the PNG back and onclone resolves when it has loaded (or
        // failed); a picture that already shows the PNG (the classic look) is left as it is
        snapshotOptions: function () {
            return {
                onclone: function (doc) {
                    doc.documentElement.classList.remove('modern', 'light');
                    var imgs = doc.querySelectorAll('img[data-classic-src]');
                    var loads = [];
                    for (var i = 0; i < imgs.length; i++) {
                        var img = imgs[i];
                        var png = img.getAttribute('data-classic-src');
                        if (img.getAttribute('src') === png) {
                            continue;
                        }
                        loads.push(new Promise(function (resolve) {
                            img.onload = resolve;
                            img.onerror = resolve;
                        }));
                        img.setAttribute('src', png);
                    }
                    return Promise.all(loads);
                }
            };
        }
    };
})();
