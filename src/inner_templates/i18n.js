(function () {
    window.QZ_TRANSLATIONS = window.QZ_TRANSLATIONS || {};

    window.qzSetTranslations = function (translations) {
        window.QZ_TRANSLATIONS = translations || {};
        let event;
        if (typeof CustomEvent === 'function') {
            event = new CustomEvent('qz-translations-updated');
        } else {
            event = document.createEvent('Event');
            event.initEvent('qz-translations-updated', true, true);
        }
        document.dispatchEvent(event);
    };

    window.qzTranslate = function (key, fallback) {
        const translated = window.QZ_TRANSLATIONS && window.QZ_TRANSLATIONS[key];
        return translated || fallback || key;
    };

    window.qzApplyTranslations = function (root) {
        const container = root || document;
        container.querySelectorAll('[data-i18n]').forEach((el) => {
            el.textContent = window.qzTranslate(el.dataset.i18n, el.textContent);
        });
        container.querySelectorAll('[data-i18n-placeholder]').forEach((el) => {
            el.setAttribute('placeholder', window.qzTranslate(el.dataset.i18nPlaceholder, el.getAttribute('placeholder') || ''));
        });
        container.querySelectorAll('[data-i18n-title]').forEach((el) => {
            el.setAttribute('title', window.qzTranslate(el.dataset.i18nTitle, el.getAttribute('title') || ''));
        });
        // The name a screen reader reads, when it differs from the visible text
        container.querySelectorAll('[data-i18n-aria-label]').forEach((el) => {
            el.setAttribute('aria-label', window.qzTranslate(el.dataset.i18nAriaLabel, el.getAttribute('aria-label') || ''));
        });
    };

    window.qzRequestTranslations = function () {
        if (typeof MainWSQueueElement === 'undefined') {
            return Promise.resolve(null);
        }
        const request = new MainWSQueueElement({ msg: 'webtranslations', content: {} }, function (response) {
            if (response.msg === 'R_webtranslations') {
                return response.content;
            }
            return null;
        }, 15000, 3);
        return request.enqueue().then((translations) => {
            if (translations) {
                window.qzSetTranslations(translations);
            }
            return translations;
        });
    };

    function requestTranslationsWhenReady(attempts) {
        if (window.QZ_OFFLINE || attempts <= 0) {
            return;
        }
        if (typeof MainWSQueueElement !== 'undefined') {
            window.qzRequestTranslations().catch(() => {});
            return;
        }
        setTimeout(() => requestTranslationsWhenReady(attempts - 1), 250);
    }

    document.addEventListener('DOMContentLoaded', () => {
        window.qzApplyTranslations();
        requestTranslationsWhenReady(20);
    });
    document.addEventListener('qz-translations-updated', () => window.qzApplyTranslations());
}());
