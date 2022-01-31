import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtWebView 1.1

Item {
    signal trainprogram_zwo_loaded(string s)
    id: column1
    // vedi trainprogram_open_clicked
    Settings {
        id: settings
    }

    Button {
        id: loadButton
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        enabled: false
        text: "Load"
        height: Math.max(parent.height * 0.1, 50)
        onClicked: {
            console.log(webView.rr);
            trainprogram_zwo_loaded(webView.rr);
            //popupclose();
        }
    }
    WebView {
        id: webView
        property var rr;
        anchors.top: loadButton.bottom
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width

        url: "https://whatsonzwift.com/workouts"
        visible: true
        onLoadingChanged: {
            if (loadRequest.errorString)
                console.error(loadRequest.errorString);
            if (loadRequest.status == WebView.LoadSucceededStatus) {
                console.error("Procedo");
                let loadScr = `
                let parsePace = function(s) {
                    let pace = 2;
                    if (s=='5k') pace = 1;
                    else if (s == 'HM') pace = 3;
                    else if (s == 'M') pace = 4;
                    return pace;
                };

                let parseDuration = function(txt) {
                    let re;
                    let objout = {
                        dur: 60,
                        repeat: 0,
                        durationType: null,
                        txt: txt
                    };
                    let dur = 60;
                    let repeat = 0;
                    if ((txt.indexOf('min') > 0 || txt.indexOf('sec') > 0) && (re = /([0-9]+x +)?([0-9]+min)? *([0-9]+sec)? +/.exec(txt.trim()))) {
                        let dd = 0;
                        objout.durationType = 'time';
                        objout.d_re = re;
                        for (let i = 1; i<re.length; i++) {
                            let trm1 = re[i];
                            if (!trm1) continue;
                            else if ((trm1 = trm1.trim()).endsWith('x')) {
                                objout.repeat = parseInt(re[i].substring(0, re[i].length - 1));
                            }
                            else if (trm1.endsWith('sec')) {
                                dd += parseInt(re[i].substring(0, re[i].length - 3));
                            }
                            else if (trm1.endsWith('min')) {
                                dd += parseInt(re[i].substring(0, re[i].length - 3)) * 60;
                            }
                        }
                        if (dd) objout.dur = dd;
                        objout.txt = txt.substring(re[0].length);
                    }
                    else if (re = /(?:([0-9]+)x +)?([0-9]+) +m +/.exec(txt)) {
                        objout.durationType = 'distance';
                        if (re[1]) {
                            objout.repeat = parseInt(re[1]);
                        }
                        objout.d_re = re;
                        objout.dur = parseInt(re[2]);
                        objout.txt = txt.substring(re[0].length);
                    }
                    return objout;
                };
                let processDOM = function() {
                    let outobj = {};
                    let div = document.querySelector('div.overview');
                    outobj.description = 'N/A';
                    if (div) {
                        let nextSibling = div.nextSibling;
                        while(nextSibling && (nextSibling.nodeType != 1 || nextSibling.tagName != 'P')) {
                            nextSibling = nextSibling.nextSibling;
                        }
                        if (nextSibling) outobj.description = nextSibling.innerText;
                    }
                    let durationType = null;
                    let gli = document.querySelector('h4.glyph-icon');
                    outobj.name = gli?gli.innerText:'N/A';
                    outobj.sportType = gli && gli.classList.contains('flaticon-run')?'run':'bike';
                    outobj.workout = [];
                    outobj.author = "whatsonzwift.com";
                    let wll = document.querySelector('.workoutlist');
                    let pace = null;
                    if (wll) {
                        let rexp = /(?:from +([0-9]+) +to +|@ +|)(?:[0-9]+rpm, +)?(?:([0-9]+)% +(?:of +(5k|10k|HM|M) +pace|FTP)|No Incline Walk)/; // fine idx = 2, pace idx = 3 inizio idx = 1
                        let tbs = wll.querySelectorAll('.textbar');
                        for (let i = 0; tbs && i<tbs.length; i++) {
                            let txt = tbs[i].innerText;
                            let elem = {};
                            elem.d_pretxt = txt;
                            let o = parseDuration(txt);
                            if (o.durationType) {
                                let dur = o.dur;
                                let repeat = o.repeat;
                                let OffDuration = -1;
                                let OffPower = -1;
                                let o2;
                                let re, re2;
                                elem.d_dur = o;
                                elem.d_posttxt = o.txt;
                                txt = o.txt;
                                if (re = rexp.exec(txt.trim())) {
                                    elem.d_re = re;
                                    let ln = re[0].length;
                                    if (txt.length > ln && txt.charAt(ln) == ',' && (o2 = parseDuration(txt.substring(ln + 1))) && o2.durationType && (re2 = rexp.exec(o2.txt))) {
                                        OffPower = parseInt(re2[2]);
                                        OffDuration = o2.dur;
                                    }
                                    if (re[1]) {
                                        if (i == 0) {
                                            elem.type = "Warmup";
                                        }
                                        else if (i == tbs.length - 1) {
                                            elem.type = "Cooldown";
                                        }
                                        else {
                                            elem.type = "Ramp";
                                        }
                                        elem.Duration = o.dur;
                                        elem.PowerLow = parseInt(re[1]) / 100.0;
                                        elem.PowerHigh = parseInt(re[2]) / 100.0;
                                    }
                                    else if (OffPower >= 0 && OffDuration >= 0) {
                                        elem.type = 'IntervalsT';
                                        if (o.repeat) elem.Repeat = o.repeat; else elem.Repeat = 0;
                                        elem.OnDuration = o.dur;
                                        elem.OnPower = parseInt(re[2]) / 100.0;
                                        elem.OffDuration = OffDuration;
                                        elem.OffPower = OffPower / 100.0;
                                    }
                                    else {
                                        elem.type = "SteadyState";
                                        if (o.repeat) elem.Repeat = o.repeat; else elem.Repeat = 0;
                                        elem.Duration = o.dur;
                                        elem.Power = re[2]?parseInt(re[2]) / 100.0:0.5;
                                    }
                                    if (re[3]) pace = parsePace(re[3]);
                                    if (pace) elem.pace = pace;
                                }
                                else if (txt == "free run" || txt == "free ride") {
                                    elem.type = "FreeRide";
                                    elem.Duration = o.dur;
                                }
                                if (durationType === null) {
                                    outobj.durationType = o.durationType;
                                    durationType = o.durationType;
                                }
                            }
                            if (elem) outobj.workout.push(elem);
                        }
                    }
                    return outobj;
                };
                let o = processDOM();
                let res = JSON.stringify(o);
                    res
                `;
                webView.runJavaScript(loadScr, function(res) {
                    console.log("AHO1 " + res);
                    let ro = JSON.parse(res);
                    if (ro.name && ro.workout && ro.workout.length) {
                        console.log("AHO2 " + ro);
                        webView.rr = res;
                        loadButton.text = 'Load ' + ro.name;
                        loadButton.enabled = true;
                    }
                });
            }
        }
    }
     Component.onCompleted: {
         headerToolbar.visible = true;
         webView.rr = 'ciao';
     }
}
