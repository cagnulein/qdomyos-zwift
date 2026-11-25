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

    // A list longer than the room: the bottom at the middle of an item, so the cut item shows
    // that the list goes on (a whole item at the edge looked like its end)
    function fitLong(menu, room) {
        if (menu.scrollHeight <= room)
            return;
        var cut = 0;
        for (var i = 0; i < menu.children.length; i++) {
            var item = menu.children[i];
            var middle = item.offsetTop + item.offsetHeight / 2;
            if (middle > room)
                break;
            cut = middle;
        }
        if (cut > 0)
            menu.style.maxHeight = cut + 'px';
    }

    // The selected item brought into view ends the menu on a whole item again: half an item
    // further, so the next one shows cut as well (not past the end of the list)
    function keepCut(menu) {
        var range = menu.scrollHeight - menu.clientHeight;
        if (range <= 0 || menu.scrollTop >= range)
            return;
        var bottom = menu.scrollTop + menu.clientHeight;
        for (var i = 1; i < menu.children.length; i++) {
            var item = menu.children[i];
            if (Math.abs(item.offsetTop - bottom) < 2) {
                menu.scrollTop = Math.min(range, menu.scrollTop + item.offsetHeight / 2);
                return;
            }
        }
    }

    // A scroll bar while the menu is open and the list does not fit: Android shows its own only
    // while the list moves. Over the menu, so the menu's scroll keeps it in place
    function addBar(menu) {
        if (menu.scrollHeight <= menu.clientHeight)
            return;
        var bar = document.createElement('div');
        bar.className = 'qz-menu-bar';
        backdrop.appendChild(bar);
        var inset = 4;
        function place() {
            var r = menu.getBoundingClientRect();
            var track = r.height - 2 * inset;
            var length = Math.max(24, track * menu.clientHeight / menu.scrollHeight);
            var range = menu.scrollHeight - menu.clientHeight;
            bar.style.left = (r.right - inset - bar.offsetWidth) + 'px';
            bar.style.top = (r.top + inset + (track - length) * (range > 0 ? menu.scrollTop / range : 0)) + 'px';
            bar.style.height = length + 'px';
        }
        menu.addEventListener('scroll', place);
        place();
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
        fitLong(menu, room);
        var height = Math.min(menu.offsetHeight, room);
        var top = Math.min(Math.max(margin, r.top), window.innerHeight - margin - height);
        menu.style.top = top + 'px';
        var current = menu.querySelector('.selected');
        if (current) {
            current.scrollIntoView({ block: 'nearest' });
            keepCut(menu);
        }
        addBar(menu);
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
