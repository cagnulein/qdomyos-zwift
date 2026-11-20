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

    function test_saved_text_shown_and_not_dirty() {
        var f = make(numberField, { savedText: 75 })
        compare(f.text, "75")
        verify(!f.dirty)
    }

    function test_dirty_until_saved() {
        var f = make(numberField, { savedText: 75 })
        f.selectAll()
        type(f, "80")
        verify(f.dirty)
        f.savedText = 80                     // OK wrote the setting, the row binding follows
        compare(f.text, "80")
        verify(!f.dirty)
    }

    function test_back_to_saved_is_not_dirty() {
        var f = make(numberField, { savedText: 75 })
        f.selectAll()
        type(f, "80")
        verify(f.dirty)
        f.selectAll()
        type(f, "75.0")
        verify(!f.dirty)                     // same number, another spelling
    }

    function test_invalid_is_dirty() {
        var f = make(numberField, { savedText: 75, minimum: 1 })
        f.selectAll()
        keyClick(Qt.Key_Backspace)
        verify(!f.valid)
        verify(f.dirty)
    }

    function test_changed_elsewhere_replaces_text() {
        // e.g. the unit switch: the saved value is shown again in the new unit
        var f = make(numberField, { savedText: 75 })
        f.savedText = 165.35
        compare(f.text, "165.35")
        verify(!f.dirty)
    }

    // The row as on the settings page: savedText bound to the setting, OK writes the setting
    QtObject { id: store; property real weight: 75 }
    Component {
        id: boundField
        SettingsNumberField { width: 300; savedText: store.weight }
    }

    function test_ok_writes_setting_and_clears_dirty() {
        store.weight = 75
        var f = make(boundField)
        compare(f.text, "75")
        f.selectAll()
        type(f, "80,5")
        verify(f.dirty)
        store.weight = f.value               // what the OK button does
        compare(f.text, "80.5")
        verify(!f.dirty)
    }

    function test_truncate_while_typing_keeps_following_setting() {
        // onTextEdited assigns text when it drops extra decimals: the old `text:` binding broke
        // there, savedText keeps following the setting
        store.weight = 75
        var f = make(boundField)
        f.selectAll()
        type(f, "7.123")
        compare(f.text, "7.12")
        store.weight = 8
        compare(f.text, "8")
        verify(!f.dirty)
    }

    // The Text size row (settings.qml, uiTextScaleTextField): the system size or our own one
    QtObject { id: textScale; property bool system: false; property int own: 100; property int systemSize: 100 }
    Component {
        id: textScaleField
        SettingsNumberField {
            width: 300
            decimals: 0
            readonly property bool followsSystem: textScale.system
            enabled: !followsSystem
            savedText: followsSystem ? textScale.systemSize : textScale.own
            onFollowsSystemChanged: text = savedText
        }
    }

    function test_text_size_mode_change_drops_typed_value() {
        // The mode turned not by the switch of the row (the search card): the system size is the
        // saved one, savedText stays the same, and the typed 130 stayed in the field
        textScale.system = false
        textScale.own = 100
        textScale.systemSize = 100
        var f = make(textScaleField)
        f.selectAll()
        type(f, "130")
        verify(f.dirty)
        textScale.system = true
        compare(f.text, "100")
        verify(!f.dirty)
        textScale.system = false
        compare(f.text, "100")
        verify(!f.dirty)                     // a grey OK
    }

    function test_text_size_mode_change_shows_size_of_mode() {
        textScale.system = false
        textScale.own = 120
        textScale.systemSize = 100
        var f = make(textScaleField)
        f.selectAll()
        type(f, "130")
        textScale.system = true
        compare(f.text, "100")
        textScale.system = false
        compare(f.text, "120")
        verify(!f.dirty)
    }

    function test_without_saved_text_never_dirty() {
        var f = make(numberField)
        type(f, "70")
        verify(!f.dirty)
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

    function test_format_dirty() {
        var f = make(formatField, { format: "time", savedText: "00:25:00" })
        compare(f.text, "00:25:00")
        verify(!f.dirty)
        f.selectAll()
        type(f, "00:30:00")
        verify(f.dirty)
        f.savedText = "00:30:00"
        verify(!f.dirty)
        var n = make(formatField, { format: "ip" })
        type(n, "10")
        verify(!n.dirty)                     // no savedText
    }

    function test_format_other_spelling_not_dirty() {
        var t = make(formatField, { format: "time", savedText: "00:08:00" })
        t.selectAll()
        type(t, "0:8:0")
        verify(!t.dirty)
        var c = make(formatField, { format: "heightCm", savedText: 180 })
        c.selectAll()
        type(c, "180.0")
        verify(!c.dirty)
        c.selectAll()
        type(c, "181")
        verify(c.dirty)
        var i = make(formatField, { format: "heightFtIn", savedText: "5'10\"" })
        i.text = "5’10"                  // a typographic apostrophe: keyClick cannot type it
        verify(i.valid)
        verify(!i.dirty)
        i.selectAll()
        type(i, "5'11")
        verify(i.dirty)
    }

    // ---- SettingsTextField ----

    QtObject { id: textStore; property string nickname: "anon" }
    Component {
        id: textField
        SettingsTextField { width: 300; savedText: textStore.nickname }
    }

    function test_text_dirty_until_saved() {
        textStore.nickname = "anon"
        var f = make(textField)
        compare(f.text, "anon")
        verify(!f.dirty)
        type(f, "x")
        verify(f.dirty)
        textStore.nickname = f.text          // what the OK button does
        verify(!f.dirty)
        f.text = "other"
        verify(f.dirty)
        f.text = textStore.nickname
        verify(!f.dirty)
    }

    function test_text_changed_elsewhere_replaces_text() {
        textStore.nickname = "anon"
        var f = make(textField)
        f.text = "typed"
        textStore.nickname = "new"
        compare(f.text, "new")
        verify(!f.dirty)
    }

    // ---- UiComboBox / ValueComboBox ----

    // UiComboBox reads the bar paddings of the app window
    QtObject {
        id: window
        function getTopPadding() { return 0 }
        function getBottomPadding() { return 0 }
    }
    QtObject {
        id: listStore
        property string metric: "Watt"
        property int index: 1
        property string sex: "Male"
    }
    Component {
        id: textCombo
        UiComboBox {
            width: 300
            model: [ "Speed", "Watt", "Cadence" ]
            savedText: listStore.metric
            onActivated: displayText = currentValue
        }
    }
    Component {
        id: indexCombo
        UiComboBox {
            width: 300
            model: [ "Classic", "Horizontal" ]
            savedIndex: listStore.index
        }
    }
    Component {
        id: valueCombo
        ValueComboBox {
            width: 300
            model: [ "Male", "Female" ]
            labels: ({ "Male": "Mann", "Female": "Frau" })
            savedText: listStore.sex
        }
    }

    function test_combo_text_dirty_until_saved() {
        listStore.metric = "Watt"
        var c = make(textCombo)
        compare(c.displayText, "Watt")
        verify(!c.dirty)
        c.currentIndex = 2
        c.activated(2)                       // a pick as the user makes it
        compare(c.displayText, "Cadence")
        verify(c.dirty)
        listStore.metric = c.displayText     // OK
        verify(!c.dirty)
        listStore.metric = "Speed"           // changed elsewhere
        compare(c.displayText, "Speed")
        verify(!c.dirty)
    }

    function test_combo_index_dirty_until_saved() {
        listStore.index = 1
        var c = make(indexCombo)
        compare(c.currentIndex, 1)
        verify(!c.dirty)
        c.currentIndex = 0
        verify(c.dirty)
        listStore.index = c.currentIndex     // OK
        verify(!c.dirty)
        listStore.index = 1
        compare(c.currentIndex, 1)
        verify(!c.dirty)
    }

    function test_value_combo_compares_raw_value() {
        listStore.sex = "Male"
        var c = make(valueCombo)
        compare(c.value, "Male")
        compare(c.displayText, "Mann")       // the label, not the raw value
        verify(!c.dirty)
        c.currentIndex = 1
        c.activated(1)
        compare(c.value, "Female")
        compare(c.displayText, "Frau")
        verify(c.dirty)
        listStore.sex = c.value              // OK
        verify(!c.dirty)
        listStore.sex = "Male"
        compare(c.displayText, "Mann")
        verify(!c.dirty)
    }

    function test_combo_without_saved_never_dirty() {
        var c = createTemporaryObject(Qt.createComponent("../../src/UiComboBox.qml"), testCase,
                                      { model: [ "a", "b" ] })
        verify(c)
        c.currentIndex = 1
        verify(!c.dirty)
    }
}
