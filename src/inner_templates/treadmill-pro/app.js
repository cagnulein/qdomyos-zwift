'use strict';

// ── DOM refs ──────────────────────────────────────────────────────────────────
const elWorkoutName   = document.getElementById('workout-name');
const elTlElapsed     = document.getElementById('tl-elapsed');
const elTlRemain      = document.getElementById('tl-remain-label');
const elTlCompleted   = document.getElementById('tl-completed');
const elTlCurrent     = document.getElementById('tl-current');
const elTlRunner      = document.getElementById('tl-runner');
const elIntervalName  = document.getElementById('interval-name');
const elIntervalBadge = document.getElementById('interval-badge');
const elIntervalDetail= document.getElementById('interval-detail');
const elSpeedValue    = document.getElementById('speed-value');
const elSpeedAvg      = document.getElementById('speed-avg');
const elSpeedMax      = document.getElementById('speed-max');
const elCadence       = document.getElementById('val-cadence');
const elWatts         = document.getElementById('val-watts');
const elHr            = document.getElementById('val-hr');
const elKcal          = document.getElementById('val-kcal');
const elDist          = document.getElementById('val-dist');
const elElapsed       = document.getElementById('elapsed');
const elConnStatus    = document.getElementById('conn-status');
const canvas          = document.getElementById('sparkline');

// ── State ─────────────────────────────────────────────────────────────────────
let wsSocket = null, wsPort = 0, wsReconnectTimer = null;
let elapsedSec = 0, elapsedTimer = null;
let speedMax = 0, speedSum = 0, speedCount = 0;
const HIST = 80;
const speedHist = [];

// ── Elapsed timer ─────────────────────────────────────────────────────────────
function startElapsedTimer() {
  if (elapsedTimer) return;
  elapsedTimer = setInterval(() => {
    elapsedSec++;
    elElapsed.textContent  = fmtHMS(elapsedSec);
    elTlElapsed.textContent = fmtMS(elapsedSec);
  }, 1000);
}

function fmtHMS(s) {
  const h = Math.floor(s / 3600), m = Math.floor((s % 3600) / 60), ss = s % 60;
  return `${pad(h)}:${pad(m)}:${pad(ss)}`;
}

function fmtMS(s) {
  const m = Math.floor(s / 60), ss = s % 60;
  return `${m}:${pad(ss)}`;
}

function pad(n) { return String(n).padStart(2, '0'); }

function formatPace(kmh) {
  if (!kmh || kmh < 0.5) return '–';
  const s = 3600 / kmh;
  return `${Math.floor(s / 60)}:${pad(Math.round(s % 60))}`;
}

// ── HR zone ───────────────────────────────────────────────────────────────────
function hrZone(bpm) {
  if (!bpm || bpm < 50) return '';
  if (bpm < 100) return 'hr-z1';
  if (bpm < 130) return 'hr-z2';
  if (bpm < 155) return 'hr-z3';
  if (bpm < 175) return 'hr-z4';
  return 'hr-z5';
}

// ── Sparkline ─────────────────────────────────────────────────────────────────
const DPR = window.devicePixelRatio || 1;

function resizeCanvas() {
  const r = canvas.getBoundingClientRect();
  canvas.width  = r.width  * DPR;
  canvas.height = r.height * DPR;
}

