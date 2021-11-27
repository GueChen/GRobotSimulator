import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: form
    GridLayout{
        id: gridlayout

        anchors.top: parent.top
        anchors.topMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
        width: 300
        height: 80

        rows: 4
        columns: 2

        Label{
            text: "This is Label"
            horizontalAlignment: Text.AlignHCenter
            font.bold: true

            font.pointSize: 16
            color: "#b95d49d3"
            font.family: "Times New Roman"
        }
        Rectangle{
            id: rectangle
            Layout.fillWidth: true
            Layout.columnSpan: 1
            Layout.preferredHeight: 50
            Layout.preferredWidth: 300
            color: "#da2d3dac"
            Label {
                text: "Rectangle"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                font.pointSize: 13
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                color: "#ffffff"
            }
        }
    }

    Rectangle{

    }
}
