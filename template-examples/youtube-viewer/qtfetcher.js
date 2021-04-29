let fetcher_socket = null;
let fetcher_queue = {};
const FETCHER_DEFAULT_TIMEOUT = 5000;
function fetcher_fetch(url, what, timeout, convert_incoming_headers, convert_outgoing_headers) {
    let req = generate_rand_string();
    if (!what)
        what = {};
    what.req = req;
    what.url = url;
    if (what.headers && convert_outgoing_headers)
        what.headers = convert_outgoing_headers(what.headers);
    let objreq = {
        "what": what,
        "timer": null,
    };
    if (timeout)
        objreq.timeout = timeout;
    if (convert_incoming_headers)
        objreq.convert_incoming_headers = convert_incoming_headers;
    let p = new Promise(function(resolve, reject) {
        objreq.resolve = resolve;
        objreq.reject = reject;
    });
    fetcher_queue[req] = objreq;
    if (!fetcher_socket)
        fetcher_connect();
    else
        fetcher_send_request(objreq);
    return p;
}

function fetcher_send_request(objreq) {
    let tim = objreq.timeout || FETCHER_DEFAULT_TIMEOUT;
    let json = JSON.stringify(objreq.what);
    objreq.timer = setTimeout(function() {
        delete fetcher_queue[objreq.what.req];
        objreq.reject(new Response(null, {"status": 408, "statusText": "Fetcher did not respond in time to " + json}));
    }, tim);
    console.log("Sending out "+json);
    fetcher_socket.send(json);
}

function generate_rand_string() {
    if (window.crypto && window.crypto.getRandomValues) {
        var rnd = new Uint8Array(16);
        window.crypto.getRandomValues(rnd);
        var cpn = "";
        for (var c = 0; c < 16; c++)
            cpn += "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_".charAt(rnd[c] & 63);
        return cpn;
    }
    return "";
}

function fetcher_connect() {
    let mysocket = new WebSocket("ws://" + host_url + "/fetcher");
    mysocket.onopen = function (event) {
        console.log("Fetcher Upgrade HTTP connection OK");
        fetcher_socket = mysocket;
        for (let [key, value] of Object.entries(fetcher_queue)) {
            if (!value.timer) {
                fetcher_send_request(value);
            }
        }
    };
    mysocket.onclose = function(e) {
        fetcher_socket = null;
        console.log('Fetcher Socket is closed. Reconnect will be attempted in 30 second.', e.reason);
        setTimeout(function() {
            fetcher_connect();
        }, 30000);
    };

    mysocket.onerror = function(err) {
        fetcher_socket = null;
        console.error('Fetcher Socket encountered error: ', err.message, 'Closing socket');
        mysocket.close();
    };
    mysocket.onmessage = function (event) {
        console.log(event.data);
        let msg = JSON.parse(event.data);
        let objreq;
        if (msg.req && (objreq = fetcher_queue[msg.req])) {
            if (objreq.timer)
                clearTimeout(objreq.timer);
            delete fetcher_queue[msg.req];
            let head = new Headers();
            initObj = {
                status: msg.init.status,
                statusText: msg.init.statusText,
                headers: head
            };
            if (msg.init.headers) {
                msg.init.headers.forEach(function(header) {
                    head.append(header[0], header[1]);
                });
            }
            if (objreq.convert_incoming_headers && initObj.headers)
                initObj.headers = objreq.convert_incoming_headers(initObj.headers);

            let resp = new Response(msg.body, initObj /* headersStatusStatusText */ );
            console.log("Fetcher Returning response object "+resp);
            objreq.resolve(resp);
        }
    };
}