function drawSparkline(targetSpeed) {
  const ctx = canvas.getContext('2d');
  const W = canvas.width, H = canvas.height;
  ctx.clearRect(0, 0, W, H);
  if (speedHist.length < 2) return;

  const maxV = Math.max(targetSpeed * 1.2, Math.max(...speedHist) * 1.1, 5);
  const toY  = v => H * 0.92 - (v / maxV) * H * 0.84;
  const toX  = i => (i / (HIST - 1)) * W;

  // gradient fill
  const grad = ctx.createLinearGradient(0, 0, 0, H);
  grad.addColorStop(0,   'rgba(0,212,90,0.30)');
  grad.addColorStop(1,   'rgba(0,212,90,0)');

  ctx.beginPath();
  ctx.moveTo(toX(0), toY(speedHist[0]));
  for (let i = 1; i < speedHist.length; i++) {
    const x0 = toX(i - 1), y0 = toY(speedHist[i - 1]);
    const x1 = toX(i),     y1 = toY(speedHist[i]);
    ctx.bezierCurveTo((x0 + x1) / 2, y0, (x0 + x1) / 2, y1, x1, y1);
  }
  ctx.lineTo(toX(speedHist.length - 1), H);
  ctx.lineTo(0, H);
  ctx.closePath();
  ctx.fillStyle = grad;
  ctx.fill();

  // line
  ctx.beginPath();
  ctx.moveTo(toX(0), toY(speedHist[0]));
  for (let i = 1; i < speedHist.length; i++) {
    const x0 = toX(i - 1), y0 = toY(speedHist[i - 1]);
    const x1 = toX(i),     y1 = toY(speedHist[i]);
    ctx.bezierCurveTo((x0 + x1) / 2, y0, (x0 + x1) / 2, y1, x1, y1);
  }
  ctx.strokeStyle = '#00d45a';
  ctx.lineWidth   = 2 * DPR;
  ctx.shadowColor = '#00d45a';
  ctx.shadowBlur  = 6 * DPR;
  ctx.stroke();
  ctx.shadowBlur  = 0;

  // target dashed
  if (targetSpeed > 0) {
    const ty = toY(targetSpeed);
    ctx.setLineDash([5 * DPR, 4 * DPR]);
    ctx.beginPath();
    ctx.moveTo(0, ty);
    ctx.lineTo(W, ty);
    ctx.strokeStyle = 'rgba(255,140,0,0.7)';
    ctx.lineWidth   = 1.5 * DPR;
    ctx.shadowColor = '#ff8c00';
    ctx.shadowBlur  = 3 * DPR;
    ctx.stroke();
    ctx.setLineDash([]);
    ctx.shadowBlur  = 0;
  }

  // tip dot
  const tx = toX(speedHist.length - 1);
  const ty2 = toY(speedHist[speedHist.length - 1]);
  ctx.beginPath();
  ctx.arc(tx, ty2, 4 * DPR, 0, Math.PI * 2);
  ctx.fillStyle   = '#00d45a';
  ctx.shadowColor = '#00d45a';
  ctx.shadowBlur  = 10 * DPR;
  ctx.fill();
  ctx.shadowBlur  = 0;
}

// ── Timeline ──────────────────────────────────────────────────────────────────
function updateTimeline(elapsed, remaining) {
  const total = elapsed + remaining;
  if (total <= 0) return;
  const pctDone = elapsed / total;
  // completed = pctDone, current segment indicator width = small fixed chunk
  const segW   = Math.min(0.04, (1 - pctDone));
  const compW  = Math.max(0, pctDone - segW / 2);
  const currW  = segW;

  elTlCompleted.style.width = (compW * 100).toFixed(1) + '%';
  elTlCurrent.style.width   = (currW * 100).toFixed(1) + '%';

  const runnerPct = Math.min(pctDone * 100, 98);
  elTlRunner.style.left = runnerPct.toFixed(1) + '%';

  elTlElapsed.textContent = fmtMS(elapsed);

  if (remaining > 0) {
    const rm = Math.floor(remaining / 60), rs = remaining % 60;
    elTlRemain.textContent = `${fmtMS(remaining)} left`;
  } else {
    elTlRemain.textContent = '';
  }
}

