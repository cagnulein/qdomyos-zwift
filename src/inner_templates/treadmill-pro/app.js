'use strict';

// ── DOM refs ──────────────────────────────────────────────────────────────────
const elElapsed      = document.getElementById('elapsed');
const elConnStatus   = document.getElementById('conn-status');
const elSpeedValue   = document.getElementById('speed-value');
const elSpeedAvg     = document.getElementById('speed-avg');
const elSpeedMax     = document.getElementById('speed-max');
const elTargetWrap   = document.getElementById('target-wrap');
const elSpeedTarget  = document.getElementById('speed-target');
const elPace         = document.getElementById('val-pace');
const elIncline      = document.getElementById('val-incline');
const elCadence      = document.getElementById('val-cadence');
const elHr           = document.getElementById('val-hr');
const elDist         = document.getElementById('val-dist');
const elKcal         = document.getElementById('val-kcal');
const elWatts        = document.getElementById('val-watts');
const elProgramHdr   = document.getElementById('program-header');
const elProgramName  = document.getElementById('program-name');
const elProgramInt   = document.getElementById('program-interval');
const elRowInfo      = document.getElementById('program-row-info');
const elProgressFill = document.getElementById('progress-fill');
const elProgressGlow = document.getElementById('progress-glow');
const elRowRemaining = document.getElementById('row-remaining');
const elSpeedHero    = document.getElementById('speed-hero');
const canvas         = document.getElementById('sparkline');

// ── State ─────────────────────────────────────────────────────────────────────
let wsSocket = null, wsPort = 0, wsReconnectTimer = null;
let elapsedSec = 0, elapsedTimer = null;
let sessionMaxSpeed = 0, speedSum = 0, speedCount = 0;
const HISTORY_LEN = 80;
const speedHistory = [];

// ── Elapsed timer ─────────────────────────────────────────────────────────────
function startElapsedTimer() {
  if (elapsedTimer) return;
  elapsedTimer = setInterval(() => {
    elapsedSec++;
    elElapsed.textContent = formatTime(elapsedSec);
  }, 1000);
}

function formatTime(s) {
  const h = Math.floor(s / 3600);
  const m = Math.floor((s % 3600) / 60);
  const ss = s % 60;
  return h > 0
    ? `${h}:${pad(m)}:${pad(ss)}`
    : `${pad(m)}:${pad(ss)}`;
}

function pad(n) { return String(n).padStart(2, '0'); }

// ── Pace formatter ────────────────────────────────────────────────────────────
function formatPace(kmh) {
  if (!kmh || kmh < 0.5) return '–';
  const sec = 3600 / kmh;
  return `${Math.floor(sec / 60)}:${pad(Math.round(sec % 60))}`;
}

// ── HR zone class ─────────────────────────────────────────────────────────────
function hrZoneClass(bpm) {
  if (!bpm || bpm < 60)  return '';
  if (bpm < 120) return 'hr-low';
  if (bpm < 150) return 'hr-normal';
  if (bpm < 170) return 'hr-high';
  return 'hr-max';
}

// ── Sparkline ─────────────────────────────────────────────────────────────────
const DPR = window.devicePixelRatio || 1;

function resizeCanvas() {
  const rect = canvas.getBoundingClientRect();
  canvas.width  = rect.width  * DPR;
  canvas.height = rect.height * DPR;
}

