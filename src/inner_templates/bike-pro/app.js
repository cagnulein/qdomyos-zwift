'use strict';

// ── Zone definitions (% of FTP, assuming FTP ≈ 200W as default) ──
const ZONES = [
  { name: 'Z1 Recovery',   max: 0.55, color: '#5ac8fa', label: 'Z1 · Recovery' },
  { name: 'Z2 Endurance',  max: 0.75, color: '#30d158', label: 'Z2 · Endurance' },
  { name: 'Z3 Tempo',      max: 0.90, color: '#ffd60a', label: 'Z3 · Tempo' },
  { name: 'Z4 Threshold',  max: 1.05, color: '#ff9f0a', label: 'Z4 · Threshold' },
  { name: 'Z5 VO2max',     max: 1.20, color: '#ff453a', label: 'Z5 · VO₂max' },
  { name: 'Z6 Anaerobic',  max: 1.50, color: '#bf5af2', label: 'Z6 · Anaerobic' },
  { name: 'Z7 Sprint',     max: Infinity, color: '#ff375f', label: 'Z7 · Sprint' },
];

// ── State ──
const MAX_POINTS = 60;  // 60 seconds of history
const powerHistory   = new Array(MAX_POINTS).fill(null);
const cadenceHistory = new Array(MAX_POINTS).fill(null);
let ftpWatts = 200;
let chart = null;
let wsPort = 0;
let wsSocket = null;
let wsReconnectTimer = null;
let elapsedSeconds = 0;
let elapsedTimer = null;
let lastDataTime = 0;

// ── DOM refs ──
const elElapsed    = document.getElementById('elapsed');
const elConnStatus = document.getElementById('connection-status');
const elPower      = document.getElementById('val-power');
const elCadence    = document.getElementById('val-cadence');
const elSpeed      = document.getElementById('val-speed');
const elHr         = document.getElementById('val-hr');
const elDist       = document.getElementById('val-dist');
const elKcal       = document.getElementById('val-kcal');
const elZoneLabel  = document.getElementById('zone-label');
const elZoneFill   = document.getElementById('zone-bar-fill');
const elZoneSegs   = document.querySelectorAll('.zone-seg');

// ── Elapsed timer ──
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

// ── Chart setup ──
function initChart() {
  const ctx = document.getElementById('chart-canvas').getContext('2d');
  const labels = Array.from({ length: MAX_POINTS }, (_, i) => i - MAX_POINTS + 1);

  chart = new Chart(ctx, {
    type: 'line',
    data: {
      labels,
      datasets: [
        {
          label: 'Power',
          data: [...powerHistory],
          borderColor: '#ff9f0a',
          backgroundColor: 'rgba(255,159,10,0.12)',
          borderWidth: 2,
          pointRadius: 0,
          fill: true,
          tension: 0.4,
          yAxisID: 'yPower',
          spanGaps: true,
        },
        {
          label: 'Cadence',
          data: [...cadenceHistory],
          borderColor: '#0a84ff',
          backgroundColor: 'rgba(10,132,255,0.08)',
          borderWidth: 1.5,
          pointRadius: 0,
          fill: true,
          tension: 0.4,
          yAxisID: 'yCadence',
          spanGaps: true,
        },
      ],
    },
    options: {
      animation: false,
      responsive: true,
      maintainAspectRatio: false,
      interaction: { mode: 'index', intersect: false },
      plugins: { legend: { display: false }, tooltip: { enabled: false } },
      scales: {
        x: { display: false },
        yPower: {
          position: 'left',
          display: false,
          min: 0,
          suggestedMax: 400,
        },
        yCadence: {
          position: 'right',
          display: false,
          min: 0,
          suggestedMax: 120,
        },
      },
    },
  });
}

// ── Zone logic ──
function updateZone(watts) {
  const ratio = watts / ftpWatts;
  let zoneIdx = ZONES.findIndex(z => ratio < z.max);
  if (zoneIdx < 0) zoneIdx = ZONES.length - 1;

  const z = ZONES[zoneIdx];
  const pctOfNext = Math.min(ratio / (ZONES[zoneIdx]?.max ?? 1.5), 1);
  const barPct    = Math.min((ratio / 1.5) * 100, 100);

  elZoneLabel.textContent  = z.label;
  elZoneLabel.style.color  = z.color;
  elZoneFill.style.width   = barPct + '%';
  elZoneFill.style.background = z.color;

  elZoneSegs.forEach((seg, i) => {
    seg.classList.toggle('active', i <= zoneIdx);
  });
}

