const fs = require('fs');
const path = require('path');
const { chromium } = require('playwright');

const artifactDir = path.join(__dirname, 'artifacts');
fs.mkdirSync(artifactDir, { recursive: true });

const fixture = [
  {
    elapsed_h: 0, elapsed_m: 0, elapsed_s: 0,
    deviceType: 1, speed: 10, inclination: 2, distance: 0,
    heart: 138, heart_avg: 138, heart_max: 138, calories: 0,
    watts: 0, watts_avg: 0, watts_max: 0, jouls: 0,
    req_power: 0, cadence: 0, req_cadence: 0,
    resistance: 0, req_resistance: 0,
    peloton_resistance: 0, peloton_req_resistance: 0,
    cadence_avg: 0, resistance_avg: 0, peloton_resistance_avg: 0,
    workoutName: 'Treadmill visual regression test',
    workoutStartDate: '20/08/2026 07:55', instructorName: ''
  },
  {
    elapsed_h: 0, elapsed_m: 0, elapsed_s: 10,
    deviceType: 1, speed: 0, inclination: 4, distance: 0.027778,
    heart: 141, heart_avg: 140, heart_max: 141, calories: 2,
    watts: 0, watts_avg: 0, watts_max: 0, jouls: 0,
    req_power: 0, cadence: 0, req_cadence: 0,
    resistance: 0, req_resistance: 0,
    peloton_resistance: 0, peloton_req_resistance: 0,
    cadence_avg: 0, resistance_avg: 0, peloton_resistance_avg: 0,
    workoutName: 'Treadmill visual regression test',
    workoutStartDate: '20/08/2026 07:55', instructorName: ''
  },
  {
    elapsed_h: 0, elapsed_m: 0, elapsed_s: 20,
    deviceType: 1, speed: 12, inclination: 6, distance: 0.061111,
    heart: 144, heart_avg: 141, heart_max: 144, calories: 4,
    watts: 0, watts_avg: 0, watts_max: 0, jouls: 0,
    req_power: 0, cadence: 0, req_cadence: 0,
    resistance: 0, req_resistance: 0,
    peloton_resistance: 0, peloton_req_resistance: 0,
    cadence_avg: 0, resistance_avg: 0, peloton_resistance_avg: 0,
    workoutName: 'Treadmill visual regression test',
    workoutStartDate: '20/08/2026 07:55', instructorName: ''
  }
];

