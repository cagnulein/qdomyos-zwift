let main_ws = null;
let main_ws_queue = [];

class MainWSQueueElement {
    constructor(msg_to_send, _inner_process, timeout, retry_num) {
        this.msg_to_send = msg_to_send;
        this.needs_to_send = msg_to_send != null;
        this.timeout = timeout || 5000;
        this.retry_num = retry_num || 1;
        this.timer = null;
        this.resolve = null;
        this.reject = null;
        this._inner_process = _inner_process;
    }

    inner_process_msg(msg) {
        if (this._inner_process)
            return this._inner_process(msg);
        else
            return {};
    }

    process_arrived_msg(msg) {
        let out = this.inner_process_msg(msg);
        if (out) {
            if (this.timer !==null) {
                clearTimeout(this.timer);
                this.timer = null;
            }
            if (this.resolve)
                setTimeout(function() { this.resolve(out); }.bind(this), 0);
        }
        return out;
    }

    enqueue() {
        main_ws_enqueue(this);
        return new Promise(function(resolve, reject) {
            this.resolve = resolve;
            this.reject = reject;
        }.bind(this));
    }

    pop_msg_to_send() {
        if (this.needs_to_send) {
            this.needs_to_send = false;
            if (this.retry_num < 0 || this.retry_num > 0)
                this.timer = setTimeout(function() {
                    this.timer = null;
                    if (this.retry_num == 0) {
                        main_ws_dequeue(this);
                        if (this.reject) {
                            this.reject(new Error('Timeout error detected'));
                        }
                    }
                    else {
                        this.needs_to_send = true;
                        main_ws_enqueue();
                    }
                }.bind(this), this.timeout);
            if (this.retry_num > 0)
                this.retry_num--;
            return this.msg_to_send;
        }
        else
            return null;
    }
}

function main_ws_dequeue(el) {
    let idx = main_ws_queue.indexOf(el);
    if (idx >= 0) {
        main_ws_queue.splice(idx, 1);
    }
}

function main_ws_enqueue(el) {
    if (el)
        main_ws_queue.push(el);
    if (main_ws)
        main_ws_queue_process();
}

function main_ws_queue_process(msg) {
    if (!main_ws)
        return;
    let jsonobj;
    for (let i = 0; i < main_ws_queue.length; i++) {
        let el = main_ws_queue[i];
        if ((jsonobj = el.pop_msg_to_send())) {
            let logString = JSON.stringify(jsonobj);
            main_ws.send(logString);
            console.log('WS >> ' + logString);
        }
        else if (msg) {
            if (el.process_arrived_msg(msg)) {
                main_ws_queue.splice(i, 1);
                i--;
                msg = null;
            }
        }
    }
}


// the socket being opened or open (main_ws is set only once it is open); a socket that is not this
// one any more was closed by main_ws_pause and is left alone
let main_ws_socket = null;
// A page kept hidden (the workout editor of the modern look, main.qml) leaves the server while it is
// hidden, or it would take every update of the workout for nothing: main_ws_pause(true) closes the
// socket and keeps it closed, main_ws_pause(false) connects again. What is sent meanwhile waits in
// the queue and goes once the socket is open again
let main_ws_paused = false;

function main_ws_pause(paused) {
    paused = !!paused;
    if (paused === main_ws_paused)
        return;
    main_ws_paused = paused;
    if (paused) {
        let socket = main_ws_socket;
        main_ws_socket = null;
        main_ws = null;
        if (socket) {
            console.log('Socket is closed while the page is hidden');
            socket.close();
        }
    } else if (!main_ws_socket) {
        main_ws_connect();
    }
}

function main_ws_connect() {
    let socket = new WebSocket((location.protocol == 'https:'?'wss://' : 'ws://') + host_url + '/' + get_template_name() + '-ws');
    main_ws_socket = socket;
    socket.onopen = function (event) {
        if (main_ws_socket !== socket) {
            socket.close();
            return;
        }
        console.log('Upgrade HTTP connection OK');
        main_ws = socket;
        main_ws_queue_process();
    };
    socket.onclose = function(e) {
        if (main_ws_socket !== socket)
            return;
        main_ws = null;
        main_ws_socket = null;
        console.log('Socket is closed. Reconnect will be attempted in 30 second.', e.reason);
        setTimeout(function() {
            if (!main_ws_paused && !main_ws_socket)
                main_ws_connect();
        }, 5000);
    };

    socket.onerror = function(err) {
        if (main_ws === socket)
            main_ws = null;
        console.error('Socket encountered error: ', err.message, 'Closing socket');
        socket.close();
    };
    socket.onmessage = function (event) {
        if (main_ws_socket !== socket)
            return;
        console.log(event.data);
        let msg = JSON.parse(event.data);
        main_ws_queue_process(msg);
    };
}
main_ws_connect();

if (typeof window !== 'undefined' && window.QZ_OFFLINE) {
    main_ws_connect = function() {};
    main_ws_enqueue = function(el) {
        if (el && typeof el.resolve === 'function') {
            el.resolve(null);
        }
    };
    main_ws_queue_process = function(msg) {
        if (msg) {
            for (const el of main_ws_queue.splice(0, main_ws_queue.length)) {
                if (el && typeof el.resolve === 'function') {
                    el.resolve(null);
                }
            }
        }
    };
}
