'use strict';

// ── DOM ───────────────────────────────────────────────────────────────────────
const elElapsed     = document.getElementById('elapsed');
const elConn        = document.getElementById('conn-status');
const elSegName     = document.getElementById('segment-name');
const elDist        = document.getElementById('val-dist');
const elTime        = document.getElementById('val-time');
const elAvgSpeed    = document.getElementById('val-avg-speed');
const elProgressFill= document.getElementById('progress-fill');
const elSpeed       = document.getElementById('val-speed');
const elTargetSpeed = document.getElementById('val-target-speed');
const elIncline     = document.getElementById('val-incline');
const elPace        = document.getElementById('val-pace');
const elHr          = document.getElementById('val-hr');
const elHrItem      = document.querySelector('.lap-hr');
const elCadence     = document.getElementById('val-cadence');
const elPauseBtn    = document.getElementById('pause-btn');
const canvas        = document.getElementById('sparkline');

// ── State ─────────────────────────────────────────────────────────────────────
let ws = null, wsPort = 0, wsReconnectTimer = null;
let elapsedSec = 0, elapsedTimer = null, paused = false;
let speedMax = 0, speedSum = 0, speedCount = 0;
const HIST = 60;
const speedHist = [];
const DPR = window.devicePixelRatio || 1;

// ── Helpers ───────────────────────────────────────────────────────────────────
const pad = n => String(n).padStart(2, '0');

function fmtTime(s) {
  const h = Math.floor(s / 3600), m = Math.floor((s % 3600) / 60), ss = s % 60;
  return h > 0 ? `${h}:${pad(m)}:${pad(ss)}` : `${pad(m)}:${pad(ss)}`;
}

function fmtPace(kmh) {
  if (!kmh || kmh < 0.5) return '–';
  const s = 3600 / kmh;
  return `${Math.floor(s / 60)}:${pad(Math.round(s % 60))}`;
}

function fmtDist(km, miles) {
  return miles ? km.toFixed(2) + 'MI' : km.toFixed(2) + 'KM';
}

function hrZone(bpm) {
  if (!bpm || bpm < 50) return '';
  if (bpm < 100) return 'hr-z1';
  if (bpm < 130) return 'hr-z2';
  if (bpm < 155) return 'hr-z3';
  if (bpm < 175) return 'hr-z4';
  return 'hr-z5';
}

// ── Elapsed timer ─────────────────────────────────────────────────────────────
function startElapsedTimer() {
  if (elapsedTimer) return;
  elapsedTimer = setInterval(() => {
    if (!paused) { elapsedSec++; elElapsed.textContent = fmtTime(elapsedSec); }
  }, 1000);
}

// ── Sparkline ─────────────────────────────────────────────────────────────────
function resizeCanvas() {
  const r = canvas.getBoundingClientRect();
  canvas.width = r.width * DPR;
  canvas.height = r.height * DPR;
}

function drawSparkline(targetSpeed) {
  const ctx = canvas.getContext('2d');
  const W = canvas.width, H = canvas.height;
  ctx.clearRect(0, 0, W, H);
  if (speedHist.length < 2) return;

  const maxV = Math.max(targetSpeed * 1.2, Math.max(...speedHist) * 1.1, 3);
  const toY = v => H * 0.9 - (v / maxV) * H * 0.82;
  const toX = i => (i / (HIST - 1)) * W;

  const grad = ctx.createLinearGradient(0, 0, 0, H);
  grad.addColorStop(0, 'rgba(0,188,212,.30)');
  grad.addColorStop(1, 'rgba(0,188,212,0)');

  ctx.beginPath();
  ctx.moveTo(toX(0), toY(speedHist[0]));
  for (let i = 1; i < speedHist.length; i++) {
    const cx = (toX(i - 1) + toX(i)) / 2;
    ctx.bezierCurveTo(cx, toY(speedHist[i-1]), cx, toY(speedHist[i]), toX(i), toY(speedHist[i]));
  }
  ctx.lineTo(toX(speedHist.length - 1), H);
  ctx.lineTo(0, H);
  ctx.closePath();
  ctx.fillStyle = grad;
  ctx.fill();

  ctx.beginPath();
  ctx.moveTo(toX(0), toY(speedHist[0]));
  for (let i = 1; i < speedHist.length; i++) {
    const cx = (toX(i - 1) + toX(i)) / 2;
    ctx.bezierCurveTo(cx, toY(speedHist[i-1]), cx, toY(speedHist[i]), toX(i), toY(speedHist[i]));
  }
  ctx.strokeStyle = '#00bcd4';
  ctx.lineWidth = 2 * DPR;
  ctx.shadowColor = '#00bcd4';
  ctx.shadowBlur = 6 * DPR;
  ctx.stroke();
  ctx.shadowBlur = 0;

  if (targetSpeed > 0) {
    const ty = toY(targetSpeed);
    ctx.setLineDash([5 * DPR, 4 * DPR]);
    ctx.beginPath(); ctx.moveTo(0, ty); ctx.lineTo(W, ty);
    ctx.strokeStyle = 'rgba(255,112,67,.75)';
    ctx.lineWidth = 1.5 * DPR;
    ctx.stroke();
    ctx.setLineDash([]);
  }
}