function drawSparkline(currentSpeed, targetSpeed) {
  const ctx = canvas.getContext('2d');
  const W = canvas.width, H = canvas.height;
  ctx.clearRect(0, 0, W, H);
  if (speedHistory.length < 2) return;

  const maxVal = Math.max(targetSpeed * 1.15, Math.max(...speedHistory) * 1.1, 5);
  const toY = v => H - (v / maxVal) * H * 0.85 - H * 0.05;
  const toX = i => (i / (HISTORY_LEN - 1)) * W;

  // gradient fill under line
  const grad = ctx.createLinearGradient(0, 0, 0, H);
  grad.addColorStop(0, 'rgba(0,230,118,0.25)');
  grad.addColorStop(1, 'rgba(0,230,118,0)');

  ctx.beginPath();
  ctx.moveTo(toX(0), toY(speedHistory[0]));
  for (let i = 1; i < speedHistory.length; i++) {
    const x0 = toX(i - 1), y0 = toY(speedHistory[i - 1]);
    const x1 = toX(i),     y1 = toY(speedHistory[i]);
    const cx = (x0 + x1) / 2;
    ctx.bezierCurveTo(cx, y0, cx, y1, x1, y1);
  }
  ctx.lineTo(toX(speedHistory.length - 1), H);
  ctx.lineTo(toX(0), H);
  ctx.closePath();
  ctx.fillStyle = grad;
  ctx.fill();

  // main line
  ctx.beginPath();
  ctx.moveTo(toX(0), toY(speedHistory[0]));
  for (let i = 1; i < speedHistory.length; i++) {
    const x0 = toX(i - 1), y0 = toY(speedHistory[i - 1]);
    const x1 = toX(i),     y1 = toY(speedHistory[i]);
    const cx = (x0 + x1) / 2;
    ctx.bezierCurveTo(cx, y0, cx, y1, x1, y1);
  }
  ctx.strokeStyle = '#00e676';
  ctx.lineWidth   = 2 * DPR;
  ctx.shadowColor = '#00e676';
  ctx.shadowBlur  = 6 * DPR;
  ctx.stroke();
  ctx.shadowBlur  = 0;

  // target dashed line
  if (targetSpeed > 0) {
    const ty = toY(targetSpeed);
    ctx.setLineDash([4 * DPR, 4 * DPR]);
    ctx.beginPath();
    ctx.moveTo(0, ty);
    ctx.lineTo(W, ty);
    ctx.strokeStyle = 'rgba(255,109,0,0.7)';
    ctx.lineWidth   = 1.5 * DPR;
    ctx.shadowColor = '#ff6d00';
    ctx.shadowBlur  = 4 * DPR;
    ctx.stroke();
    ctx.setLineDash([]);
    ctx.shadowBlur  = 0;
  }

  // current dot (tip of line)
  const tipX = toX(speedHistory.length - 1);
  const tipY = toY(speedHistory[speedHistory.length - 1]);
  ctx.beginPath();
  ctx.arc(tipX, tipY, 4 * DPR, 0, Math.PI * 2);
  ctx.fillStyle = '#00e676';
  ctx.shadowColor = '#00e676';
  ctx.shadowBlur  = 10 * DPR;
  ctx.fill();
  ctx.shadowBlur  = 0;
}

