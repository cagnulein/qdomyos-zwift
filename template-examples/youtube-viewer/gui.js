function button_video(el, idx) {
    if ($(el).hasClass('disabled')) {
        return false;
    }
    else {
        go_to_video(idx);
    }
}

function clear_playlist() {
    $('ul.dropdown-menu').empty();
}

function add_video_to_button(item) {
    let li = $('<li>');
    let a = $('<a>');
    a.attr('href', '#');
    a.addClass('dropdown-item');
    a.attr('data-uid', item.uid);
    a.text(item.title);
    a.click(function (e) {
        go_to_video(item.uid);
    });
    li.append(a);
    $('ul.dropdown-menu').append(li);
}

function set_playlist_button_enabled(enabled) {
    if (!enabled)
        $('#playlist_items').addClass('disabled');
    else
        $('#playlist_items').removeClass('disabled');
}

function remove_playlist_button() {
    $('#playlist_items_cont').remove();
}

function set_prev_button_enabled(enabled) {
    if (!enabled)
        $('#prev_button').addClass('disabled');
    else
        $('#prev_button').removeClass('disabled');
}

function set_next_button_enabled(enabled) {
    if (!enabled)
        $('#next_button').addClass('disabled');
    else
        $('#next_button').removeClass('disabled');
}

function set_video_title(title) {
    $('#video_title').text(title);
}

function set_video_enabled(uid) {
    $('ul.dropdown-menu a').removeClass('active');
    $('ul.dropdown-menu a[data-uid=\'' + uid + '\']').addClass('active');
}
