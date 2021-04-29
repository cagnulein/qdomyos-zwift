ct_pref = {corsAPIHost: ""};
ct_page = Page.Media;
let youtube_consent_id = Math.floor(Math.random() * 899) + 100;
function ui_addToPlaylist(b) {

}
let youtube_playlist_convert_incoming_headers = function(headers) {
    let pairs = [];
    for (let pair of headers.entries()) {
        if (pair[0] == 'set-cookie' || pair[0] == 'set-cookie2')
            pairs.push(pair[1]);
    }
    headers.append('x-set-cookies', JSON.stringify(pairs));
    return headers;
};
let youtube_playlist_convert_outgoing_headers = function(headers) {
    let removeHeaders = [
        'connection',
        'host',
        'origin',
        // Strip Heroku-specific headers
        'x-heroku-queue-wait-time',
        'x-heroku-queue-depth',
        'x-heroku-dynos-in-use',
        'x-request-start',

    ];
    removeHeaders.forEach(function(header) {
        delete headers[header];
    });
    if (headers["x-mode"]) {
        if (headers["x-mode"] == "navigate") {
            headers["sec-fetch-mode"] = "navigate";
            headers["sec-fetch-site"] = "none";
            headers["sec-fetch-user"] = "?1";
            delete headers["origin"];
        } else {
            headers["sec-fetch-mode"] = "cors";
            headers["sec-fetch-site"] = "same-origin";
            //headers["origin"] = location.protocol + "//" + location.host;
        }
        delete headers["x-mode"];
    }
    let s = "";
    if (headers["x-cookies"] && headers["x-cookies"].length)
        s = "; ";
    headers["cookie"] = headers["x-cookies"]+s + "CONSENT=YES+cb.20210328-17-p0.en+FX+"+youtube_consent_id;
    delete headers["x-cookies"];
    return headers;
};
fetch = function(url, obj) {
    return fetcher_fetch(url, obj, 12000, youtube_playlist_convert_incoming_headers, youtube_playlist_convert_outgoing_headers);
};