// ── Data update ───────────────────────────────────────────────────────────────
function applyData(d) {
  const speed       = parseFloat(d.speed        ?? 0);
  const cadence     = Math.round(d.cadence      ?? 0);
  const hr          = Math.round(d.heart        ?? 0);
  const dist        = parseFloat(d.distance     ?? 0);
  const kcal        = Math.round(d.calories     ?? 0);
  const watts       = Math.round(d.watts        ?? 0);
  const targetSpeed = parseFloat(d.target_speed ?? 0);
  const targetIncl  = parseFloat(d.target_inclination ?? 0);
  const inclination = parseFloat(d.inclination  ?? 0);

  // elapsed sync
  if (d.elapsed_h !== undefined || d.elapsed_m !== undefined || d.elapsed_s !== undefined) {
    elapsedSec = (d.elapsed_h ?? 0) * 3600 + (d.elapsed_m ?? 0) * 60 + (d.elapsed_s ?? 0);
    elElapsed.textContent = fmtHMS(elapsedSec);
  }
  startElapsedTimer();

  const remaining = (d.remaining_time_h ?? 0) * 3600
                  + (d.remaining_time_m ?? 0) * 60
                  + (d.remaining_time_s ?? 0);

  // timeline
  updateTimeline(elapsedSec, remaining);

  // workout / interval header
  const name = d.workoutName ?? '';
  elWorkoutName.textContent = name || '–';

  if (name) {
    const step = d.nextrow ?? 0;
    elIntervalName.textContent = name;
    elIntervalBadge.textContent = step > 0 ? `Interval ${step}` : '';
    elIntervalBadge.style.display = step > 0 ? '' : 'none';

    const parts = [];
    if (d.row_name) parts.push(d.row_name);
    if (targetSpeed > 0) parts.push(`Target Speed: ${targetSpeed.toFixed(1)} km/h`);
    if (targetIncl  !== 0) parts.push(`Grade: ${targetIncl.toFixed(1)}%`);
    const rowDist = parseFloat(d.row_distance ?? 0);
    if (rowDist > 0) parts.push(`Distance: ${rowDist.toFixed(1)} km`);
    elIntervalDetail.textContent = parts.join(' · ') || '–';
  } else {
    elIntervalName.textContent  = '–';
    elIntervalBadge.style.display = 'none';
    elIntervalDetail.textContent = '–';
  }

  // speed
  speedHist.push(speed);
  if (speedHist.length > HIST) speedHist.shift();
  if (speed > speedMax) speedMax = speed;
  if (speed > 0) { speedSum += speed; speedCount++; }

  elSpeedValue.textContent = speed.toFixed(1);
  elSpeedValue.classList.remove('over', 'under');
  if (targetSpeed > 0) {
    const r = speed / targetSpeed;
    if (r > 1.05)     elSpeedValue.classList.add('over');
    else if (r < 0.9) elSpeedValue.classList.add('under');
  }

  elSpeedAvg.textContent = speedCount > 0 ? (speedSum / speedCount).toFixed(1) : '–';
  elSpeedMax.textContent = speedMax > 0 ? speedMax.toFixed(1) : '–';

  drawSparkline(targetSpeed);

  // tiles
  elCadence.textContent = cadence || '0';
  elWatts.textContent   = watts   || '–';
  elKcal.textContent    = kcal    || '–';

  elHr.className = 'card-value ' + hrZone(hr);
  elHr.textContent = hr > 0 ? hr : '0';

  // odometer
  elDist.textContent = dist.toFixed(2);
}

// ── WebSocket ─────────────────────────────────────────────────────────────────
function connectWS(port) {
  if (wsSocket) { try { wsSocket.close(); } catch (_) {} }
  wsPort = port;
  wsSocket = new WebSocket(`ws://localhost:${port}/`);
  wsSocket.onopen    = () => { setConnected(true);  clearTimeout(wsReconnectTimer); wsReconnectTimer = null; };
  wsSocket.onmessage = (ev) => {
    try { const m = JSON.parse(ev.data); if (m?.msg === 'workout' && m.content) applyData(m.content); } catch (_) {}
  };
  wsSocket.onclose   = () => { setConnected(false); scheduleReconnect(); };
  wsSocket.onerror   = () => { setConnected(false); };
}

function scheduleReconnect() {
  if (wsReconnectTimer) return;
  wsReconnectTimer = setTimeout(() => { wsReconnectTimer = null; if (wsPort) connectWS(wsPort); }, 2000);
}

function setConnected(ok) {
  elConnStatus.textContent = ok ? 'Connected' : 'Connecting…';
  elConnStatus.className   = ok ? 'connected'  : '';
}

function discoverPort() {
  const fromUrl  = parseInt(location.port, 10);
  const cands    = [fromUrl, 6666, 6667, 6668].filter(Boolean);
  let i = 0;
  (function tryNext() {
    if (i >= cands.length) i = 0;
    const port = cands[i++];
    const ws   = new WebSocket(`ws://localhost:${port}/`);
    ws.onopen  = () => { ws.close(); connectWS(port); };
    ws.onerror = () => setTimeout(tryNext, 500);
  })();
}

window.addEventListener('resize', () => { resizeCanvas(); drawSparkline(0); });

document.addEventListener('DOMContentLoaded', () => {
  resizeCanvas();
  setConnected(false);
  discoverPort();
});
