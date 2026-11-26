// Modern look of the app on its web pages (charts, workout history, maps, workout editor,
// workout preview). The QML page passes
// the theme colours in the URL fragment (#modern=1&dark=1&bg=...) for the first load
// (window.ui.webThemeFragment() in main.qml) and calls qzApplyTheme() when the theme changes
// later (window.ui.webThemeScript()). Without the fragment - the classic look - the page keeps
// its own look and nothing here changes it.
// The page CSS reads the colours as variables (--qz-bg, --qz-surface, ...) under html.modern,
// html.modern.light for the light theme. Loaded in <head>, before the page draws.
(function () {
    var VARS = {
        bg: '--qz-bg', surface: '--qz-surface', surfaceHigh: '--qz-surface-high',
        surfaceHighest: '--qz-surface-highest', outline: '--qz-outline', text: '--qz-text',
        muted: '--qz-muted', accent: '--qz-accent', accentInk: '--qz-accent-ink',
        danger: '--qz-danger'
    };

    // theme: the object of window.ui.webTheme, or null when the modern look is switched off
    window.qzApplyTheme = function (theme) {
        var root = document.documentElement;
        if (theme) {
            for (var key in VARS) {
                if (theme[key]) {
                    root.style.setProperty(VARS[key], theme[key]);
                }
            }
            // true from runJavaScript, '1' from the fragment
            var dark = theme.dark === true || theme.dark === '1';
            root.classList.add('modern');
            root.classList.toggle('light', !dark);
            root.style.colorScheme = dark ? 'dark' : 'light';
        } else {
            root.classList.remove('modern');
            root.classList.remove('light');
            root.style.colorScheme = '';
        }
        // the charts restyle themselves (qzcharttheme.js); the first call comes before they exist
        if (window.qzChartTheme) {
            window.qzChartTheme.refresh();
        }
        // pages that draw with colours of their own (the workout editor, the workout preview
        // chips) redraw on this event
        var event = document.createEvent('Event');
        event.initEvent('qzthemechange', false, false);
        window.dispatchEvent(event);
    };

    // The accent of the theme with the given opacity, as "rgba(...)" - the fill of a selected chip
    // (the selected rows of the app: the accent at 0.14-0.24); null in the classic look or when
    // the accent is not "#rrggbb" ("#aarrggbb" from QML too)
    window.qzAccentTonal = function (alpha) {
        var root = document.documentElement;
        if (!root.classList.contains('modern'))
            return null;
        var hex = root.style.getPropertyValue('--qz-accent').trim();
        if (hex.charAt(0) !== '#' || (hex.length !== 7 && hex.length !== 9))
            return null;
        var rgb = hex.substring(hex.length - 6);
        var parts = [rgb.substring(0, 2), rgb.substring(2, 4), rgb.substring(4, 6)].map(function (h) {
            return parseInt(h, 16);
        });
        if (parts.some(isNaN))
            return null;
        return 'rgba(' + parts.join(',') + ',' + alpha + ')';
    };

    var theme = {};
    window.location.hash.substring(1).split('&').forEach(function (pair) {
        var i = pair.indexOf('=');
        if (i > 0) {
            theme[pair.substring(0, i)] = decodeURIComponent(pair.substring(i + 1));
        }
    });
    if (theme.modern === '1') {
        window.qzApplyTheme(theme);
    }
})();
