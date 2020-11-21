import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2

HomeForm{
    objectName: "home"
    signal start_clicked;
    signal stop_clicked;

    start.onClicked: { start_clicked(); }
    stop.onClicked: { stop_clicked(); }
    Component.onCompleted: { console.log("completed"); }
}
