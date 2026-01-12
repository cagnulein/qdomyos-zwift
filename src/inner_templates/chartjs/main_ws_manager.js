let main_ws = null;
let main_ws_queue = [];
class MainWSQueueElement {
    constructor(msg_to_send, inner_process, timeout, retry_num) {
        this.msg_to_send = msg_to_send;
        this.needs_to_send = msg_to_send != null;
        this.timeout = timeout || 5000;
        this.retry_num = retry_num || 1;
        this.timer = null;
        this.resolve = null;
        this.reject = null;
        this._inner_process = inner_process;
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

// Function to get WebSocket port (HTTP port + 1)
function getWebSocketPort() {
    // Extract current port from URL
    let currentPort = window.location.port;

    // If port is not explicitly set in URL, use default ports (80 for HTTP, 443 for HTTPS)
    if (!currentPort) {
        currentPort = (window.location.protocol === 'https:') ? 443 : 80;
    }

    // Add 1 to get WebSocket port
    return parseInt(currentPort) + 1;
}

function main_ws_connect() {
    // Get the WebSocket port (HTTP port + 1)
    let wsPort = getWebSocketPort();

    // Construct the WebSocket URL with the correct port
    let wsProtocol = (location.protocol === 'https:') ? 'wss://' : 'ws://';
    let wsHost = window.location.hostname;
    let wsUrl = wsProtocol + wsHost + ':' + wsPort + '/' + get_template_name();

    console.log('Connecting to WebSocket at: ' + wsUrl);

    let socket = new WebSocket(wsUrl);
    socket.onopen = function (event) {
        console.log('WebSocket connection established successfully');
        main_ws = socket;
        main_ws_queue_process();
    };

    socket.onclose = function(e) {
        main_ws = null;
        console.log('Socket is closed. Reconnect will be attempted in 5 seconds.', e.reason);
        setTimeout(function() {
            main_ws_connect();
        }, 5000);
    };

    socket.onerror = function(err) {
        main_ws = null;
        console.error('Socket encountered error: ', err.message, 'Closing socket');
        socket.close();
    };

    socket.onmessage = function (event) {
        console.log('WS << ' + event.data);
        try {
            let msg = JSON.parse(event.data);
            main_ws_queue_process(msg);
        } catch (e) {
            console.error('Error parsing WebSocket message:', e);
        }
    };
}

// Initialize WebSocket connection
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
