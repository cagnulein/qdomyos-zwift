const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');

for (const template of ['chartjs', 'floating', 'googlemaps', 'maps2d']) {
    const source = fs.readFileSync(`src/inner_templates/${template}/main_ws_manager.js`, 'utf8');
    for (const [protocol, port, expected] of [
        ['http:', '6666', `ws://qz.local:6667/${template}`],
        ['https:', '', `wss://qz.local:444/${template}`],
    ]) {
        const sockets = [];
        const timers = [];
        class WebSocket {
            constructor(url) { this.url = url; sockets.push(this); }
            close() { this.closed = true; }
            send() {}
        }
        const context = {WebSocket, location: {protocol, port, hostname: 'qz.local'},
            window: {}, console: {log() {}, error() {}}, get_template_name: () => template,
            setTimeout: callback => { timers.push(callback); return timers.length; }, clearTimeout() {}};
        vm.createContext(context);
        vm.runInContext(source, context);
        assert.equal(sockets[0].url, expected, `${template} uses the WebSocket port`);
        sockets[0].onopen({});
        sockets[0].onclose({reason: 'test'});
        assert.equal(timers.length, 1, `${template} schedules reconnect`);
        timers.shift()();
        assert.equal(sockets[1].url, expected, `${template} reconnects to the same port`);
        if (template === 'chartjs') {
            context.main_ws_pause(true);
            assert.equal(sockets[1].closed, true);
            context.main_ws_pause(false);
            assert.equal(sockets[2].url, expected);
        }
    }
}
console.log('Qt 6 WebSocket template client tests passed');
