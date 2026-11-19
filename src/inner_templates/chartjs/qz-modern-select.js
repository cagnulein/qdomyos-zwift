// Modern look of the app: drop-down lists of the web pages as a menu of the page itself.
// Android draws the list of a <select> as a system dialog in the theme of the activity - a
// white full-screen list, whatever the page looks like. Under html.modern the selects take no
// taps (qz-modern.css); a tap on one opens this menu under it in the colours of the page, like
// the combo boxes of the QML pages. The choice goes into the select with a change event, so
// the page code does not change. The classic look keeps the system list.
(function () {
    'use strict';

    // qz-modern.css takes the taps off the selects only on a page marked so
    document.documentElement.classList.add('qz-select-menus');

    var backdrop = null;

    function close() {
        if (backdrop) {
            backdrop.remove();
            backdrop = null;
        }
    }

    function inside(r, x, y) {
        return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom;
    }

    // The point is not cut off by a scrolling box around the select
    function unclipped(sel, x, y) {
        for (var el = sel.parentElement; el && el !== document.body; el = el.parentElement) {
            if (getComputedStyle(el).overflowY !== 'visible' && !inside(el.getBoundingClientRect(), x, y))
                return false;
        }
        return true;
    }

    // The select under the tap: it takes no taps itself, so the event lands on its parent. The
    // tap target has to hold it - a dialog or a header lying over the field got the tap itself
    function selectAt(target, x, y) {
        var selects = document.querySelectorAll('select');
        for (var i = 0; i < selects.length; i++) {
            var sel = selects[i];
            if (sel.disabled || sel.offsetParent === null)
                continue;
            if (inside(sel.getBoundingClientRect(), x, y) && target instanceof Node
                    && target.contains(sel) && unclipped(sel, x, y))
                return sel;
        }
        return null;
    }

    // The select of a label tapped beside it (its caption above the field): the label would
    // focus the select, and WebKit of iOS opens its own list on that
    function labelSelect(target) {
        var label = target instanceof Element ? target.closest('label') : null;
        var sel = label ? (label.control || label.querySelector('select')) : null;
        if (!sel || sel.tagName !== 'SELECT' || sel.disabled || sel.offsetParent === null)
            return null;
        return sel;
    }

    function open(sel) {
        close();
        backdrop = document.createElement('div');
        backdrop.className = 'qz-menu-backdrop';
        var menu = document.createElement('div');
        menu.className = 'qz-menu';
        menu.setAttribute('role', 'listbox');

        Array.prototype.forEach.call(sel.options, function (opt) {
            if (opt.hidden)
                return;
            var item = document.createElement('div');
            item.className = 'qz-menu-item' + (opt.selected ? ' selected' : '') + (opt.disabled ? ' disabled' : '');
            item.setAttribute('role', 'option');
            item.setAttribute('aria-selected', opt.selected ? 'true' : 'false');
            item.textContent = opt.textContent;
            if (!opt.disabled) {
                item.addEventListener('click', function (e) {
                    e.stopPropagation();
                    close();
                    if (sel.value !== opt.value) {
                        sel.value = opt.value;
                        sel.dispatchEvent(new Event('input', { bubbles: true }));
                        sel.dispatchEvent(new Event('change', { bubbles: true }));
                    }
                });
            }
            menu.appendChild(item);
        });

        backdrop.addEventListener('click', close);
        backdrop.appendChild(menu);
        document.body.appendChild(backdrop);

        // Over the field, its top at the top of the field, as the lists of the settings open
        // (UiComboBox); moved up when the room below is short; as wide as the field
        var r = sel.getBoundingClientRect();
        var margin = 12;
        var width = Math.max(r.width, 160);
        var left = Math.min(Math.max(margin, r.left), window.innerWidth - width - margin);
        menu.style.left = left + 'px';
        menu.style.width = width + 'px';
        var room = window.innerHeight - 2 * margin;
        menu.style.maxHeight = room + 'px';
        var height = Math.min(menu.offsetHeight, room);
        var top = Math.min(Math.max(margin, r.top), window.innerHeight - margin - height);
        menu.style.top = top + 'px';
        var current = menu.querySelector('.selected');
        if (current)
            current.scrollIntoView({ block: 'nearest' });
    }

    document.addEventListener('click', function (e) {
        if (!document.documentElement.classList.contains('modern') || backdrop)
            return;
        var sel = selectAt(e.target, e.clientX, e.clientY) || labelSelect(e.target);
        if (!sel)
            return;
        // A label around the select would focus it: the menu takes its place
        e.preventDefault();
        e.stopPropagation();
        open(sel);
    }, true);

    window.addEventListener('resize', close);
    // a new theme or the modern look switched off: the page is drawn again under the menu
    window.addEventListener('qzthemechange', close);
    document.addEventListener('keydown', function (e) {
        if (backdrop && (e.key === 'Escape' || e.key === 'Esc')) {
            e.preventDefault();
            close();
        }
    }, true);
    // Back of the app (WorkoutEditor.qml, handleBack): closes an open menu first, as it closes
    // the system list; false when there was none
    window.qzMenuBack = function () {
        if (!backdrop)
            return false;
        close();
        return true;
    };
    // The menu stays where it opened: the page moving under it closes it
    // (the menu's own scroll of a long list excepted)
    window.addEventListener('scroll', function (e) {
        if (backdrop && !(e.target instanceof Node && backdrop.contains(e.target)))
            close();
    }, true);
})();
