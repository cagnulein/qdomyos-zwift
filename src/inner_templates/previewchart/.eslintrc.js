module.exports = {
    'env': {
        'browser': true,
        'es2021': true,
        'commonjs': true,
        'es6': true,
        'jquery': true
    },
    'extends': 'eslint:recommended',
    'parserOptions': {
        'ecmaVersion': 12,
        //'sourceType': 'module'
    },
    'globals': {
        'host_url': true,
        'MainWSQueueElement': true,
        'Chart': true,
        'get_template_name': true
    },
    'rules': {
        'indent': [
            'error',
            4
        ],
        'linebreak-style': [
            'error',
            'windows'
        ],
        'quotes': [
            'error',
            'single'
        ],
        'semi': [
            'error',
            'always'
        ],
        'no-unused-vars': ['off']
    }
};