// ── Send command ──────────────────────────────────────────────────────────────
function sendCmd(cmd) {
  if (ws && ws.readyState === WebSocket.OPEN)
    ws.send(JSON.stringify({ msg: 'cmd', content: { command: cmd } }));
}

function togglePause() {
  paused = !paused;
  elPauseBtn.textContent = paused ? '▶ Resume' : '⏸ Pause';
  sendCmd(paused ? 'pause' : 'start');
}

// ── Data update ───────────────────────────────────────────────────────────────
function applyData(d) {
  const speed       = parseFloat(d.speed        ?? 0);
  const targetSpd   = parseFloat(d.target_speed ?? 0);
  const incline     = parseFloat(d.inclination  ?? 0);
  const hr          = Math.round(d.heart        ?? 0);
  const cadence     = Math.round(d.cadence      ?? 0);
  const dist        = parseFloat(d.distance     ?? 0);
  const kcal        = Math.round(d.calories     ?? 0);

  // elapsed
  if (d.elapsed_h !== undefined || d.elapsed_m !== undefined || d.elapsed_s !== undefined) {
    elapsedSec = (d.elapsed_h ?? 0) * 3600 + (d.elapsed_m ?? 0) * 60 + (d.elapsed_s ?? 0);
    elElapsed.textContent = fmtTime(elapsedSec);
  }
  startElapsedTimer();

  // segment name
  const seg = d.row_name ?? d.workoutName ?? '';
  elSegName.textContent = seg || '–';

  // stats row
  elDist.textContent = dist > 0 ? dist.toFixed(2) + 'km' : '–';
  elTime.textContent = fmtTime(elapsedSec);
  speedHist.push(speed);
  if (speedHist.length > HIST) speedHist.shift();
  if (speed > speedMax) speedMax = speed;
  if (speed > 0) { speedSum += speed; speedCount++; }
  elAvgSpeed.textContent = speedCount > 0 ? (speedSum / speedCount).toFixed(1) + 'km/h' : '–';

  // progress
  const remain = (d.remaining_time_h ?? 0) * 3600 + (d.remaining_time_m ?? 0) * 60 + (d.remaining_time_s ?? 0);
  if (remain > 0 && elapsedSec > 0) {
    const pct = Math.min(elapsedSec / (elapsedSec + remain) * 100, 100);
    elProgressFill.style.width = pct.toFixed(1) + '%';
  }

  // speed control
  elSpeed.textContent = speed.toFixed(1);
  if (targetSpd > 0) {
    elTargetSpeed.textContent = 'TARGET ' + targetSpd.toFixed(1) + 'km/h';
  } else {
    elTargetSpeed.textContent = 'km/h';
  }

  // incline control
  elIncline.textContent = incline.toFixed(1);

  // lap / middle
  elPace.textContent = fmtPace(speed);
  elHr.textContent = hr > 0 ? hr : '–';
  const zone = hrZone(hr);
  elHrItem.className = 'lap-item lap-hr' + (zone ? ' ' + zone : '');
  elCadence.textContent = cadence || '–';

  drawSparkline(targetSpd);
}

// ── WebSocket ─────────────────────────────────────────────────────────────────
function connect(port) {
  if (ws) { try { ws.close(); } catch (_) {} }
  wsPort = port;
  ws = new WebSocket(`ws://localhost:${port}/`);
  ws.onopen    = () => { setConn(true); clearTimeout(wsReconnectTimer); wsReconnectTimer = null; };
  ws.onmessage = ev => { try { const m = JSON.parse(ev.data); if (m?.msg === 'workout' && m.content) applyData(m.content); } catch (_) {} };
  ws.onclose   = () => { setConn(false); scheduleReconnect(); };
  ws.onerror   = () => { setConn(false); };
}

function scheduleReconnect() {
  if (wsReconnectTimer) return;
  wsReconnectTimer = setTimeout(() => { wsReconnectTimer = null; if (wsPort) connect(wsPort); }, 2000);
}

function setConn(ok) {
  elConn.textContent = ok ? 'CONNECTED' : 'CONNECTING…';
  elConn.className   = ok ? 'connected' : '';
}

function discoverPort() {
  const fromUrl = parseInt(location.port, 10);
  const cands = [fromUrl, 6666, 6667, 6668].filter(Boolean);
  let i = 0;
  (function tryNext() {
    if (i >= cands.length) i = 0;
    const p = cands[i++];
    const t = new WebSocket(`ws://localhost:${p}/`);
    t.onopen  = () => { t.close(); connect(p); };
    t.onerror = () => setTimeout(tryNext, 500);
  })();
}

window.addEventListener('resize', () => { resizeCanvas(); drawSparkline(0); });

document.addEventListener('DOMContentLoaded', () => {
  resizeCanvas();
  setConn(false);
  discoverPort();
});
