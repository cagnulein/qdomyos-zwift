/**
 * Author and copyright: Stefan Haack (https://shaack.com)
 * Repository: https://github.com/shaack/bootstrap-input-spinner
 * License: MIT, see file 'LICENSE'
 */

var inputSpinnerCustomEditors = {
    RawEditor: function (props, element) {
        this.parse = function (customFormat) {
            // parse nothing
            return customFormat
        }
        this.render = function (number) {
            // render raw
            return number
        }
    },
    TimeEditor: function (props, element) {
        // could be implemented more elegant maybe, but works
        this.parse = function (customFormat) {
            var trimmed = customFormat.trim()
            var sign = 1
            if (trimmed.charAt(0) === "-") {
                sign = -1
                trimmed = trimmed.replace("-", "")
            }
            var parts = trimmed.split(":")
            var hours = 0, minutes
            if (parts[1]) {
                hours = parseInt(parts[0], 10)
                minutes = parseInt(parts[1], 10)
            } else {
                minutes = parseInt(parts[0], 10)
            }
            return (hours * 60 + minutes) * sign
        }
        this.render = function (number) {
            var minutes = Math.abs(number % 60)
            if (minutes < 10) {
                minutes = "0" + minutes
            }
            var hours
            if (number >= 0) {
                hours = Math.floor(number / 60)
                return hours + ":" + minutes
            } else {
                hours = Math.ceil(number / 60)
                return "-" + Math.abs(hours) + ":" + minutes
            }
        }
    }
}
