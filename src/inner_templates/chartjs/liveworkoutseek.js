/* Long-press seek on the existing live chart. The selected position is only
 * applied after explicit confirmation and a second server-side state check. */
(function () {
    'use strict';
    let pointer = null, holdTimer = null, pending = false, active = false;
    let chosen = 0, sessionId = '', rows = [], durations = [], downX = 0, downY = 0;
    let marker, balloon, dialog;
    const tr = (key, fallback) => window.qzTranslate ? window.qzTranslate(key, fallback) : fallback;

    function request(msg, content) {
        return new MainWSQueueElement({msg: msg, content: content || {}}, function (reply) {
            return reply.msg === 'R_' + msg ? (reply.content || {}) : null;
        }, 4000, 1).enqueue();
    }
    function initElements() {
        if (marker) return;
        marker = document.createElement('div');
        balloon = document.createElement('div');
        dialog = document.createElement('div');
        marker.style.cssText = 'display:none;position:fixed;width:2px;background:#ff9700;z-index:9997;pointer-events:none';
        balloon.style.cssText = 'display:none;position:fixed;z-index:9998;background:#222;color:white;padding:8px;border-radius:5px;font:14px sans-serif;pointer-events:none;white-space:pre-line';
        dialog.style.cssText = 'display:none;position:fixed;inset:0;background:rgba(0,0,0,.7);z-index:9999;align-items:center;justify-content:center';
        const box = document.createElement('div');
        box.style.cssText = 'background:#fff;color:#222;padding:18px;border-radius:8px;text-align:center;max-width:85%;font:16px sans-serif';
        const label = document.createElement('p');
        label.textContent = tr('workout.resumeFromHereConfirm', 'Resume the workout from this point?');
        const cancel = document.createElement('button');
        cancel.textContent = tr('common.cancel', 'Cancel');
        const confirm = document.createElement('button');
        confirm.textContent = tr('common.confirm', 'Confirm');
        cancel.style.marginRight = '12px';
        cancel.onclick = hide;
        confirm.onclick = function () {
            const seconds = chosen, workoutId = sessionId;
            hide();
            request('setliveseek', {seconds: seconds, workoutId: workoutId}).then(function (result) {
                if (!result.applied) window.alert(tr('workout.seekUnavailable', 'This workout cannot be resumed from that point.'));
            }).catch(function () { window.alert(tr('workout.seekUnavailable', 'This workout cannot be resumed from that point.')); });
        };
        box.append(label, cancel, confirm);
        dialog.appendChild(box);
        document.body.append(marker, balloon, dialog);
    }
    function hide() {
        if (marker) marker.style.display = balloon.style.display = dialog.style.display = 'none';
        active = false;
        pending = false;
        sessionId = '';
    }
    function timeText(n) {
        const h = Math.floor(n / 3600), m = Math.floor(n / 60) % 60, s = n % 60;
        return (h ? h + ':' + String(m).padStart(2, '0') : String(m)) + ':' + String(s).padStart(2, '0');
    }
    function rowAt(second) {
        let pos = 0;
        for (let i = 0; i < rows.length; ++i) {
            if (second < pos + durations[i]) return {row: rows[i], offset: second - pos, duration: durations[i]};
            pos += durations[i];
        }
        return null;
    }
    function showPosition(event, canvas, chart) {
        const rect = canvas.getBoundingClientRect(), scale = chart.scales.x;
        const total = durations.reduce((a, b) => a + b, 0);
        if (!scale || total <= 0) return;
        const x = Math.min(rect.right, Math.max(rect.left, event.clientX));
        chosen = Math.max(0, Math.min(total - 1, Math.round(scale.getValueForPixel(x - rect.left))));
        const selected = rowAt(chosen), row = selected && selected.row, lines = [timeText(chosen)];
        if (row) {
            if (Number(row.speed) >= 0) lines.push(tr('workoutEditor.speed', 'Speed') + ': ' + (Number(row.speed) * (typeof miles === 'number' ? miles : 1)).toFixed(1) + (typeof miles === 'number' && miles !== 1 ? ' mph' : ' km/h'));
            if (row.inclination !== undefined && Number(row.inclination) >= -50) lines.push(tr('workoutEditor.incline', 'Incline') + ': ' + row.inclination + '%');
            if (Number(row.powerfrom) >= 0 && Number(row.powerto) >= 0) {
                const fraction = selected.duration > 1 ? selected.offset / (selected.duration - 1) : 0;
                const power = Number(row.powerfrom) + (Number(row.powerto) - Number(row.powerfrom)) * fraction;
                lines.push(tr('workoutEditor.power', 'Power') + ': ' + Math.round(power) + ' W');
            } else if (Number(row.powerzonefrom) >= 0 && Number(row.powerzoneto) >= 0) {
                const fraction = selected.duration > 1 ? selected.offset / (selected.duration - 1) : 0;
                const percent = Number(row.powerzonefrom) + (Number(row.powerzoneto) - Number(row.powerzonefrom)) * fraction;
                lines.push(tr('workoutEditor.power', 'Power') + ': ' + Math.round(percent) + '% FTP');
            } else if (Number(row.power) >= 0) lines.push(tr('workoutEditor.power', 'Power') + ': ' + row.power + ' W');
            if (Number(row.cadence) >= 0) lines.push(tr('workoutEditor.cadence', 'Cadence') + ': ' + row.cadence + ' rpm');
            if (Number(row.resistance) >= 0) lines.push(tr('workoutEditor.resistance', 'Resistance') + ': ' + row.resistance);
        }
        marker.style.left = x + 'px'; marker.style.top = rect.top + 'px'; marker.style.height = rect.height + 'px';
        marker.style.display = 'block';
        balloon.textContent = lines.join('\n'); balloon.style.display = 'block';
        balloon.style.left = Math.max(0, Math.min(window.innerWidth - balloon.offsetWidth, x - balloon.offsetWidth / 2)) + 'px';
        balloon.style.top = Math.max(0, rect.top + 5) + 'px';
    }
    function install() {
        const canvas = document.getElementById('canvas');
        if (!canvas) return;
        initElements();
        canvas.addEventListener('pointerdown', function (event) {
            if (pointer !== null || dialog.style.display !== 'none') return;
            pointer = event.pointerId;
            const startX = event.clientX, startY = event.clientY;
            downX = startX; downY = startY;
            holdTimer = setTimeout(async function () {
                holdTimer = null;
                const heldPointer = pointer;
                if (heldPointer === null) return;
                pending = true;
                try {
                    const state = await request('getliveseekstate');
                    if (!state.supported || pointer !== heldPointer) { pending = false; return; }
                    const program = await request('gettrainingprogram');
                    if (pointer !== heldPointer || !Array.isArray(program.list)) { pending = false; return; }
                    rows = program.list;
                    durations = rows.map(row => Math.max(0, Math.round(Number(row.duration_s) || 0)));
                    if (!durations.length || durations.some(d => d <= 0)) { pending = false; return; }
                    const chart = Chart.getChart(canvas);
                    if (!chart) { pending = false; return; }
                    sessionId = state.workoutId;
                    pending = false; active = true;
                    canvas.setPointerCapture(heldPointer);
                    showPosition({clientX: startX, clientY: startY}, canvas, chart);
                } catch (err) { pending = false; }
            }, 650);
        });
        canvas.addEventListener('pointermove', function (event) {
            if (event.pointerId !== pointer) return;
            if (active) {
                event.preventDefault();
                const chart = Chart.getChart(canvas);
                if (chart) showPosition(event, canvas, chart);
            } else if (holdTimer && Math.hypot(event.clientX - downX, event.clientY - downY) > 8) {
                // Keep the existing chart's pan/zoom behaviour for ordinary gestures.
                clearTimeout(holdTimer); holdTimer = null;
            }
        });
        canvas.addEventListener('pointerup', function (event) {
            if (event.pointerId !== pointer) return;
            if (holdTimer) clearTimeout(holdTimer);
            holdTimer = null; pointer = null;
            if (active) { event.preventDefault(); dialog.style.display = 'flex'; }
            else pending = false;
        });
        canvas.addEventListener('pointercancel', function () {
            if (holdTimer) clearTimeout(holdTimer);
            holdTimer = null; pointer = null; hide();
        });
    }
    window.addEventListener('load', install);
})();
