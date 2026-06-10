'use strict';

// ── DOM refs ──
const elElapsed        = document.getElementById('elapsed');
const elConnStatus     = document.getElementById('connection-status');
const elSpeedValue     = document.getElementById('speed-value');
const elSpeedAvg       = document.getElementById('speed-avg');
const elSpeedMax       = document.getElementById('speed-max');
const elPace           = document.getElementById('val-pace');
const elIncline        = document.getElementById('val-incline');
const elCadence        = document.getElementById('val-cadence');
const elHr             = document.getElementById('val-hr');
const elDist           = document.getElementById('val-dist');
const elKcal           = document.getElementById('val-kcal');
const elWatts          = document.getElementById('val-watts');

// program header
const elProgramSection = document.getElementById('program-header');
const elProgramName    = document.getElementById('program-name');
const elProgramInterval= document.getElementById('program-interval');
const elRowInfo        = document.getElementById('program-row-info');
const elTargetSpeed    = document.getElementById('chip-target-speed');
const elTargetIncline  = document.getElementById('chip-target-incline');
const elProgressFill   = document.getElementById('progress-bar-fill');
const elRowRemaining   = document.getElementById('row-remaining-value');

// target speed bar
const elTargetFill   = document.getElementById('target-speed-fill');
const elTargetMarker = document.getElementById('target-speed-marker');
const elTargetLabel  = document.getElementById('target-label');
const elCurrentLabel = document.getElementById('current-label');

// ── State ──
let wsSocket = null, wsPort = 0, wsReconnectTimer = null;
let elapsedSeconds = 0, elapsedTimer = null;
let sessionMaxSpeed = 0;
const MAX_SPEED = 25; // km/h — bar top

// ── Elapsed timer (local smooth increment) ──
function startElapsedTimer() {
  if (elapsedTimer) return;
  elapsedTimer = setInterval(() => {
    elapsedSeconds++;
    const h = Math.floor(elapsedSeconds / 3600);
    const m = Math.floor((elapsedSeconds % 3600) / 60);
    const s = elapsedSeconds % 60;
    elElapsed.textContent = h > 0
      ? `${h}:${String(m).padStart(2,'0')}:${String(s).padStart(2,'0')}`
      : `${String(m).padStart(2,'0')}:${String(s).padStart(2,'0')}`;
  }, 1000);
}

// ── Pace formatter ──
function formatPace(kmh) {
  if (!kmh || kmh < 0.5) return '–';
  const secPerKm = 3600 / kmh;
  const m = Math.floor(secPerKm / 60);
  const s = Math.round(secPerKm % 60);
  return `${m}:${String(s).padStart(2,'0')}`;
}

// ── HR colour ──
function hrClass(bpm) {
  if (!bpm || bpm < 60)  return '';
  if (bpm < 120) return 'hr-low';
  if (bpm < 150) return 'hr-normal';
  if (bpm < 170) return 'hr-high';
  return 'hr-max';
}

// ── Speed colour (relative to target) ──
function speedColor(speed, target) {
  if (!target || target <= 0) return '';
  const ratio = speed / target;
  if (ratio < 0.9)  return '#5ac8fa';  // slower than target → blue
  if (ratio > 1.05) return '#ff9f0a';  // faster → orange
  return '#30d158';                     // on target → green
}