// ── Data update ───────────────────────────────────────────────────────────────
function applyData(d) {
  const speed        = parseFloat(d.speed        ?? 0);
  const cadence      = Math.round(d.cadence      ?? 0);
  const hr           = Math.round(d.heart        ?? 0);
  const dist         = parseFloat(d.distance     ?? 0);
  const kcal         = Math.round(d.calories     ?? 0);
  const watts        = Math.round(d.watts        ?? 0);
  const incline      = parseFloat(d.inclination  ?? 0);
  const targetSpeed  = parseFloat(d.target_speed ?? 0);
  const targetIncl   = parseFloat(d.target_inclination ?? 0);

  // elapsed sync from server
  if (d.elapsed_h !== undefined || d.elapsed_m !== undefined || d.elapsed_s !== undefined) {
    elapsedSec = (d.elapsed_h ?? 0) * 3600 + (d.elapsed_m ?? 0) * 60 + (d.elapsed_s ?? 0);
    elElapsed.textContent = formatTime(elapsedSec);
  }
  startElapsedTimer();

  // speed history
  speedHistory.push(speed);
  if (speedHistory.length > HISTORY_LEN) speedHistory.shift();

  // running stats
  if (speed > sessionMaxSpeed) sessionMaxSpeed = speed;
  if (speed > 0) { speedSum += speed; speedCount++; }

  // speed hero number
  elSpeedValue.textContent = speed.toFixed(1);

  // colour based on target
  const heroEl = elSpeedHero;
  elSpeedValue.classList.remove('over', 'under');
  heroEl.classList.remove('over-target', 'under-target');
  if (targetSpeed > 0) {
    const ratio = speed / targetSpeed;
    if (ratio > 1.05) {
      elSpeedValue.classList.add('over');
      heroEl.classList.add('over-target');
    } else if (ratio < 0.9) {
      elSpeedValue.classList.add('under');
      heroEl.classList.add('under-target');
    }
  }

  // avg / max / target sub-row
  elSpeedAvg.textContent = speedCount > 0 ? (speedSum / speedCount).toFixed(1) : '–';
  elSpeedMax.textContent = sessionMaxSpeed > 0 ? sessionMaxSpeed.toFixed(1) : '–';
  if (targetSpeed > 0) {
    elTargetWrap.style.display = '';
    elSpeedTarget.textContent  = targetSpeed.toFixed(1);
  } else {
    elTargetWrap.style.display = 'none';
  }

  // sparkline
  drawSparkline(speed, targetSpeed);

  // tiles
  elPace.textContent    = formatPace(speed);
  elIncline.textContent = incline.toFixed(1);
  elCadence.textContent = cadence || '–';

  elHr.className = 'tile-value ' + hrZoneClass(hr);
  elHr.textContent = hr > 0 ? hr : '–';

  // bottom row
  elDist.textContent  = dist > 0 ? dist.toFixed(2) : '–';
  elWatts.textContent = watts || '–';
  elKcal.textContent  = kcal || '–';

  // program header
  const name = d.workoutName ?? '';
  if (name) {
    elProgramHdr.classList.remove('program-hidden');
    elProgramName.textContent = name;

    const step = d.nextrow ?? 0;
    elProgramInt.textContent = step > 0 ? `STEP ${step}` : '';
    elProgramInt.style.display = step > 0 ? '' : 'none';

    const chips = [];
    if (targetSpeed > 0)
      chips.push(`<span class="row-chip"><span class="row-chip-val">${targetSpeed.toFixed(1)}</span> km/h</span>`);
    if (targetIncl !== 0)
      chips.push(`<span class="row-chip"><span class="row-chip-val">${targetIncl.toFixed(1)}%</span> grade</span>`);
    elRowInfo.innerHTML = chips.join('<span style="opacity:.3;padding:0 4px">·</span>');

    // row remaining
    const rs = (d.row_remaining_time_h ?? 0) * 3600
             + (d.row_remaining_time_m ?? 0) * 60
             + (d.row_remaining_time_s ?? 0);
    elRowRemaining.textContent = rs > 0
      ? `${pad(Math.floor(rs / 60))}:${pad(rs % 60)} left`
      : '';

    // session progress bar
    const remain = (d.remaining_time_h ?? 0) * 3600
                 + (d.remaining_time_m ?? 0) * 60
                 + (d.remaining_time_s ?? 0);
    if (remain > 0 && elapsedSec > 0) {
      const pct = Math.min((elapsedSec / (elapsedSec + remain)) * 100, 100);
      elProgressFill.style.width = pct + '%';
      elProgressGlow.style.right  = (100 - pct) + '%';
    }
  } else {
    elProgramHdr.classList.add('program-hidden');
  }
}

// ── WebSocket ─────────────────────────────────────────────────────────────────
function connectWS(port) {
  if (wsSocket) { try { wsSocket.close(); } catch (_) {} }
  wsPort   = port;
  wsSocket = new WebSocket(`ws://localhost:${port}/`);

  wsSocket.onopen = () => {
    setConnected(true);
    clearTimeout(wsReconnectTimer);
    wsReconnectTimer = null;
  };

  wsSocket.onmessage = (ev) => {
    try {
      const msg = JSON.parse(ev.data);
      if (msg && msg.msg === 'workout' && msg.content) applyData(msg.content);
    } catch (_) {}
  };

  wsSocket.onclose = () => { setConnected(false); scheduleReconnect(); };
  wsSocket.onerror = () => { setConnected(false); };
}

function scheduleReconnect() {
  if (wsReconnectTimer) return;
  wsReconnectTimer = setTimeout(() => {
    wsReconnectTimer = null;
    if (wsPort) connectWS(wsPort);
  }, 2000);
}

function setConnected(ok) {
  elConnStatus.textContent = ok ? 'CONNECTED' : 'CONNECTING…';
  elConnStatus.className   = ok ? 'connected'  : '';
}

// ── Port discovery ────────────────────────────────────────────────────────────
function discoverPort() {
  const fromUrl  = parseInt(location.port, 10);
  const candidates = [fromUrl, 6666, 6667, 6668].filter(Boolean);
  let i = 0;
  (function tryNext() {
    if (i >= candidates.length) i = 0;
    const port = candidates[i++];
    const ws   = new WebSocket(`ws://localhost:${port}/`);
    ws.onopen  = () => { ws.close(); connectWS(port); };
    ws.onerror = () => setTimeout(tryNext, 500);
  })();
}

// ── Init ──────────────────────────────────────────────────────────────────────
window.addEventListener('resize', () => { resizeCanvas(); drawSparkline(0, 0); });

document.addEventListener('DOMContentLoaded', () => {
  resizeCanvas();
  setConnected(false);
  discoverPort();
});
