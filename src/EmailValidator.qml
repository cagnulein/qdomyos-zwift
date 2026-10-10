import QtQuick 2.14

// Accepts an empty string (feature off) or a single address in Latin letters: name@domain.tld
// (other letters and spaces cannot be typed)
// Kept in its own file: RegularExpressionValidator needs QtQuick 2.14, settings.qml imports 2.7
RegularExpressionValidator {
    regularExpression: /^$|^[A-Za-z0-9._%+-]+@[A-Za-z0-9-]+(\.[A-Za-z0-9-]+)*\.[A-Za-z]{2,}$/
}
