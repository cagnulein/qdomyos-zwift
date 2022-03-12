if (typeof PeertubeGrabber === 'function')
    on_grabber_load('skrepin', new PeertubeGrabber('tube.skrep.in', 'skrepin', urlParams));
else
    dyn_module_load('./peertubecommon_grabber.js', function() {
        on_grabber_load('skrepin', new PeertubeGrabber('tube.skrep.in', 'skrepin', urlParams));
    });