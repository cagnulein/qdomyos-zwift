const host_url = (!location.host || location.host.length == 0)?'192.168.25.24:7666':location.host;

function get_template_name() {
    let splits = location.pathname.split('/');
    if (splits.length>=2)
        return splits[splits.length - 2];
    else
        return '';
}