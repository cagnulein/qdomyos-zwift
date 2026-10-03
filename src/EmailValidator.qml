import QtQuick 2.14

// Accepts an empty string (feature off) or a single address without spaces: name@domain.tld
// Kept in its own file: RegularExpressionValidator needs QtQuick 2.14, settings.qml imports 2.7
RegularExpressionValidator {
    regularExpression: /^$|^[^\s@]+@[^\s@]+\.[^\s@]+$/
}
