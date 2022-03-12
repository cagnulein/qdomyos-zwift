if (typeof PeertubeGrabber === 'function')
    on_grabber_load('peertube', new PeertubeGrabber('peertube.co.uk', 'peertube', urlParams));
else
    dyn_module_load('./peertubecommon_grabber.js', function() {
        on_grabber_load('peertube', new PeertubeGrabber('peertube.co.uk', 'peertube', urlParams));
    });