// Full screen charts (chartjs/chart.htm and the workout history, previewchart/chart.htm, with
// chartfullscreen.css): a button in the top-left corner of every chart card spreads the card over
// the whole page; there the shown charts go one after another with a swipe, and the button, the
// back key (ChartJsTest.qml, PreviewChart.qml: handleBack) or the back arrow bring the page back.
// The charts come through qzcharttheme.js (create, restyle), which calls prepare/chartChanged.
// A CSS overlay, not the Fullscreen API: that one does not work in the app's web view.
// The app learns the state from the page title (WebView.title): FS_TITLE while spread.
// Plain ES5, the syntax of dochartlive.js: the web view of Android 5.1 runs it.
(function () {
    var FS_TITLE = 'Line Chart #fullscreen';
    var ICON_EXPAND = 'M7 14H5v5h5v-2H7v-3zm-2-4h2V7h3V5H5v5zm12 7h-3v2h5v-5h-2v3zM14 5v2h3v3h2V5h-5z';
    var ICON_COLLAPSE = 'M5 16h3v3h2v-5H5v2zm3-8H5v2h5V5H8v3zm6 11h2v-3h3v-2h-5v5zm2-11V5h-2v5h5V8h-3z';
    var SWIPE_MIN_PX = 60;
    var SWIPE_MAX_MS = 800;
    var TOAST_MS = 3000;
    var BUTTON_ROW_PX = 36; // the height of the button (chartfullscreen.css)
    var BASE_FONT_PX = 12;  // Chart.js default font size, the one the page's charts use
    var FIRST_ANIMATION_MS = 1500; // Chart.js animates a new chart for 1000 ms

    var normalTitle = document.title;
    var current = null;     // the spread card
    var saved = [];         // [{obj, key, had, value}] the options changed for the big look
    var toastTimer = null;
    var touch = null;

    function tr(key, fallback) {
        return window.qzTranslate ? window.qzTranslate(key, fallback) : fallback;
    }

    // the cards in the order the page shows them: the workout history puts every chart in a
    // <section> and orders them with CSS order (Workout History Charts in the settings), not in
    // the markup, so the swipe and the "i / N" counter follow that order
    function cards() {
        var list = [];
        var canvases = document.querySelectorAll('canvas[id^="canvas"]');
        for (var i = 0; i < canvases.length; i++) {
            if (canvases[i].parentNode)
                list.push({ card: canvases[i].parentNode, order: sectionOrder(canvases[i]), index: i });
        }
        list.sort(function (a, b) { return a.order - b.order || a.index - b.index; });
        return list.map(function (item) { return item.card; });
    }

    // CSS order of the <section> around the element; 0 without one (the live charts page)
    function sectionOrder(el) {
        for (; el && el !== document.body; el = el.parentNode) {
            if (el.tagName === 'SECTION')
                return Number(window.getComputedStyle(el).order) || 0;
        }
        return 0;
    }

    // a card the page shows: not inside a hidden box (display: none, or height 0 as the heart
    // chart without a sensor), and its chart is made
    function shown(card) {
        if (card !== current && card.offsetParent === null)
            return false;
        for (var el = card; el && el !== document.body; el = el.parentNode) {
            var style = el.style;
            if (style && (style.display === 'none' || style.height === '0' || style.height === '0px'))
                return false;
        }
        return !!chartOf(card);
    }

    function chartOf(card) {
        var canvas = card.querySelector('canvas');
        return canvas && window.Chart && Chart.getChart ? Chart.getChart(canvas) : null;
    }

    function iconSvg(path) {
        return '<svg viewBox="0 0 24 24" width="24" height="24" aria-hidden="true"><path fill="currentColor" d="' +
               path + '"/></svg>';
    }

    function updateButton(card) {
        var button = card.querySelector('.qz-fs-button');
        if (!button)
            return;
        var on = card === current;
        button.innerHTML = iconSvg(on ? ICON_COLLAPSE : ICON_EXPAND);
        button.setAttribute('aria-label', on ? tr('chart.exitFullScreen', 'Exit full screen')
                                             : tr('chart.fullScreen', 'Full screen'));
    }

    function addButtons() {
        cards().forEach(function (card) {
            if (card.querySelector('.qz-fs-button'))
                return;
            card.classList.add('qz-fs-card');
            var button = document.createElement('button');
            button.type = 'button';
            button.className = 'qz-fs-button';
            // not in the summary picture of the workout mail (html2canvas)
            button.setAttribute('data-html2canvas-ignore', 'true');
            // not while the chart first grows: the end of that animation saves its picture for
            // the mail (animation.onComplete), and a spread chart would go there big
            var readyAt = Date.now() + FIRST_ANIMATION_MS;
            button.addEventListener('click', function (e) {
                e.stopPropagation();
                if (Date.now() < readyAt)
                    return;
                if (current === card)
                    exit();
                else
                    enter(card, true);
            });
            card.appendChild(button);
            updateButton(card);
        });
    }

    // The changes are kept as paths from the options root, not as the objects: Chart.js
    // merges the scales into new objects on every update, the old ones are not drawn any more
    function setPath(options, path, value) {
        var obj = options;
        for (var i = 0; i < path.length - 1; i++) {
            if (!obj[path[i]] || typeof obj[path[i]] !== 'object')
                obj[path[i]] = {};
            obj = obj[path[i]];
        }
        var key = path[path.length - 1];
        saved.push({ path: path, had: Object.prototype.hasOwnProperty.call(obj, key), value: obj[key] });
        obj[key] = value;
    }

    function restoreOptions(options) {
        for (var i = saved.length - 1; i >= 0 && options; i--) {
            var s = saved[i];
            var obj = options;
            for (var j = 0; obj && j < s.path.length - 1; j++)
                obj = obj[s.path[j]];
            if (!obj || typeof obj !== 'object')
                continue;
            var key = s.path[s.path.length - 1];
            if (s.had)
                obj[key] = s.value;
            else
                delete obj[key];
        }
        saved = [];
    }

    // the spread chart fills the screen (no aspect ratio) with fonts readable from the
    // treadmill. Works on the raw options (chart.config.options, as qzcharttheme.js does),
    // only the sizes: the theme keeps its colours and weights in the same objects
    function bigOptions(options) {
        var side = Math.min(window.innerWidth, window.innerHeight);
        var tick = Math.max(14, Math.min(28, Math.round(side / 24)));
        setPath(options, ['maintainAspectRatio'], false);
        var scales = options.scales || {};
        Object.keys(scales).forEach(function (id) {
            var scale = scales[id];
            if (!scale || typeof scale !== 'object')
                return;
            setPath(options, ['scales', id, 'ticks', 'font', 'size'], tick);
            // zone names drawn inside the chart (a negative padding in px) grow with the font
            var padding = scale.ticks && scale.ticks.padding; // raw options: ticks may be missing
            if (typeof padding === 'number' && padding < 0)
                setPath(options, ['scales', id, 'ticks', 'padding'], Math.round(padding * tick / BASE_FONT_PX));
            if (scale.title && scale.title.display)
                setPath(options, ['scales', id, 'title', 'font', 'size'], tick);
        });
        setPath(options, ['plugins', 'title', 'font', 'size'], Math.round(tick * 1.2));
        setPath(options, ['plugins', 'legend', 'labels', 'font', 'size'], tick);
        // the values under a held finger, readable as the axes
        setPath(options, ['plugins', 'tooltip', 'titleFont', 'size'], tick);
        setPath(options, ['plugins', 'tooltip', 'bodyFont', 'size'], tick);
    }

    // the button sits in the title row: a taller row keeps it off the top label of the y axis
    function roomForButton(options) {
        var title = options.plugins && options.plugins.title;
        if (title && title.display)
            title.padding = { top: BUTTON_ROW_PX - 22, bottom: 12 };
        else {
            if (!options.layout)
                options.layout = {};
            options.layout.padding = { top: BUTTON_ROW_PX };
        }
    }

    function redraw(chart) {
        if (!chart)
            return;
        // update first: resize() sizes by the options resolved at the last update
        chart.update('none');
        chart.resize();
    }

    function showToast(text) {
        var toast = document.getElementById('qz_fs_toast');
        if (!toast) {
            toast = document.createElement('div');
            toast.id = 'qz_fs_toast';
            toast.setAttribute('data-html2canvas-ignore', 'true');
            document.body.appendChild(toast);
        }
        toast.textContent = text;
        toast.classList.add('on');
        clearTimeout(toastTimer);
        toastTimer = setTimeout(function () { toast.classList.remove('on'); }, TOAST_MS);
    }

    function hideToast() {
        var toast = document.getElementById('qz_fs_toast');
        clearTimeout(toastTimer);
        if (toast)
            toast.classList.remove('on');
    }

    function updateCounter() {
        var counter = document.getElementById('qz_fs_counter');
        if (!counter) {
            counter = document.createElement('div');
            counter.id = 'qz_fs_counter';
            counter.setAttribute('data-html2canvas-ignore', 'true');
            document.body.appendChild(counter);
        }
        var list = cards().filter(shown);
        var i = list.indexOf(current);
        counter.textContent = current && list.length > 1 ? (i + 1) + ' / ' + list.length : '';
    }

    function enter(card, hint) {
        if (current && current !== card)
            leave(current);
        current = card;
        card.classList.add('qz-fs-on');
        document.documentElement.classList.add('qz-fs');
        var chart = chartOf(card);
        if (chart)
            bigOptions(chart.config.options);
        redraw(chart);
        updateButton(card);
        updateCounter();
        document.title = FS_TITLE;
        if (hint && cards().filter(shown).length > 1)
            showToast(tr('chart.swipeHint', 'Swipe left or right to switch charts'));
    }

    // the card back in the page, the options as the page made them
    function leave(card) {
        card.classList.remove('qz-fs-on');
        current = null;
        var chart = chartOf(card);
        restoreOptions(chart && chart.config.options);
        redraw(chart);
        updateButton(card);
    }

    function exit() {
        if (!current)
            return;
        var card = current;
        leave(card);
        document.documentElement.classList.remove('qz-fs');
        hideToast();
        updateCounter();
        document.title = normalTitle;
        // back where it was in the page, not at the top
        if (card.scrollIntoView)
            card.scrollIntoView();
    }

    function step(delta) {
        if (!current)
            return;
        var list = cards().filter(shown);
        if (list.length < 2)
            return;
        var i = list.indexOf(current);
        var next = list[(i + delta + list.length) % list.length];
        enter(next, false);
    }

    document.addEventListener('touchstart', function (e) {
        if (!current || e.touches.length !== 1) {
            touch = null;
            return;
        }
        touch = { x: e.touches[0].clientX, y: e.touches[0].clientY, t: Date.now() };
    }, true);

    document.addEventListener('touchend', function (e) {
        if (!current || !touch || !e.changedTouches.length)
            return;
        var dx = e.changedTouches[0].clientX - touch.x;
        var dy = e.changedTouches[0].clientY - touch.y;
        var quick = Date.now() - touch.t < SWIPE_MAX_MS;
        touch = null;
        if (quick && Math.abs(dx) >= SWIPE_MIN_PX && Math.abs(dx) > 1.5 * Math.abs(dy))
            step(dx < 0 ? 1 : -1);
    }, true);

    // a turn of the screen: the new size and the fonts for it
    window.addEventListener('resize', function () {
        if (!current)
            return;
        var chart = chartOf(current);
        if (!chart)
            return;
        restoreOptions(chart.config.options);
        bigOptions(chart.config.options);
        redraw(chart);
    });

    document.addEventListener('qz-translations-updated', function () {
        cards().forEach(updateButton);
    });

    window.qzChartFullscreen = {
        // qzcharttheme.js before it makes or refreshes the chart of a canvas: a live refresh brings
        // new options, the spread chart gets the big look on them before it is drawn (the old
        // options are dropped with what was saved of them)
        prepare: function (canvas, options) {
            if (!options)
                return;
            roomForButton(options);
            if (current && canvas && canvas.parentNode === current) {
                saved = [];
                bigOptions(options);
            }
        },
        // and after: a new chart may be the first one of its card
        chartChanged: function () {
            addButtons();
            if (current)
                updateCounter();
        },
        // the app's back (ChartJsTest.qml, PreviewChart.qml): true when it closed the full screen
        exit: function () {
            var was = !!current;
            exit();
            return was;
        },
        active: function () {
            return !!current;
        }
    };
})();