// ── Heart rate color ──
function hrClass(bpm) {
  if (!bpm || bpm < 60) return '';
  if (bpm < 120) return 'hr-low';
  if (bpm < 150) return 'hr-normal';
  if (bpm < 170) return 'hr-high';
  return 'hr-max';
}

// ── Data update ──
function applyData(d) {
  lastDataTime = Date.now();

  const power   = Math.round(d.watts   ?? 0);
  const cadence = Math.round(d.cadence ?? 0);
  const speed   = parseFloat(d.speed   ?? 0).toFixed(1);
  const hr      = Math.round(d.heart   ?? 0);
  const dist    = parseFloat(d.distance ?? 0).toFixed(2);
  const kcal    = Math.round(d.calories ?? 0);

  // QZ sends elapsed time as separate h/m/s fields
  if (d.elapsed_h !== undefined || d.elapsed_m !== undefined || d.elapsed_s !== undefined) {
    elapsedSeconds = (d.elapsed_h ?? 0) * 3600 + (d.elapsed_m ?? 0) * 60 + (d.elapsed_s ?? 0);
  }

  elPower.textContent   = power   || '–';
  elCadence.textContent = cadence || '–';
  elSpeed.textContent   = speed   !== '0.0' ? speed : '–';
  elKcal.textContent    = kcal    || '–';
  elDist.textContent    = dist    !== '0.00' ? dist : '–';

  if (hr > 0) {
    elHr.textContent  = hr;
    elHr.className    = 'stat-value ' + hrClass(hr);
  } else {
    elHr.textContent  = '–';
    elHr.className    = 'stat-value';
  }

  // zone bar
  if (power > 0) updateZone(power);

  // push to history ring
  powerHistory.shift();   powerHistory.push(power   || null);
  cadenceHistory.shift(); cadenceHistory.push(cadence || null);

  if (chart) {
    chart.data.datasets[0].data = [...powerHistory];
    chart.data.datasets[1].data = [...cadenceHistory];
    chart.update('none');
  }
}

// ── WebSocket ──
function connectWS(port) {
  if (wsSocket) { try { wsSocket.close(); } catch(_){} }
  wsPort = port;
  const url = `ws://localhost:${port}/`;
  wsSocket = new WebSocket(url);

  wsSocket.onopen = () => {
    setStatus(true);
    clearTimeout(wsReconnectTimer);
    startElapsedTimer();
  };

  wsSocket.onmessage = (ev) => {
    try {
      const msg = JSON.parse(ev.data);
      // QZ sends a flat object with all metrics
      if (typeof msg === 'object' && !Array.isArray(msg)) {
        applyData(msg);
      }
    } catch(_) {}
  };

  wsSocket.onclose  = () => { setStatus(false); scheduleReconnect(); };
  wsSocket.onerror  = () => { setStatus(false); };
}

function scheduleReconnect() {
  if (wsReconnectTimer) return;
  wsReconnectTimer = setTimeout(() => {
    wsReconnectTimer = null;
    if (wsPort) connectWS(wsPort);
  }, 2000);
}

function setStatus(connected) {
  elConnStatus.textContent = connected ? 'CONNECTED' : 'CONNECTING…';
  elConnStatus.className   = connected ? 'connected' : '';
}

// ── Port discovery — same strategy as WorkoutEditor.qml ──
// The port is stored in QSettings as "template_inner_QZWS_port".
// We poll the page URL for ?port= query param (set by the QML WebView url).
// If not present, we try common ports.
function discoverPort() {
  const params = new URLSearchParams(location.search);
  const qp = parseInt(params.get('port'), 10);
  if (qp) { connectWS(qp); return; }

  // Try the URL origin port first (same server), then fallback 6666/6667/6668
  const originPort = parseInt(location.port, 10);
  const candidates = [originPort, 6666, 6667, 6668].filter(Boolean);

  let i = 0;
  function tryNext() {
    if (i >= candidates.length) { i = 0; }
    const port = candidates[i++];
    const ws = new WebSocket(`ws://localhost:${port}/`);
    ws.onopen = () => { ws.close(); connectWS(port); };
    ws.onerror = () => { setTimeout(tryNext, 500); };
  }
  tryNext();
}

// ── Init ──
document.addEventListener('DOMContentLoaded', () => {
  initChart();
  setStatus(false);
  discoverPort();
});
