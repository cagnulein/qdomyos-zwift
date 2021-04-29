var video_auto_next = true;
function bootstrapDetectBreakpoint() {
    const breakpointNames = ['xl', 'lg', 'md', 'sm', 'xs'];
    let breakpointValues = [];
    for (const breakpointName of breakpointNames) {
        breakpointValues[breakpointName] = window.getComputedStyle(document.documentElement).getPropertyValue('--breakpoint-' + breakpointName);
    }
    let i = breakpointNames.length;
    for (const breakpointName of breakpointNames) {
        i--;
        if (window.matchMedia('(min-width: ' + breakpointValues[breakpointName] + ')').matches) {
            return {name: breakpointName, index: i};
        }
    }
    return null;
}
function get_search_start_char() {
    return location.protocol.startsWith('file')?'#':'?';
}
function get_template_name() {
    let splits = location.pathname.split('/');
    if (splits.length>=2)
        return splits[splits.length - 2];
    else
        return '';
}

function get_url_without_file() {
    /*let idx = location.pathname.lastIndexOf('/');
    let end = location.pathname;
    if (idx > 0) {
        end = end.substring(0, idx + 1);
    }
    return (location.origin && location.origin !== 'null'?location.origin:location.protocol) + end;*/
    let idx = location.href.lastIndexOf('/');
    if (idx > 0 && idx <location.href.length - 1)
        return location.href.substring(0, idx + 1);
    else
        return location.href;

}

function init_width_height_from_url() {
    video_width = urlParams.has('width')?urlParams.get('width'):1702;
    video_height = urlParams.has('height')?urlParams.get('height'):673;
}
//debug 'grabber=youtube&par=UUXDorkXBjDsh0wNethPe-zQ&grabber=youtube&par=wP6l4MD1tTc&grabber=personal&par=subs'
const workout_file = 'workout.htm';
const lastconf_key = 'lastconf';
var urlParams = null;
var video_width = null;
var video_height = null;
const host_url = (!location.host || location.host.length == 0)?'192.168.25.24:7666':location.host;
let search_var = (location.protocol.startsWith('file')?window.location.hash:window.location.search).substring(1);


function dyn_module_load(link, onload) {
    let tag = document.createElement('script');
    tag.type = 'text/javascript';
    if (onload) {
        tag.addEventListener('load', function(event) {
            console.log('script loaded ' + link);
            onload();
        });
    }
    if (link.startsWith('//'))
        tag.text = link.substring(2);
    else
        tag.src = link;
    var firstScriptTag = document.getElementsByTagName('script')[0];
    firstScriptTag.parentNode.insertBefore(tag, firstScriptTag);
}

function pad(num, size) {
    num = num.toString();
    while (num.length < size) num = '0' + num;
    return num;
}
