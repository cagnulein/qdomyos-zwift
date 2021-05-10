// Below is just a demonstration and is not tested thoroughly for 
// production-ready web applications by any means.  
// But it should give you an idea.

/** 
 * Caches the user-input data from the targeted form, stores it in the cookies 
 * and fetches back to the form when requested or needed. 
 */
var formCache = (function () {
    var _form = null, 
        _formData = [],
        _strFormElements = 'input[type=\'text\'],'
                + 'input[type=\'checkbox\'],' 
                + 'input[type=\'radio\'],' 
                // + "input[type='password'],"  // leave password field out 
                + 'input[type=\'hidden\'],'
                // + "input[type='image'],"
                + 'input[type=\'file\'],'
                // more input types...
                + 'input[type=\'email\'],'
                + 'input[type=\'number\'],'
                + 'input[type=\'tel\'],'
                + 'input[type=\'url\'],'
                + 'select,'
                + 'textarea';

    function _warn() {
        console.log('formCache is not initialized.');
    }

    return {

        /**
         * Initializes the formCache with a target form (id). 
         * You can pass any container id for the formId parameter, formCache will 
         * still look for form elements inside the given container. If no form id 
         * is passed, it will target the first <form> element in the DOM. 
         */
        init: function (formId) {
            var f = (typeof formId === 'undefined' || formId === null || $.trim(formId) === '') 
                ? $('form').first() 
                : $('#' + formId);
            _form = f.length > 0 ? f : null;
            console.log(_form);
            return formCache; // make it chainable
        },

        /** 
         * Stores the form data in the cookies.
         */
        save: function () {
            if (_form === null) return _warn();

            _form
                .find(_strFormElements)
                .each(function() {
                    var f = $(this).attr('id') + ':' + formCache.getFieldValue($(this));
                    _formData.push(f);
                });
            docCookies.setItem('formData', _formData.join(), 31536e3); // 1 year expiration (persistent)
            console.log('Cached form data:', _formData);
            return formCache;
        },

        /** 
         * Fills out the form elements from the data previously stored in the cookies.
         */
        fetch: function () {
            if (_form === null) return _warn();

            if (!docCookies.hasItem('formData')) return;
            var fd = _formData.length < 1 ? docCookies.getItem('formData').split(',') : _formData;
            $.each(fd, function (i, item) {
                var s = item.split(':');
                var elem = $('#' + s[0]);
                formCache.setFieldValue(elem, s[1]);
            });
            return formCache;
        },

        /** 
         * Sets the value of the specified form field from previously stored data.
         */
        setFieldValue: function (elem, value) {
            if (_form === null) return _warn();

            if (elem.is('input:text') || elem.is('input:hidden') || elem.is('input:image') ||
                    elem.is('input:file') || elem.is('textarea')) {
                elem.val(value);
            } else if (elem.is('input:checkbox') || elem.is('input:radio')) {
                elem.prop('checked', value);
            } else if (elem.is('select')) {
                elem.prop('selectedIndex', value);
            }
            return formCache;
        },

        /**
         * Gets the previously stored value of the specified form field.
         */
        getFieldValue: function (elem) {
            if (_form === null) return _warn();

            if (elem.is('input:text') || elem.is('input:hidden') || elem.is('input:image') ||
                elem.is('input:file') || elem.is('input:number') || elem.is('textarea')) {
                return elem.val();
            } else if (elem.is('input:checkbox') || elem.is('input:radio')) {
                return elem.prop('checked');
            } else if (elem.is('select')) {
                return elem.prop('selectedIndex');
            }
            else return null;
        },

        /**
         * Clears the cache and removes the previously stored form data from cookies.
         */
        clear: function () {
            _formData = [];
            docCookies.removeItem('formData');
            return formCache;
        },

        /**
         * Clears all the form fields. 
         * This is different from form.reset() which only re-sets the fields 
         * to their initial values.
         */
        clearForm: function () {
            _form
                .find(_strFormElements)
                .each(function() {
                    var elem = $(this);
                    if (elem.is('input:text') || elem.is('input:password') || elem.is('input:hidden') || 
                        elem.is('input:image') || elem.is('input:file') || elem.is('textarea')) {
                        elem.val('');
                    } else if (elem.is('input:checkbox') || elem.is('input:radio')) {
                        elem.prop('checked', false);
                    } else if (elem.is('select')) {
                        elem.prop('selectedIndex', -1);
                    }
                });
            return formCache;
        }
    };
})();

// Save form data right before we unload the form-page
$(window).on('beforeunload', function (event) {
    formCache.save();
    return false;
});