// ── Data update ──
function applyData(d) {
  const speed       = parseFloat(d.speed        ?? 0);
  const speedAvg    = parseFloat(d.speed_avg    ?? 0).toFixed(1);
  const cadence     = Math.round(d.cadence      ?? 0);
  const hr          = Math.round(d.heart        ?? 0);
  const dist        = parseFloat(d.distance     ?? 0).toFixed(2);
  const kcal        = Math.round(d.calories     ?? 0);
  const watts       = Math.round(d.watts        ?? 0);
  const incline     = parseFloat(d.inclination  ?? 0).toFixed(1);
  const targetSpeed = parseFloat(d.target_speed ?? 0);
  const targetIncl  = parseFloat(d.target_inclination ?? 0);

  // elapsed from server
  if (d.elapsed_h !== undefined || d.elapsed_m !== undefined || d.elapsed_s !== undefined) {
    elapsedSeconds = (d.elapsed_h ?? 0) * 3600 + (d.elapsed_m ?? 0) * 60 + (d.elapsed_s ?? 0);
  }

  // speed hero
  if (speed > sessionMaxSpeed) sessionMaxSpeed = speed;
  elSpeedValue.textContent = speed > 0 ? speed.toFixed(1) : '0.0';
  elSpeedValue.style.color  = speedColor(speed, targetSpeed) || '';
  elSpeedAvg.textContent    = speedAvg !== '0.0' ? speedAvg : '–';
  document.getElementById('speed-max').textContent =
    sessionMaxSpeed > 0 ? sessionMaxSpeed.toFixed(1) : '–';

  // pace
  elPace.textContent    = formatPace(speed);
  elIncline.textContent = incline !== '0.0' ? incline : '0.0';
  elCadence.textContent = cadence || '–';
  elWatts.textContent   = watts || '–';

  if (hr > 0) {
    elHr.textContent  = hr;
    elHr.className    = 'metric-value ' + hrClass(hr);
  } else {
    elHr.textContent  = '–';
    elHr.className    = 'metric-value';
  }

  elDist.textContent  = dist !== '0.00' ? dist : '–';
  elKcal.textContent  = kcal || '–';

  // target speed bar
  if (targetSpeed > 0) {
    const cap = Math.max(targetSpeed * 1.2, MAX_SPEED);
    const fillPct   = Math.min((speed / cap) * 100, 100);
    const markerPct = Math.min((targetSpeed / cap) * 100, 100);
    elTargetFill.style.width    = fillPct + '%';
    elTargetMarker.style.left   = markerPct + '%';
    elTargetLabel.textContent   = 'Target ' + targetSpeed.toFixed(1);
    elCurrentLabel.textContent  = speed.toFixed(1) + ' km/h';
    document.getElementById('target-speed-bar').style.display = 'block';
  } else {
    document.getElementById('target-speed-bar').style.display = 'none';
  }

  // program header
  const name = d.workoutName ?? '';
  if (name && name.length > 0) {
    elProgramSection.classList.remove('program-hidden');
    elProgramName.textContent = name;

    const interval = d.nextrow ?? 0;
    elProgramInterval.textContent = interval > 0 ? `Step ${interval}` : '';
    elProgramInterval.style.display = interval > 0 ? 'inline-flex' : 'none';

    // row chips
    const chips = [];
    if (targetSpeed > 0)
      chips.push(`<span class="row-chip"><span class="row-chip-value">${targetSpeed.toFixed(1)}</span> km/h target</span>`);
    if (targetIncl !== 0)
      chips.push(`<span class="row-chip"><span class="row-chip-value">${targetIncl.toFixed(1)}%</span> incline</span>`);
    elRowInfo.innerHTML = chips.join('<span style="color:var(--surface2)">·</span>');

    // row remaining time
    const rs = (d.row_remaining_time_h ?? 0) * 3600
             + (d.row_remaining_time_m ?? 0) * 60
             + (d.row_remaining_time_s ?? 0);
    if (rs > 0) {
      const rm = Math.floor(rs / 60), rss = rs % 60;
      elRowRemaining.textContent = `${String(rm).padStart(2,'0')}:${String(rss).padStart(2,'0')} left`;
      elRowRemaining.style.display = 'block';
    } else {
      elRowRemaining.style.display = 'none';
    }

    // total progress bar
    const total = (d.remaining_time_h ?? 0) * 3600
                + (d.remaining_time_m ?? 0) * 60
                + (d.remaining_time_s ?? 0);
    if (total > 0 && elapsedSeconds > 0) {
      const fullDuration = elapsedSeconds + total;
      const pct = Math.min((elapsedSeconds / fullDuration) * 100, 100);
      elProgressFill.style.width = pct + '%';
    } else {
      elProgressFill.style.width = '0%';
    }
  } else {
    elProgramSection.classList.add('program-hidden');
  }
}

// ── WebSocket ──
function connectWS(port) {
  if (wsSocket) { try { wsSocket.close(); } catch(_){} }
  wsPort = port;
  wsSocket = new WebSocket(`ws://localhost:${port}/`);

  wsSocket.onopen  = () => {
    setStatus(true);
    clearTimeout(wsReconnectTimer);
    wsReconnectTimer = null;
    startElapsedTimer();
  };

  wsSocket.onmessage = (ev) => {
    try {
      const msg = JSON.parse(ev.data);
      if (msg && msg.msg === 'workout' && msg.content)
        applyData(msg.content);
    } catch(_) {}
  };

  wsSocket.onclose = () => { setStatus(false); scheduleReconnect(); };
  wsSocket.onerror = () => { setStatus(false); };
}

function scheduleReconnect() {
  if (wsReconnectTimer) return;
  wsReconnectTimer = setTimeout(() => {
    wsReconnectTimer = null;
    if (wsPort) connectWS(wsPort);
  }, 2000);
}

function setStatus(ok) {
  elConnStatus.textContent = ok ? 'CONNECTED' : 'CONNECTING…';
  elConnStatus.className   = ok ? 'connected' : '';
}

// ── Port discovery ──
function discoverPort() {
  const p = parseInt(location.port, 10);
  const candidates = [p, 6666, 6667, 6668].filter(Boolean);
  let i = 0;
  function tryNext() {
    if (i >= candidates.length) i = 0;
    const port = candidates[i++];
    const ws = new WebSocket(`ws://localhost:${port}/`);
    ws.onopen  = () => { ws.close(); connectWS(port); };
    ws.onerror = () => setTimeout(tryNext, 500);
  }
  tryNext();
}

document.addEventListener('DOMContentLoaded', () => {
  setStatus(false);
  discoverPort();
});