(async () => {
  const browser = await chromium.launch({ headless: true });
  const page = await browser.newPage({
    viewport: { width: 430, height: 932 },
    deviceScaleFactor: 1
  });

  const consoleErrors = [];
  const pageErrors = [];
  page.on('console', msg => {
    if (msg.type() === 'error') consoleErrors.push(msg.text());
  });
  page.on('pageerror', err => pageErrors.push(String(err)));

  const url = process.env.QZ_CHART_URL || 'http://127.0.0.1:8765/tst/js/treadmill_summary_visual_fixture.htm';
  await page.goto(url, { waitUntil: 'load' });

  await page.evaluate(samples => {
    ensurePowerZones();
    ensureHeartZones();
    window.process_arr(samples);
  }, fixture);

  await page.waitForFunction(() => {
    const speedCanvas = document.getElementById('canvasSpeedInclination');
    const heartCanvas = document.getElementById('canvasHeart');
    const speedChart = speedCanvas && window.Chart && Chart.getChart(speedCanvas);
    const heartChart = heartCanvas && window.Chart && Chart.getChart(heartCanvas);
    return Boolean(speedChart && heartChart && window.qzTreadmillSummaryDiagnostics && window.qzTreadmillSummaryDiagnostics.valid);
  }, null, { timeout: 10000 });

  await page.waitForTimeout(300);

  const inspection = await page.evaluate(() => {
    const speedCanvas = document.getElementById('canvasSpeedInclination');
    const heartCanvas = document.getElementById('canvasHeart');
    const speedChart = Chart.getChart(speedCanvas);
    const heartChart = Chart.getChart(heartCanvas);
    const speedRect = speedCanvas.getBoundingClientRect();
    const heartRect = heartCanvas.getBoundingClientRect();
    const grandParent = speedCanvas.parentElement && speedCanvas.parentElement.parentElement;
    return {
      diagnostics: window.qzTreadmillSummaryDiagnostics,
      canvas: {
        width: speedRect.width,
        height: speedRect.height,
        display: getComputedStyle(speedCanvas).display,
        visible: speedRect.width > 0 && speedRect.height > 0
      },
      chart: {
        width: speedChart.width,
        height: speedChart.height,
        datasets: speedChart.data.datasets.map(ds => ({
          label: ds.label,
          points: ds.data.map(p => ({ x: p.x, y: p.y }))
        }))
      },
      heartCanvas: {
        width: heartRect.width,
        height: heartRect.height,
        display: getComputedStyle(heartCanvas).display,
        visible: heartRect.width > 0 && heartRect.height > 0
      },
      heartChart: {
        width: heartChart.width,
        height: heartChart.height,
        datasets: heartChart.data.datasets.map(ds => ({
          label: ds.label,
          points: ds.data.map(p => ({ x: p.x, y: p.y }))
        }))
      },
      speedContainerInsideBadge: Boolean(grandParent && grandParent.id === 'watt_badge'),
      summaryText: document.getElementById('watt_badge').innerText
    };
  });

  const heartPoints = inspection.heartChart.datasets[0].points;
  const checks = {
    diagnosticsValid: inspection.diagnostics && inspection.diagnostics.valid === true,
    durationIs20Seconds: inspection.diagnostics && inspection.diagnostics.durationSeconds === 20,
    canvasVisible: inspection.canvas.visible,
    chartHasSize: inspection.chart.width > 0 && inspection.chart.height > 0,
    twoDatasets: inspection.chart.datasets.length >= 2,
    speedHasVisibleValues: inspection.chart.datasets[0].points.every(p => Number.isFinite(p.y) && p.y > 0),
    inclinationMatchesFixture: JSON.stringify(inspection.chart.datasets[1].points.map(p => p.y)) === JSON.stringify([2, 4, 6]),
    heartCanvasVisible: inspection.heartCanvas.visible,
    heartChartHasSize: inspection.heartChart.width > 0 && inspection.heartChart.height > 0,
    heartDatasetPresent: inspection.heartChart.datasets.length === 1 && inspection.heartChart.datasets[0].label === 'Heart Rate',
    heartMatchesFixture: JSON.stringify(heartPoints.map(p => p.y)) === JSON.stringify([138, 141, 144]),
    heartElapsedMatchesFixture: JSON.stringify(heartPoints.map(p => p.x)) === JSON.stringify([0, 10, 20]),
    movedInsideBadge: inspection.speedContainerInsideBadge,
    summaryAdaptedForTreadmill: inspection.summaryText.includes('Avg Pace') && inspection.summaryText.includes('Avg Incline') && inspection.summaryText.includes('Elevation Gain') && inspection.summaryText.includes('Avg Heart Rate'),
    noPageErrors: pageErrors.length === 0
  };

  await page.locator('#watt_badge').screenshot({ path: path.join(artifactDir, 'treadmill-summary-badge.png') });
  await page.locator('#canvasHeart').screenshot({ path: path.join(artifactDir, 'treadmill-heart-rate-chart.png') });
  await page.screenshot({ path: path.join(artifactDir, 'treadmill-summary-full-page.png'), fullPage: true });

  const report = {
    generatedAt: new Date().toISOString(),
    url,
    viewport: { width: 430, height: 932 },
    inspection,
    checks,
    consoleErrors,
    pageErrors,
    passed: Object.values(checks).every(Boolean)
  };
  fs.writeFileSync(path.join(artifactDir, 'treadmill-summary-visual-report.json'), JSON.stringify(report, null, 2));

  console.log(JSON.stringify(report, null, 2));
  await browser.close();
  if (!report.passed) process.exit(1);
})().catch(err => {
  console.error(err && err.stack ? err.stack : err);
  process.exit(1);
});
