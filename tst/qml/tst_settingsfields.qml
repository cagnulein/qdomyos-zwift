import QtQuick 2.14
import QtTest 1.14
import "../../src"

// SettingsNumberField / SettingsFormatField: typing into the field as a user does.
// Run: qmltestrunner -input tst/qml   (QT_QPA_PLATFORM=offscreen, QT_QUICK_CONTROLS_STYLE=Material)
TestCase {
    id: testCase
    name: "SettingsFields"
    width: 400
    height: 200
    when: windowShown

    Component { id: numberField; SettingsNumberField { width: 300 } }
    Component { id: formatField; SettingsFormatField { width: 300 } }

    function make(component, props) {
        var field = createTemporaryObject(component, testCase, props || {})
        verify(field)
        field.forceActiveFocus()
        return field
    }

    // Types the characters one by one, as the keyboard does: the validator refuses some of them
    function type(field, chars) {
        for (var i = 0; i < chars.length; i++)
            keyClick(chars[i])
        return field.text
    }

    // ---- SettingsNumberField ----

    function test_comma_is_a_point() {
        var f = make(numberField)
        compare(type(f, "72,5"), "72,5")
        compare(f.value, 72.5)
        verify(f.valid)
    }

    function test_one_number_only() {
        var f = make(numberField)
        compare(type(f, "170.5.5"), "170.55")
        f.text = ""
        compare(type(f, "7a"), "7")
        f.text = ""
        compare(type(f, "1 2"), "12")
    }

    function test_minus_only_when_signed() {
        var f = make(numberField)
        compare(type(f, "-5"), "5")
        var s = make(numberField, { signed: true })
        compare(type(s, "-5.25"), "-5.25")
        compare(s.value, -5.25)
        verify(s.valid)
    }

    function test_incomplete_is_not_valid() {
        var s = make(numberField, { signed: true })
        compare(type(s, "-"), "-")
        verify(!s.valid)
        compare(s.errorText, "Enter a number")
        var f = make(numberField)
        verify(!f.valid)
        compare(f.errorText, "Enter a number")
    }

    function test_integer_takes_no_point() {
        var f = make(numberField, { decimals: 0 })
        compare(type(f, "35.7"), "357")
        f.text = ""
        compare(type(f, "4,5"), "45")
        f.text = ""
        verify(!f.valid)
        compare(f.errorText, "Enter a whole number")
    }

    function test_two_decimals_typed() {
        var f = make(numberField)
        compare(type(f, "1.2345"), "1.23")
    }

    function test_range() {
        var f = make(numberField, { decimals: 0, minimum: 1, maximum: 120 })
        type(f, "150")
        verify(!f.valid)
        compare(f.errorText, "Allowed range: 1 to 120")
        f.text = "35"
        verify(f.valid)
        var m = make(numberField, { minimum: 1 })
        type(m, "0")
        verify(!m.valid)
        compare(m.errorText, "Minimum: 1")
        var x = make(numberField, { maximum: 100 })
        type(x, "120")
        compare(x.errorText, "Maximum: 100")
    }

    function test_enter_only_when_acceptable() {
        var f = make(numberField)
        var accepted = 0
        f.accepted.connect(function() { accepted++ })
        keyClick(Qt.Key_Return)
        compare(accepted, 0)
        type(f, "70")
        keyClick(Qt.Key_Return)
        compare(accepted, 1)
    }

    function test_saved_wrong_value_is_shown() {
        // A negative weight saved before the check: shown as it is, with the reason
        var f = make(numberField, { text: "-70", minimum: 1 })
        compare(f.text, "-70")
        verify(!f.valid)
        compare(f.errorText, "The value cannot be negative")
    }

    function test_longer_saved_value_stays_valid() {
        // e.g. pounds converted from kilograms: more decimals than can be typed
        var f = make(numberField, { text: "154.32358" })
        verify(f.valid)
        compare(f.value, 154.32358)
    }

    // ---- SettingsFormatField ----

    function test_time() {
        var f = make(formatField, { format: "time" })
        compare(type(f, "5 min"), "5")
        f.text = ""
        type(f, "00:07:")
        verify(!f.valid)
        compare(f.errorText, "Enter the time as hh:mm:ss")
        type(f, "30")
        compare(f.text, "00:07:30")
        verify(f.valid)
        f.text = "00:61:00"
        verify(!f.valid)
    }

    function test_ip() {
        var f = make(formatField, { format: "ip" })
        verify(f.valid)                      // empty: feature off
        compare(type(f, "192.168.1.a"), "192.168.1.")
        verify(!f.valid)
        compare(f.errorText, "Enter an IP address, e.g. 192.168.1.10")
        type(f, "10")
        verify(f.valid)
        f.text = "256.1.1.1"
        verify(!f.valid)
    }

    function test_host() {
        var f = make(formatField, { format: "host" })
        compare(type(f, "treadmill.local"), "treadmill.local")
        verify(f.valid)
        f.text = ""
        compare(type(f, "192.168.1.10:5555"), "192.168.1.10:5555")
        verify(f.valid)
        f.text = ""
        compare(type(f, "a b"), "ab")
        f.text = "192.168.1.10."
        verify(!f.valid)
        compare(f.errorText, "Enter an IP address or a host name, e.g. 192.168.1.10")
    }

    function test_height() {
        var c = make(formatField, { format: "heightCm" })
        compare(type(c, "17a5"), "175")
        verify(c.valid)
        c.text = "177.79999999999998"       // converted from feet and inches
        verify(c.valid)
        var i = make(formatField, { format: "heightFtIn" })
        type(i, "5'10\"")
        compare(i.text, "5'10\"")
        verify(i.valid)
        i.text = "5"
        verify(!i.valid)
        compare(i.errorText, "Enter the height as feet'inches, e.g. 5'10\"")
    }
}
