(function () {
    const state = {
        miles: false,
        device: 'bike',
        intervals: [],
        programs: [],
        programFiles: {}, // Map of program name -> file object (with url, path, etc.)
        showAdvanced: false,
        lastSaved: '',
        loading: false
    };

    const selectors = {};

    const FIELD_DEFS = [
        { key: 'name', label: 'Label', type: 'text', group: 'basic', devices: 'all' },
        { key: 'duration', label: 'Duration', type: 'duration', group: 'basic', devices: 'all' },
        { key: 'speed', label: 'Speed', type: 'number', unitKey: 'speed', step: 0.1, min: 0, group: 'basic', devices: ['treadmill'], defaultValue: () => state.miles ? 6.0 : 9.5 },
        { key: 'inclination', label: 'Incline', type: 'number', unitSuffix: '%', step: 0.5, min: -10, max: 30, group: 'basic', devices: ['treadmill', 'elliptical'], defaultValue: 1.0 },
        { key: 'resistance', label: 'Resistance', type: 'number', step: 1, min: 0, max: 100, group: 'basic', devices: ['bike', 'elliptical'], defaultValue: 20 },
        { key: 'cadence', label: 'Cadence', type: 'number', unitSuffix: 'rpm', min: 0, max: 240, group: 'basic', devices: ['bike', 'elliptical', 'rower'], defaultValue: 80 },
        { key: 'power', label: 'Power', type: 'number', unitSuffix: 'W', min: 0, max: 2000, group: 'basic', devices: ['bike', 'rower'], defaultValue: 150 },
        { key: 'forcespeed', label: 'Force Speed', type: 'bool', group: 'basic', devices: ['treadmill'], linkedTo: 'speed' },
        { key: 'fanspeed', label: 'Fan', type: 'number', min: 0, max: 8, group: 'advanced', devices: 'all', defaultValue: 0 },
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

    // Default values that indicate a field should not be enabled
    const DEFAULT_DISABLED_VALUES = {
        speed: -1,
        cadence: -1,
        resistance: -1,
        power: -1,
        inclination: -200,
        requested_peloton_resistance: -1,
        zoneHR: -1,
        HRmin: -1,
        HRmax: -1,
        minSpeed: -1,
        maxSpeed: -1,
        maxResistance: -1,
        mets: -1
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
        // Always start with one interval
        if (!state.intervals.length) {
            state.intervals.push(createInterval(1));
        }
        // Render immediately to show the initial interval
        renderIntervals();
        updateChart();
        updateStatus();
        updateControls();

        // Then fetch environment and programs in background
        Promise.allSettled([fetchEnvironment(), refreshProgramList()]).then(() => {
            // Re-render after fetching environment (in case device changed)
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
            state.programFiles = {}; // Map of name -> file object
            renderProgramOptions();
            updateControls();
            return Promise.resolve();
        }
        return sendMessage('loadtrainingprograms', '', 'R_loadtrainingprograms').then(content => {
            // Backend returns content.files (array of objects with name, url, isFolder, etc.)
            const files = Array.isArray(content && content.files) ? content.files : [];
            // Filter out folders
            const workoutFiles = files.filter(f => !f.isFolder);
            // Store file names for dropdown
            state.programs = workoutFiles.map(f => f.name);
            // Store file objects for URL lookup
            state.programFiles = {};
            workoutFiles.forEach(f => {
                state.programFiles[f.name] = f;
            });
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
        // Get file URL from stored program files
        const fileObj = state.programFiles[name];
        if (!fileObj || !fileObj.url) {
            announce('Cannot find workout file', true);
            setWorking(false);
            return;
        }
        const fileUrl = fileObj.url;

        // Send message to open the training program file (no response expected)
        sendMessage('trainprogram_open_clicked', { url: fileUrl });

        // Wait a bit for the file to load, then get the program rows
        setTimeout(() => {
            sendMessage('gettrainingprogram', '', 'R_gettrainingprogram')
                .then(content => {
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
                })
                .catch(err => {
                    console.error(err);
                    announce('Unable to load workout', true);
                })
                .finally(() => setWorking(false));
        }, 300); // Give backend time to load the file
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
                let value;
                if (def.type === 'bool') {
                    value = row[def.key] === true || row[def.key] === 1;
                    out[def.key] = value;
                    out['__enabled_' + def.key] = true;
                } else if (def.type === 'number') {
                    value = Number(row[def.key]);
                    // Check if value is the default disabled value
                    const isDefaultValue = DEFAULT_DISABLED_VALUES[def.key] !== undefined &&
                                          value === DEFAULT_DISABLED_VALUES[def.key];

                    if (!isDefaultValue) {
                        out[def.key] = value;
                        // Mark field as enabled if it has a non-default value
                        out['__enabled_' + def.key] = true;
                        // Handle linked fields (like forcespeed)
                        if (def.key === 'speed') {
                            // Enable forcespeed when speed is enabled
                            out['__enabled_forcespeed'] = true;
                            // Set forcespeed value from row, or default to false
                            if (row.forcespeed !== undefined && row.forcespeed !== null) {
                                out['forcespeed'] = row.forcespeed === true || row.forcespeed === 1;
                            } else {
                                out['forcespeed'] = false;
                            }
                        }
                    } else {
                        // Value is default, mark as disabled
                        out['__enabled_' + def.key] = false;
                        // Disable linked fields too
                        if (def.key === 'speed') {
                            out['__enabled_forcespeed'] = false;
                        }
                    }
                } else {
                    out[def.key] = row[def.key];
                    out['__enabled_' + def.key] = true;
                }
            } else {
                // Field not present in saved workout, mark as disabled
                out['__enabled_' + def.key] = false;
            }
        });
        out.__selected = false;
        return out;
    }

    function detectDevice(rows) {
        // Helper to check if a field has a valid (non-default) value
        const hasValidValue = (row, key) => {
            if (row[key] === undefined || row[key] === null) return false;
            if (DEFAULT_DISABLED_VALUES[key] !== undefined) {
                return row[key] !== DEFAULT_DISABLED_VALUES[key];
            }
            return true;
        };

        // Check for treadmill: has speed or inclination
        for (const row of rows) {
            if (hasValidValue(row, 'speed') || hasValidValue(row, 'inclination')) {
                return 'treadmill';
            }
        }

        // Check for bike/elliptical: has resistance
        for (const row of rows) {
            if (hasValidValue(row, 'resistance') || hasValidValue(row, 'maxResistance')) {
                if (hasValidValue(row, 'inclination')) {
                    return 'elliptical';
                }
                return 'bike';
            }
        }

        // Check for rower/bike: has power
        for (const row of rows) {
            if (hasValidValue(row, 'power')) {
                if (hasValidValue(row, 'cadence') && !rows.some(r => hasValidValue(r, 'resistance'))) {
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

        // Initialize all fields as disabled by default
        FIELD_DEFS.forEach(def => {
            if (def.key !== 'name' && def.key !== 'duration') {
                base['__enabled_' + def.key] = false;
            }
        });

        switch (state.device) {
        case 'bike':
            base.resistance = 20;
            base.__enabled_resistance = true;
            base.cadence = 80;
            base.__enabled_cadence = true;
            base.power = 180;
            base.__enabled_power = false; // disabled by default, user chooses resistance OR power
            break;
        case 'elliptical':
            base.resistance = 12;
            base.__enabled_resistance = true;
            base.inclination = 5;
            base.__enabled_inclination = true;
            base.cadence = 60;
            base.__enabled_cadence = true;
            break;
        case 'rower':
            base.power = 180;
            base.__enabled_power = true;
            base.cadence = 28;
            base.__enabled_cadence = true;
            break;
        default:
            // treadmill
            base.speed = state.miles ? 6.0 : 9.5;
            base.__enabled_speed = true;
            base.inclination = 1.0;
            base.__enabled_inclination = true;
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
                // Skip linked fields entirely from UI rendering
                if (field.linkedTo) {
                    return;
                }
                const value = row[field.key];
                const isEnabled = row['__enabled_' + field.key] !== false; // enabled by default
                const fieldWrap = document.createElement('div');
                fieldWrap.className = 'field';
                if (!isEnabled) {
                    fieldWrap.classList.add('disabled');
                }

                // Create label with enable/disable checkbox (except for name, duration, and linked fields)
                const labelWrap = document.createElement('label');
                labelWrap.className = 'field-label';

                if (field.key !== 'name' && field.key !== 'duration' && !field.linkedTo) {
                    const enableCheckbox = document.createElement('input');
                    enableCheckbox.type = 'checkbox';
                    enableCheckbox.checked = isEnabled;
                    enableCheckbox.dataset.index = index;
                    enableCheckbox.dataset.key = field.key;
                    enableCheckbox.addEventListener('change', (e) => {
                        const checked = e.target.checked;
                        row['__enabled_' + field.key] = checked;
                        if (checked) {
                            fieldWrap.classList.remove('disabled');
                            // Set default value if field is empty
                            if (row[field.key] === undefined || row[field.key] === null || row[field.key] === '') {
                                const defaultVal = typeof field.defaultValue === 'function' ? field.defaultValue() : field.defaultValue;
                                if (defaultVal !== undefined) {
                                    row[field.key] = defaultVal;
                                }
                            }
                            // Handle linked fields (like forcespeed)
                            if (field.key === 'speed') {
                                row['__enabled_forcespeed'] = true;
                                row['forcespeed'] = false; // default to false
                            }
                        } else {
                            fieldWrap.classList.add('disabled');
                            // Disable linked fields
                            if (field.key === 'speed') {
                                row['__enabled_forcespeed'] = false;
                            }
                        }
                        // Re-render to update the UI
                        renderIntervals();
                        updateChart();
                        updateStatus();
                    });
                    labelWrap.appendChild(enableCheckbox);
                }

                const labelText = document.createElement('span');
                labelText.textContent = resolveFieldLabel(field);
                labelWrap.appendChild(labelText);
                fieldWrap.appendChild(labelWrap);

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
                    const inputWrapper = document.createElement('div');
                    inputWrapper.className = 'field-with-buttons';

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

                    // Add +/- buttons for duration and number fields
                    if (field.type === 'duration' || field.type === 'number') {
                        const decreaseBtn = document.createElement('button');
                        decreaseBtn.textContent = '-';
                        decreaseBtn.type = 'button';
                        decreaseBtn.title = 'Decrease';
                        decreaseBtn.addEventListener('click', () => handleIncrement(input, field, -1));

                        const increaseBtn = document.createElement('button');
                        increaseBtn.textContent = '+';
                        increaseBtn.type = 'button';
                        increaseBtn.title = 'Increase';
                        increaseBtn.addEventListener('click', () => handleIncrement(input, field, 1));

                        inputWrapper.appendChild(decreaseBtn);
                        inputWrapper.appendChild(input);
                        inputWrapper.appendChild(increaseBtn);
                        fieldWrap.appendChild(inputWrapper);
                    } else {
                        fieldWrap.appendChild(input);
                    }
                }

                grid.appendChild(fieldWrap);
            });

            card.appendChild(grid);
            selectors.intervalList.appendChild(card);
        });
        updateControls();
    }

    function shouldRenderField(field) {
        // Linked fields are shown conditionally in the render loop
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

    function handleIncrement(input, field, direction) {
        const index = Number(input.dataset.index);
        const key = input.dataset.key;
        const type = input.dataset.type;

        if (Number.isNaN(index) || !state.intervals[index]) {
            return;
        }

        if (type === 'duration') {
            // Increment/decrement duration by 5 seconds
            const currentSeconds = parseDuration(input.value || '00:05:00');
            const newSeconds = Math.max(0, currentSeconds + (direction * 5));
            const normalized = formatDuration(newSeconds);
            state.intervals[index][key] = normalized;
            input.value = normalized;
        } else if (type === 'number') {
            // Increment/decrement number by step or 1
            const step = field.step !== undefined ? field.step : 1;
            const currentValue = input.value !== '' ? Number(input.value) : 0;
            let newValue = currentValue + (direction * step);

            // Apply min/max constraints
            if (field.min !== undefined) {
                newValue = Math.max(field.min, newValue);
            }
            if (field.max !== undefined) {
                newValue = Math.min(field.max, newValue);
            }

            // Round to proper decimal places based on step
            if (step < 1) {
                const decimals = step.toString().split('.')[1]?.length || 1;
                newValue = Number(newValue.toFixed(decimals));
            }

            state.intervals[index][key] = newValue;
            input.value = newValue;
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
                    // Verify file exists in program list before starting
                    if (state.programs.includes(payload.name)) {
                        console.log('File verified in list, starting workout');
                        return startProgram(payload.name);
                    } else {
                        announce('Workout file not ready, please try again', true);
                        console.error('File not found in programs list after save');
                    }
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

                // Handle linked fields: forcespeed is automatically 1 if speed is enabled
                if (field.linkedTo) {
                    const parentEnabled = interval['__enabled_' + field.linkedTo] !== false;
                    if (field.key === 'forcespeed' && field.linkedTo === 'speed') {
                        // Always include forcespeed: 1 if speed is enabled, 0 otherwise
                        row[field.key] = parentEnabled ? 1 : 0;
                    }
                    return;
                }

                // Skip disabled fields
                const isEnabled = interval['__enabled_' + field.key] !== false;
                if (!isEnabled) {
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
            // Remove all __enabled_ properties from the copy
            Object.keys(intervalCopy).forEach(key => {
                if (key.startsWith('__enabled_')) {
                    delete intervalCopy[key];
                }
            });
            rows.push(Object.assign({ start, durationSeconds: duration }, intervalCopy));
            series.forEach(serie => {
                // Skip disabled fields in the chart
                const isEnabled = interval['__enabled_' + serie.key] !== false;
                if (!isEnabled) {
                    return;
                }
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
