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

function get_template_name() {
    let splits = location.pathname.split('/');
    if (splits.length>=2)
        return splits[splits.length - 2];
    else
        return '';
}

const host_url = (!location.host || location.host.length == 0)?'192.168.25.223:7666':location.host;


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


function button_set_enabled(select, v) {
    if (!Array.isArray(select))
        select = [select];
    for(let s of select) {
        s = $(s);
        if (v)
            s.removeClass('disabled');
        else if (!s.hasClass('disabled'))
            s.addClass('disabled');
    }
}

function js_download(filename, data, mime) {
    var blob = new Blob([data], {type: mime});
    if(window.navigator.msSaveOrOpenBlob) {
        window.navigator.msSaveBlob(blob, filename);
    }
    else{
        var elem = window.document.createElement('a');
        elem.href = window.URL.createObjectURL(blob);
        elem.download = filename;        
        document.body.appendChild(elem);
        elem.click();        
        document.body.removeChild(elem);
    }
}

let XML = {
    parse: (string, type = 'text/xml') => new DOMParser().parseFromString(string, type),  // like JSON.parse
    stringify: DOM => new XMLSerializer().serializeToString(DOM),                         // like JSON.stringify

    transform: (xml, xsl) => {
        let proc = new XSLTProcessor();
        proc.importStylesheet(typeof xsl == 'string' ? XML.parse(xsl) : xsl);
        let output = proc.transformToFragment(typeof xml == 'string' ? XML.parse(xml) : xml, document);
        return typeof xml == 'string' ? XML.stringify(output) : output; // if source was string then stringify response, else return object
    },

    minify: node => XML.toString(node, false),
    prettify: node => XML.toString(node, true),
    toString: (node, pretty, level = 0, singleton = false) => { 
        if (typeof node == 'string') node = XML.parse(node);
        let tabs = pretty ? Array(level + 1).fill('').join('\t') : '';
        let newLine = pretty ? '\n' : '';
        if (node.nodeType == 3) return (singleton ? '' : tabs) + node.textContent.trim() + (singleton ? '' : newLine);
        if (!node.tagName) return XML.toString(node.firstChild, pretty);
        let output = tabs + `<${node.tagName}`; // >\n
        for (let i = 0; i < node.attributes.length; i++)
            output += ` ${node.attributes[i].name}="${node.attributes[i].value}"`;
        if (node.childNodes.length == 0) return output + ' />' + newLine;
        else output += '>';
        let onlyOneTextChild = ((node.childNodes.length == 1) && (node.childNodes[0].nodeType == 3));
        if (!onlyOneTextChild) output += newLine;
        for (let i = 0; i < node.childNodes.length; i++)
            output += XML.toString(node.childNodes[i], pretty, level + 1, onlyOneTextChild);
        return output + (onlyOneTextChild ? '' : tabs) + `</${node.tagName}>` + newLine;
    }
};