let fetcher_socket = null;
let fetcher_queue = {};
const FETCHER_DEFAULT_TIMEOUT = 5000;



class MyXMLHttpRequest {
    constructor(convert_incoming_headers, convert_outgoing_headers) {
        this.headers = new Headers();
        this.onload = null;
        this.onloadend = null;
        this.onerror = null;
        this.onprogress = null;
        this.onabort = null;
        this.ontimeout = null;
        this.timeout = FETCHER_DEFAULT_TIMEOUT;
        this.responseType = '';
        this.method = 'GET';
        this.url = '';
        this.convert_outgoing_headers = convert_outgoing_headers;
        this.convert_incoming_headers = convert_incoming_headers;
        this.readyState = 0;
        this.responseHeaders = null;
        this.status = null;
        this.statusText = null;
        this.response = null;
        this.responseURL = null;
        this.req = null;
    }
    open(method, url, conf) {
        this.method = method;
        this.url = url;
        this.readyState = 1;
    }

    getAllResponseHeaders() {
        if (!this.responseHeaders)
            return '';
        else {
            let rv = '';
            for (let pair of this.responseHeaders.entries()) {
                rv += pair[0].toLowerCase() + ': ' + pair[1] + '\r\n';
            }
            return rv;
        }
    }

    getResponseHeader(nm) {
        if (this.responseHeaders && this.responseHeaders.has(nm))
            return this.responseHeaders.get(nm);
        else
            return '';
    }

    setRequestHeader(h, v) {
        this.headers.append(h, v);
    }

    abort() {
        if (this.req) {
            if (this.req.timer)
                clearTimeout(this.req.timer);
            delete fetcher_queue[this.req.what.req];
        }
        if (this.onabort)
            this.onabort.bind(this)(this);
    }

    send(body) {
        let req = generate_rand_string();
        let what = {};
        what.req = req;
        what.url = this.url;
        what.responseType = this.responseType;
        what.headers = {};
        for (let pair of this.headers.entries()) {
            what.headers[pair[0]]  = pair[1];
        }
        what.body = body || '';
        what.method = this.method;
        if (what.headers && this.convert_outgoing_headers)
            what.headers = this.convert_outgoing_headers(what.headers);
        let objreq = {
            'what': what,
            'timer': null,
            'timeout': this.timeout,
            'xhr': this
        };
        if (this.convert_incoming_headers)
            objreq.convert_incoming_headers = this.convert_incoming_headers;
        this.readyState = 2;
        fetcher_queue[req] = objreq;
        if (!fetcher_socket)
            fetcher_connect();
        else
            fetcher_send_request(objreq);
        this.req = objreq;
    }
}
function XMLHttpRequest_get_class(convert_incoming_headers, convert_outgoing_headers) {
    class XMLHttpRequest extends MyXMLHttpRequest {
        constructor() {
            super(convert_incoming_headers, convert_outgoing_headers);
        }
    }
    return XMLHttpRequest;
}

function fetcher_fetch(url, what, timeout, convert_incoming_headers, convert_outgoing_headers) {
    let req = generate_rand_string();
    if (!what)
        what = {};
    what.req = req;
    what.url = url;
    if (what.headers && convert_outgoing_headers)
        what.headers = convert_outgoing_headers(what.headers);
    let objreq = {
        'what': what,
        'timer': null,
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
    if (objreq.xhr)
        objreq.xhr.readyState = 3;
    objreq.timer = setTimeout(function() {
        delete fetcher_queue[objreq.what.req];
        if (objreq.reject)
            objreq.reject(new Response(null, {'status': 408, 'statusText': 'Fetcher did not respond in time to ' + json}));
        else if (objreq.xhr.ontimeout)
            objreq.xhr.ontimeout.bind(objreq.xhr)(objreq.xhr);
    }, tim);
    console.log('Sending out '+json);
    fetcher_socket.send(json);
}

function _base64ToArrayBuffer(base64) {
    var binary_string = window.atob(base64);
    var len = binary_string.length;
    var bytes = new Uint8Array(len);
    for (var i = 0; i < len; i++) {
        bytes[i] = binary_string.charCodeAt(i);
    }
    return bytes.buffer;
}

function base64toBlob(base64Data, contentType) {
    contentType = contentType || '';
    var sliceSize = 1024;
    var byteCharacters = atob(base64Data);
    var bytesLength = byteCharacters.length;
    var slicesCount = Math.ceil(bytesLength / sliceSize);
    var byteArrays = new Array(slicesCount);

    for (var sliceIndex = 0; sliceIndex < slicesCount; ++sliceIndex) {
        var begin = sliceIndex * sliceSize;
        var end = Math.min(begin + sliceSize, bytesLength);

        var bytes = new Array(end - begin);
        for (var offset = begin, i = 0; offset < end; ++i, ++offset) {
            bytes[i] = byteCharacters[offset].charCodeAt(0);
        }
        byteArrays[sliceIndex] = new Uint8Array(bytes);
    }
    return new Blob(byteArrays, { type: contentType });
}

function fetcher_connect() {
    let mysocket = new WebSocket((location.protocol == 'https:'?'wss://' : 'ws://') + host_url + '/fetcher');
    mysocket.onopen = function (event) {
        console.log('Fetcher Upgrade HTTP connection OK');
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
            if (objreq.resolve) {
                let head = new Headers();
                let initObj = {
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
                console.log('Fetcher Returning response object '+resp);
                objreq.resolve(resp);
            }
            else {
                let head = new Headers();
                if (msg.init.headers) {
                    msg.init.headers.forEach(function(header) {
                        head.append(header[0], header[1]);
                    });
                }
                let xhr = objreq.xhr;
                xhr.responseURL = msg.init.responseURL;
                xhr.status = msg.init.status;
                xhr.statusText = msg.init.statusText;
                xhr.responseHeaders = head;
                xhr.readyState = 4;
                console.log('Preparing xhr');
                let rt = objreq.what.responseType;
                if (rt == '' || rt == 'text') {
                    xhr.response = msg.body;
                }
                else if (rt == 'arraybuffer') {
                    xhr.response = _base64ToArrayBuffer(msg.body);
                }
                else if (rt == 'blob') {
                    xhr.response = base64toBlob(msg.body);
                }
                else if (rt == 'document') {
                    xhr.response = msg.body; //change!!!
                }
                else if (rt == 'json') {
                    xhr.response = JSON.parse(msg.body);
                }
                if (xhr.onload)
                    xhr.onload.bind(xhr)(xhr);
                if (xhr.onloadend)
                    xhr.onloadend.bind(xhr)(xhr);
            }
        }
    };
}
