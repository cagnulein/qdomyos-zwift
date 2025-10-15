(function () {
    const state = {
        miles: false,
        device: 'treadmill',
        intervals: [],
        programs: [],
        showAdvanced: false,
        lastSaved: '',
        loading: false
    };

    const selectors = {};

    const FIELD_DEFS = [
        { key: 'name', label: 'Label', type: 'text', group: 'basic', devices: 'all' },
        { key: 'duration', label: 'Duration', type: 'duration', group: 'basic', devices: 'all' },
        { key: 'speed', label: 'Speed', type: 'number', unitKey: 'speed', step: 0.1, min: 0, group: 'basic', devices: ['treadmill'] },
        { key: 'inclination', label: 'Incline', type: 'number', unitSuffix: '%', step: 0.5, min: -10, max: 30, group: 'basic', devices: ['treadmill', 'elliptical'] },
        { key: 'resistance', label: 'Resistance', type: 'number', step: 1, min: 0, max: 100, group: 'basic', devices: ['bike', 'elliptical'] },
        { key: 'cadence', label: 'Cadence', type: 'number', unitSuffix: 'rpm', min: 0, max: 240, group: 'basic', devices: ['bike', 'elliptical', 'rower'] },
        { key: 'power', label: 'Power', type: 'number', unitSuffix: 'W', min: 0, max: 2000, group: 'basic', devices: ['bike', 'rower'] },
        { key: 'fanspeed', label: 'Fan', type: 'number', min: 0, max: 8, group: 'advanced', devices: 'all' },
        { key: 'forcespeed', label: 'Force Speed', type: 'bool', group: 'advanced', devices: ['treadmill'] },
        { key: 'requested_peloton_resistance', label: 'Peloton Res.', type: 'number', min: -1, max: 100, group: 'advanced', devices: ['bike'] },
        { key: 'loopTimeHR', label: 'HR Loop (s)', type: 'number', min: 1, max: 60, group: 'advanced', devices: 'all' },
        { key: 'zoneHR', label: 'HR Zone', type: 'number', min: -1, max: 5, group: 'advanced', devices: 'all' },
        { key: 'HRmin', label: 'HR Min', type: 'number', min: -1, max: 240, group: 'advanced', devices: 'all' },
        { key: 'HRmax', label: 'HR Max', type: 'number', min: -1, max: 240, group: 'advanced', devices: 'all' },
        { key: 'minSpeed', label: 'Min Speed', type: 'number', unitKey: 'speed', group: 'advanced', devices: ['treadmill', 'bike'] },
        { key: 'maxSpeed', label: 'Max Speed', type: 'number', unitKey: 'speed', group: 'advanced', devices: ['treadmill', 'bike'] },
        { key: 'maxResistance', label: 'Max Resistance', type: 'number', min: -1, max: 100, group: 'advanced', devices: ['bike', 'elliptical'] },
        { key: 'mets', label: 'METS', type: 'number', min: -1, max: 40, group: 'advanced', devices: 'all' }
    ];

    const SERIES_DEFS = {
        treadmill: [
            { key: 'speed', label: () => `Speed (${state.miles ? 'mph' : 'km/h'})`, color: '#42a5f5', unit: () => state.miles ? 'mph' : 'km/h', axis: 'speedAxis', axisLabel: () => state.miles ? 'Speed (mph)' : 'Speed (km/h)', axisPosition: 'left' },
            { key: 'inclination', label: () => 'Incline (%)', color: '#26c6da', unit: () => '%', axis: 'inclineAxis', axisLabel: () => 'Incline (%)', axisPosition: 'right' }
        ],
        bike: [
            { key: 'resistance', label: () => 'Resistance', color: '#ab47bc', unit: () => 'lvl', axis: 'resistanceAxis', axisLabel: () => 'Resistance', axisPosition: 'left' },
            { key: 'cadence', label: () => 'Cadence (rpm)', color: '#29b6f6', unit: () => 'rpm', axis: 'cadenceAxis', axisLabel: () => 'Cadence (rpm)', axisPosition: 'right' },
            { key: 'power', label: () => 'Power (W)', color: '#ef6c00', unit: () => 'W', axis: 'powerAxis', axisLabel: () => 'Power (W)', axisPosition: 'left' }
        ],
        elliptical: [
            { key: 'resistance', label: () => 'Resistance', color: '#7e57c2', unit: () => 'lvl', axis: 'resistanceAxis', axisLabel: () => 'Resistance', axisPosition: 'left' },
            { key: 'inclination', label: () => 'Ramp (%)', color: '#66bb6a', unit: () => '%', axis: 'inclineAxis', axisLabel: () => 'Ramp (%)', axisPosition: 'right' }
        ],
        rower: [
            { key: 'power', label: () => 'Power (W)', color: '#fb8c00', unit: () => 'W', axis: 'powerAxis', axisLabel: () => 'Power (W)', axisPosition: 'left' },
            { key: 'cadence', label: () => 'Stroke Rate', color: '#26a69a', unit: () => 'spm', axis: 'cadenceAxis', axisLabel: () => 'Strokes/min', axisPosition: 'right' }
        ]
    };

    document.addEventListener('DOMContentLoaded', () => {
        cacheDom();
        bindEvents();
        bootstrap();
        if (window.QZ_OFFLINE) {
            announce('Offline mode: load/save/start disabled', true);
            updateControls();
        }
    });

    function cacheDom() {
        selectors.name = document.getElementById('workoutName');
        selectors.device = document.getElementById('deviceSelect');
        selectors.advanced = document.getElementById('advancedToggle');
        selectors.intervalList = document.getElementById('intervalList');
        selectors.addInterval = document.getElementById('addInterval');
        selectors.repeatSelection = document.getElementById('repeatSelection');
        selectors.clearIntervals = document.getElementById('clearIntervals');
        selectors.newWorkout = document.getElementById('newWorkout');
        selectors.saveWorkout = document.getElementById('saveWorkout');
        selectors.saveStartWorkout = document.getElementById('saveStartWorkout');
        selectors.programSelect = document.getElementById('programSelect');
        selectors.loadProgram = document.getElementById('loadProgram');
        selectors.refreshPrograms = document.getElementById('refreshPrograms');
        selectors.statusDuration = document.getElementById('statusDuration');
        selectors.statusIntervals = document.getElementById('statusIntervals');
        selectors.statusMessage = document.getElementById('statusMessage');
        selectors.offlineBanner = document.getElementById('offlineBanner');
    }

    function bindEvents() {
        selectors.device.addEventListener('change', () => {
            if (window.QZ_OFFLINE) {
                state.device = selectors.device.value;
                renderIntervals();
                updateChart();
                return;
            }
            setDevice(selectors.device.value);
        });
        selectors.advanced.addEventListener('change', () => {
            state.showAdvanced = selectors.advanced.checked;
            renderIntervals();
            updateChart();
        });
        selectors.addInterval.addEventListener('click', () => {
            addInterval();
            announce('Interval added');
        });
        selectors.repeatSelection.addEventListener('click', repeatSelection);
        selectors.clearIntervals.addEventListener('click', () => {
            if (state.intervals.length && !confirm('Remove all intervals?')) {
                return;
            }
            state.intervals = [];
            addInterval();
            renderIntervals();
            updateChart();
            updateStatus();
            updateControls();
        });
        selectors.newWorkout.addEventListener('click', () => {
            state.lastSaved = '';
            selectors.name.value = '';
            state.intervals = [];
            addInterval();
            renderIntervals();
            updateChart();
            updateStatus();
            updateControls();
            announce('New workout ready');
        });
        selectors.saveWorkout.addEventListener('click', () => saveWorkflow(false));
        selectors.saveStartWorkout.addEventListener('click', () => saveWorkflow(true));
        selectors.loadProgram.addEventListener('click', () => {
            if (window.QZ_OFFLINE) {
                announce('Offline: cannot load workouts', true);
                return;
            }
            const name = selectors.programSelect.value;
            if (!name) {
                announce('Select a workout to load', true);
                return;
            }
            loadProgram(name);
        });
        selectors.refreshPrograms.addEventListener('click', () => {
            if (window.QZ_OFFLINE) {
                announce('Offline: cannot refresh list', true);
                return;
            }
            refreshProgramList();
        });
    }

    function bootstrap() {
        Promise.allSettled([fetchEnvironment(), refreshProgramList()]).then(() => {
            if (!state.intervals.length) {
                addInterval();
            }
            renderIntervals();
            updateChart();
            updateStatus();
            updateControls();
        });
    }

    function fetchEnvironment() {
        if (window.QZ_OFFLINE) {
            return Promise.resolve();
        }
        return sendMessage('workouteditor_env', {}, 'R_workouteditor_env').then(content => {
            if (!content) {
                return;
            }
            state.miles = !!content.miles;
            if (content.device) {
                state.device = content.device;
            }
            selectors.device.value = state.device;
        }).catch(err => {
            console.error(err);
            announce('Environment not available', true);
        });
    }

    function refreshProgramList() {
        if (window.QZ_OFFLINE) {
            state.programs = [];
            renderProgramOptions();
            updateControls();
            return Promise.resolve();
        }
        return sendMessage('loadtrainingprograms', '', 'R_loadtrainingprograms').then(content => {
            state.programs = Array.isArray(content && content.list) ? content.list : [];
            renderProgramOptions();
            updateControls();
        }).catch(err => {
            console.error(err);
            announce('Cannot load program list', true);
        });
    }

    function renderProgramOptions() {
        selectors.programSelect.innerHTML = '';
        const placeholder = document.createElement('option');
        placeholder.value = '';
        placeholder.textContent = state.programs.length ? 'Select saved workout' : 'No saved workouts';
        selectors.programSelect.appendChild(placeholder);
        state.programs.sort((a, b) => a.localeCompare(b));
        state.programs.forEach(name => {
            const opt = document.createElement('option');
            opt.value = name;
            opt.textContent = name;
            selectors.programSelect.appendChild(opt);
        });
        if (state.lastSaved && state.programs.includes(state.lastSaved)) {
            selectors.programSelect.value = state.lastSaved;
        }
    }

    function loadProgram(name) {
        setWorking(true);
        sendMessage('loadtrainingprograms', name, 'R_loadtrainingprograms').then(content => {
            const rows = Array.isArray(content && content.list) ? content.list : [];
            if (!rows.length) {
                announce('Workout is empty or cannot be read', true);
                return;
            }
            state.intervals = rows.map((row, idx) => convertRow(row, idx));
            state.device = detectDevice(state.intervals) || state.device;
            selectors.device.value = state.device;
            selectors.name.value = name;
            state.lastSaved = name;
            renderIntervals();
            updateChart();
            updateStatus();
            updateControls();
            announce(`Loaded ${name}`);
        }).catch(err => {
            console.error(err);
            announce('Unable to load workout', true);
        }).finally(() => setWorking(false));
    }

    function convertRow(row, idx) {
        const out = {};
        out.name = row.name || `Interval ${idx + 1}`;
        if (!out.name || out.name === 'null') {
            out.name = `Interval ${idx + 1}`;
        }
        if (row.duration && typeof row.duration === 'string') {
            out.duration = row.duration;
        } else if (typeof row.duration_s === 'number') {
            out.duration = formatDuration(row.duration_s);
        } else {
            out.duration = '00:05:00';
        }
        FIELD_DEFS.forEach(def => {
            if (def.key === 'name' || def.key === 'duration') {
                return;
            }
            if (row[def.key] !== undefined && row[def.key] !== null) {
                if (def.type === 'bool') {
                    out[def.key] = row[def.key] === true || row[def.key] === 1;
                } else if (def.type === 'number') {
                    out[def.key] = Number(row[def.key]);
                } else {
                    out[def.key] = row[def.key];
                }
            }
        });
        out.__selected = false;
        return out;
    }

    function detectDevice(rows) {
        for (const row of rows) {
            if (row.speed !== undefined || row.inclination !== undefined) {
                return 'treadmill';
            }
        }
        for (const row of rows) {
            if (row.resistance !== undefined || row.maxResistance !== undefined) {
                if (row.inclination !== undefined) {
                    return 'elliptical';
                }
                return 'bike';
            }
        }
        for (const row of rows) {
            if (row.power !== undefined) {
                if (row.cadence !== undefined && !rows.some(r => r.resistance !== undefined)) {
                    return 'rower';
                }
                return 'bike';
            }
        }
        return null;
    }

    function setDevice(key) {
        if (!key) {
            return;
        }
        state.device = key;
        selectors.device.value = key;
        renderIntervals();
        updateChart();
    }

    function addInterval(afterIndex) {
        const interval = createInterval(state.intervals.length + 1);
        if (typeof afterIndex === 'number' && afterIndex >= 0) {
            state.intervals.splice(afterIndex + 1, 0, interval);
        } else {
            state.intervals.push(interval);
        }
        renderIntervals();
        updateChart();
        updateStatus();
    }

    function createInterval(count) {
        const base = {
            name: `Interval ${count}`,
            duration: '00:05:00'
        };
        switch (state.device) {
        case 'bike':
            base.resistance = 20;
            base.cadence = 80;
            base.power = 180;
            break;
        case 'elliptical':
            base.resistance = 12;
            base.inclination = 5;
            base.cadence = 60;
            break;
        case 'rower':
            base.power = 180;
            base.cadence = 28;
            break;
        default:
            base.speed = state.miles ? 6.0 : 9.5;
            base.inclination = 1.0;
            break;
        }
        base.__selected = false;
        return base;
    }

    function renderIntervals() {
        selectors.intervalList.innerHTML = '';
        if (!state.intervals.length) {
            return;
        }
        state.intervals.forEach((row, index) => {
            const card = document.createElement('div');
            card.className = 'interval-card';
            if (row.__selected) {
                card.classList.add('selected');
            }

            if (row.__selected === undefined) {
                row.__selected = false;
            }

            const header = document.createElement('div');
            header.className = 'card-header';

            const headerLeft = document.createElement('div');
            headerLeft.className = 'card-header-left';

            const selectBox = document.createElement('input');
            selectBox.type = 'checkbox';
            selectBox.checked = !!row.__selected;
            selectBox.title = 'Select interval';
            selectBox.addEventListener('change', event => {
                row.__selected = event.target.checked;
                card.classList.toggle('selected', row.__selected);
                updateControls();
            });
            headerLeft.appendChild(selectBox);

            const title = document.createElement('div');
            title.className = 'card-header-name';
            title.textContent = row.name ? `${row.name}` : `Interval ${index + 1}`;
            headerLeft.appendChild(title);

            header.appendChild(headerLeft);

            const actions = document.createElement('div');
            actions.className = 'card-actions';
            actions.appendChild(actionButton('↑', () => moveInterval(index, -1), index === 0));
            actions.appendChild(actionButton('↓', () => moveInterval(index, 1), index === state.intervals.length - 1));
            actions.appendChild(actionButton('Copy', () => duplicateInterval(index)));
            actions.appendChild(actionButton('Del', () => removeInterval(index), state.intervals.length === 1));
            header.appendChild(actions);
            card.appendChild(header);

            const grid = document.createElement('div');
            grid.className = 'field-grid';

            FIELD_DEFS.forEach(field => {
                if (!shouldRenderField(field)) {
                    return;
                }
                const value = row[field.key];
                const fieldWrap = document.createElement('div');
                fieldWrap.className = 'field';

                const label = document.createElement('label');
                label.textContent = resolveFieldLabel(field);
                fieldWrap.appendChild(label);

                if (field.type === 'bool') {
                    const checkbox = document.createElement('input');
                    checkbox.type = 'checkbox';
                    checkbox.checked = !!value;
                    checkbox.dataset.index = index;
                    checkbox.dataset.key = field.key;
                    checkbox.dataset.type = field.type;
                    checkbox.addEventListener('change', handleFieldChange);
                    fieldWrap.appendChild(checkbox);
                } else {
                    const input = document.createElement('input');
                    input.dataset.index = index;
                    input.dataset.key = field.key;
                    input.dataset.type = field.type;
                    if (field.type === 'duration') {
                        input.type = 'text';
                        input.value = value || '00:05:00';
                        input.placeholder = 'hh:mm:ss';
                    } else if (field.type === 'text') {
                        input.type = 'text';
                        input.value = value || '';
                    } else {
                        input.type = 'number';
                        if (field.step !== undefined) input.step = field.step;
                        if (field.min !== undefined) input.min = field.min;
                        if (field.max !== undefined) input.max = field.max;
                        input.value = value !== undefined ? value : '';
                    }
                    input.addEventListener(field.type === 'duration' ? 'change' : 'input', handleFieldChange);
                    fieldWrap.appendChild(input);
                }

                grid.appendChild(fieldWrap);
            });

            card.appendChild(grid);
            selectors.intervalList.appendChild(card);
        });
        updateControls();
    }

    function shouldRenderField(field) {
        if (field.group === 'advanced' && !state.showAdvanced) {
            return false;
        }
        if (field.devices === 'all') {
            return true;
        }
        return Array.isArray(field.devices) && field.devices.indexOf(state.device) >= 0;
    }

    function resolveFieldLabel(field) {
        if (typeof field.label === 'function') {
            return field.label();
        }
        if (field.unitKey === 'speed') {
            return `${field.label} (${state.miles ? 'mph' : 'km/h'})`;
        }
        if (field.unitSuffix) {
            return `${field.label} (${field.unitSuffix})`;
        }
        return field.label;
    }

    function actionButton(text, handler, disabled) {
        const btn = document.createElement('button');
        btn.className = 'small';
        btn.textContent = text;
        if (disabled) {
            btn.disabled = true;
        }
        btn.addEventListener('click', handler);
        return btn;
    }

    function handleFieldChange(event) {
        const target = event.target;
        const index = Number(target.dataset.index);
        const key = target.dataset.key;
        const type = target.dataset.type;
        if (Number.isNaN(index) || !state.intervals[index]) {
            return;
        }
        if (type === 'bool') {
            state.intervals[index][key] = target.checked;
        } else if (type === 'duration') {
            const seconds = parseDuration(target.value);
            const normalized = formatDuration(seconds);
            state.intervals[index][key] = normalized;
            target.value = normalized;
        } else {
            const raw = target.value;
            state.intervals[index][key] = raw === '' ? undefined : Number(raw);
        }
        if (key === 'name') {
            const label = target.closest('.interval-card').querySelector('.card-header-name');
            if (label) {
                label.textContent = state.intervals[index][key] || `Interval ${index + 1}`;
            }
        }
        updateChart();
        updateStatus();
    }

    function moveInterval(index, delta) {
        const target = index + delta;
        if (target < 0 || target >= state.intervals.length) {
            return;
        }
        const swap = state.intervals[target];
        state.intervals[target] = state.intervals[index];
        state.intervals[index] = swap;
        renderIntervals();
        updateChart();
        updateStatus();
    }

    function duplicateInterval(index) {
        const original = state.intervals[index];
        const copy = cloneInterval(original);
        copy.name = `${original.name || `Interval ${index + 1}`} copy`;
        state.intervals.splice(index + 1, 0, copy);
        renderIntervals();
        updateChart();
        updateStatus();
    }

    function removeInterval(index) {
        if (state.intervals.length === 1) {
            announce('Cannot remove the only interval', true);
            return;
        }
        state.intervals.splice(index, 1);
        renderIntervals();
        updateChart();
        updateStatus();
    }

    function saveWorkflow(startAfter) {
        if (window.QZ_OFFLINE) {
            announce('Offline: cannot save workouts', true);
            return;
        }
        const payload = buildPayload();
        if (!payload) {
            return;
        }
        setWorking(true);
        sendMessage('savetrainingprogram', payload, 'R_savetrainingprogram').then(content => {
            if (!content) {
                announce('Save failed', true);
                return;
            }
            state.lastSaved = payload.name;
            selectors.name.value = payload.name;
            announce(`Saved ${payload.name}`);
            return refreshProgramList().then(() => {
                selectors.programSelect.value = payload.name;
                if (startAfter) {
                    return startProgram(payload.name);
                }
            });
        }).catch(err => {
            console.error(err);
            announce('Unable to save workout', true);
        }).finally(() => setWorking(false));
    }

    function startProgram(name) {
        if (window.QZ_OFFLINE) {
            announce('Offline: cannot start workouts', true);
            return Promise.resolve();
        }
        return sendMessage('workouteditor_start', { name }, 'R_workouteditor_start').then(resp => {
            if (!resp || !resp.ok) {
                announce('Unable to start workout', true);
                return;
            }
            announce('Workout started');
        }).catch(err => {
            console.error(err);
            announce('Unable to start workout', true);
        });
    }

    function buildPayload() {
        const nameInput = selectors.name.value.trim();
        const sanitized = sanitizeName(nameInput || state.lastSaved || 'Workout');
        if (!state.intervals.length) {
            announce('Add at least one interval', true);
            return null;
        }
        const list = [];
        for (const interval of state.intervals) {
            const durationSec = parseDuration(interval.duration);
            if (!durationSec) {
                announce('Invalid duration in intervals', true);
                return null;
            }
            const row = {
                duration: formatDuration(durationSec)
            };
            FIELD_DEFS.forEach(field => {
                if (field.key === 'name' || field.key === 'duration') {
                    return;
                }
                const value = interval[field.key];
                if (value !== undefined && value !== null && value !== '') {
                    row[field.key] = field.type === 'number' ? Number(value) : value;
                }
            });
            list.push(row);
        }
        return {
            name: sanitized,
            list: list
        };
    }

    function sanitizeName(name) {
        return name.replace(/\s+/g, '_').replace(/[^A-Za-z0-9_\-]/g, '_');
    }

    function updateChart() {
        const payload = buildChartPayload();
        WorkoutEditorApp.update(payload);
    }

    function buildChartPayload() {
        let cursor = 0;
        const rows = [];
        const seriesDefs = SERIES_DEFS[state.device] || [];
        const series = seriesDefs.map(def => ({
            key: def.key,
            label: def.label(),
            color: def.color,
            unit: typeof def.unit === 'function' ? def.unit() : def.unit,
            axis: def.axis,
            axisLabel: typeof def.axisLabel === 'function' ? def.axisLabel() : def.axisLabel,
            axisPosition: def.axisPosition,
            points: []
        }));

        state.intervals.forEach(interval => {
            const duration = parseDuration(interval.duration);
            if (!duration) {
                return;
            }
            const start = cursor;
            const end = cursor + duration;
            const intervalCopy = Object.assign({}, interval);
            delete intervalCopy.__selected;
            rows.push(Object.assign({ start, durationSeconds: duration }, intervalCopy));
            series.forEach(serie => {
                const value = interval[serie.key];
                if (value !== undefined && value !== null && value !== '') {
                    serie.points.push({ x: start, y: Number(value) });
                    serie.points.push({ x: end, y: Number(value) });
                }
            });
            cursor = end;
        });

        return {
            title: selectors.name.value.trim() || 'Untitled Workout',
            subtitle: devicePrettyName(state.device),
            totalSeconds: cursor,
            series: series.filter(s => s.points.length),
            rows: rows
        };
    }

    function updateStatus() {
        const totalSeconds = state.intervals.reduce((acc, interval) => acc + parseDuration(interval.duration), 0);
        selectors.statusDuration.textContent = formatDuration(totalSeconds);
        selectors.statusIntervals.textContent = String(state.intervals.length);
    }

    function formatDuration(totalSeconds) {
        totalSeconds = Math.max(0, Math.floor(totalSeconds || 0));
        const hours = Math.floor(totalSeconds / 3600);
        const minutes = Math.floor((totalSeconds % 3600) / 60);
        const seconds = totalSeconds % 60;
        const hh = hours.toString().padStart(2, '0');
        const mm = minutes.toString().padStart(2, '0');
        const ss = seconds.toString().padStart(2, '0');
        return `${hh}:${mm}:${ss}`;
    }

    function parseDuration(text) {
        if (!text) {
            return 0;
        }
        const parts = text.split(':').map(part => parseInt(part, 10));
        if (parts.some(num => isNaN(num))) {
            return 0;
        }
        if (parts.length === 3) {
            return parts[0] * 3600 + parts[1] * 60 + parts[2];
        }
        if (parts.length === 2) {
            return parts[0] * 60 + parts[1];
        }
        if (parts.length === 1) {
            return parts[0];
        }
        return 0;
    }

    function devicePrettyName(key) {
        switch (key) {
        case 'bike':
            return 'Bike';
        case 'elliptical':
            return 'Elliptical';
        case 'rower':
            return 'Rower';
        default:
            return 'Treadmill';
        }
    }

    function announce(message, error) {
        selectors.statusMessage.textContent = message || '';
        selectors.statusMessage.classList.toggle('error', !!error);
        if (message) {
            clearTimeout(announce.timer);
            announce.timer = setTimeout(() => {
                selectors.statusMessage.textContent = '';
                selectors.statusMessage.classList.remove('error');
            }, 5000);
        }
    }

    function setWorking(active) {
        state.loading = active;
        [selectors.saveWorkout, selectors.saveStartWorkout, selectors.loadProgram, selectors.refreshPrograms, selectors.addInterval, selectors.clearIntervals]
            .forEach(btn => btn && (btn.disabled = active));
        updateControls();
    }

    function sendMessage(msg, content, replyMsg) {
        if (window.QZ_OFFLINE) {
            return Promise.resolve(null);
        }
        const el = new MainWSQueueElement({ msg, content }, function (response) {
            if (response.msg === replyMsg) {
                return response.content;
            }
            return null;
        }, 15000, 3);
        return el.enqueue();
    }

    function repeatSelection() {
        const selected = [];
        state.intervals.forEach((row, idx) => {
            if (row.__selected) {
                selected.push(idx);
            }
        });
        if (!selected.length) {
            announce('Select one or more consecutive intervals first', true);
            return;
        }
        selected.sort((a, b) => a - b);
        for (let i = 1; i < selected.length; i++) {
            if (selected[i] !== selected[i - 1] + 1) {
                announce('Selection must be consecutive', true);
                return;
            }
        }

        const promptValue = prompt('Repeat block how many times (total cycles)?', '2');
        if (promptValue === null) {
            return;
        }
        const times = parseInt(promptValue, 10);
        if (Number.isNaN(times) || times < 2) {
            announce('Enter a number greater than 1', true);
            return;
        }

        const start = selected[0];
        const end = selected[selected.length - 1];
        const block = state.intervals.slice(start, end + 1).map(cloneInterval);

        let insertAt = end + 1;
        for (let t = 1; t < times; t++) {
            const clones = block.map(cloneInterval);
            state.intervals.splice(insertAt, 0, ...clones);
            insertAt += clones.length;
        }

        state.intervals.forEach(row => { row.__selected = false; });
        renderIntervals();
        updateChart();
        updateStatus();
        updateControls();
        announce(`Block repeated ${times} times`);
    }

    function hasSelection() {
        return state.intervals.some(row => row.__selected);
    }

    function updateControls() {
        const offline = window.QZ_OFFLINE;
        if (selectors.saveWorkout) {
            selectors.saveWorkout.disabled = state.loading || offline;
        }
        if (selectors.saveStartWorkout) {
            selectors.saveStartWorkout.disabled = state.loading || offline;
        }
        if (selectors.loadProgram) {
            selectors.loadProgram.disabled = state.loading || offline || !state.programs.length;
        }
        if (selectors.refreshPrograms) {
            selectors.refreshPrograms.disabled = state.loading || offline;
        }
        if (selectors.programSelect) {
            selectors.programSelect.disabled = offline || state.loading || !state.programs.length;
        }
        if (selectors.repeatSelection) {
            selectors.repeatSelection.disabled = state.loading || !hasSelection();
        }
        if (selectors.offlineBanner) {
            selectors.offlineBanner.classList.toggle('hidden', !offline);
        }
    }

    function cloneInterval(interval) {
        const clone = JSON.parse(JSON.stringify(interval));
        delete clone.__selected;
        return clone;
    }

})();
