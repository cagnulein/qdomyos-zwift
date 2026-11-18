// Modern look of the app on its web pages (charts, workout history, maps). The QML page passes
// the theme colours in the URL fragment (#modern=1&dark=1&bg=...) for the first load
// (window.ui.webThemeFragment() in main.qml) and calls qzApplyTheme() when the theme changes
// later (window.ui.webThemeScript()). Without the fragment - the classic look - the page keeps
// its own look and nothing here changes it.
// The page CSS reads the colours as variables (--qz-bg, --qz-surface, ...) under html.modern,
// html.modern.light for the light theme. Loaded in <head>, before the page draws.
(function () {
    var VARS = {
        bg: '--qz-bg', surface: '--qz-surface', surfaceHigh: '--qz-surface-high',
        outline: '--qz-outline', text: '--qz-text', muted: '--qz-muted', accent: '--qz-accent'
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
