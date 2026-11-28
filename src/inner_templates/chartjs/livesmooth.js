// The Smooth Chart Lines setting (chart_smooth_lines) on the live charts (chartlive.htm,
// treadmillchartlive.htm): the power and the speed line. The index in its list in settings.qml:
// Off, Auto, then the moving average windows in seconds, as in dochart.js
var LIVE_SMOOTH_WINDOWS = [0, -1, 3, 5, 10, 15, 30];

// The window in seconds for the setting value, 0: the raw line. A live chart is shown only while
// the workout runs, so Auto is 3 s here, as Auto on the Charts page during a workout. Not saved
// yet (null) or unknown: Off
function liveSmoothWindow(setting) {
    let index = parseInt(setting, 10);
    if (!(index >= 0 && index < LIVE_SMOOTH_WINDOWS.length))
        return 0;
    let w = LIVE_SMOOTH_WINDOWS[index];
    return w < 0 ? 3 : w;
}

// A trailing moving average of the point i of row over the last `seconds` of the elapsed time (x):
// the points from x - seconds + 1 to x. Trailing, not centered as on the Charts page: a point
// drawn once does not change when the next ones arrive. Rounded to `digits` decimals.
// A point with no value stays a gap, as on the Charts page. While the workout is paused the
// elapsed time stands still and the points keep coming with the same x: only the last point of
// each x counts, as the one row per second of the session, so the pause does not weigh in
function isLiveValue(y) {
    return typeof y === 'number' && y === y; // y === y: not NaN
}

function trailingAverage(row, i, seconds, digits) {
    if (!isLiveValue(row[i].y))
        return row[i].y;
    let k = Math.pow(10, digits || 0), sum = 0, count = 0;
    for (let j = i; j >= 0 && row[j].x > row[i].x - seconds; j--) {
        if (j < i && row[j + 1].x === row[j].x)
            continue; // not the last point of its x
        if (isLiveValue(row[j].y)) {
            sum += row[j].y;
            count++;
        }
    }
    return count ? Math.round(sum / count * k) / k : row[i].y;
}

// The line drawn for the raw points of row: row itself when the window is off (as without the
// setting), otherwise a new row of the trailing averages. The maximums and zones stay on row
function smoothTrailing(row, seconds, digits) {
    if (!(seconds > 1))
        return row;
    return row.map(function (p, i) {
        return { x: p.x, y: trailingAverage(row, i, seconds, digits) };
    });
}
