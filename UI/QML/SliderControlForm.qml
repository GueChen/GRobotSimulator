import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Item {
    id: sliderPlane

    width: 250
    height: 800

    //property alias custom: custom
    GridLayout {
        id: gridlayout
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.horizontalCenter: parent.horizontalCenter
        columnSpacing: 10
        rowSpacing: 50
        rows: 8
        columns: 3

        CustomSlider {
            id: slider0
            name: "关节 0"
            Layout.fillWidth: true
            Layout.columnSpan: 3
            onValChanged: {
                robot.setLeftArmRotation(0, val);
            }
        }
        CustomSlider {
            id: slider1
            name: "关节 1"
            Layout.fillWidth: true
            Layout.columnSpan: 3
            onValChanged: {
                robot.setLeftArmRotation(1, val);
            }
        }
        CustomSlider {
            id: slider2
            name: "关节 2"
            Layout.fillWidth: true
            Layout.columnSpan: 3
            onValChanged: {
                robot.setLeftArmRotation(2, val);
            }
        }
        CustomSlider {
            id: slider3
            name: "关节 3"
            Layout.fillWidth: true
            Layout.columnSpan: 3
            onValChanged: {
                robot.setLeftArmRotation(3, val);
            }
        }
        CustomSlider {
            id: slider4
            name: "关节 4"
            Layout.fillWidth: true
            Layout.columnSpan: 3
            onValChanged: {
                robot.setLeftArmRotation(4, val);
            }
        }
        CustomSlider {
            id: slider5
            name: "关节 5"
            Layout.fillWidth: true
            Layout.columnSpan: 3
            onValChanged: {
                robot.setLeftArmRotation(5, val);
            }
        }
        CustomSlider {
            id: slider6
            name: "关节 6"
            Layout.fillWidth: true
            Layout.columnSpan: 3
            onValChanged: {
                robot.setLeftArmRotation(6, val);
            }
        }
        Button{
            id: button
            text: "Send"
            Layout.row: 8
            Layout.column: 0
            Layout.columnSpan: 2
            contentItem: Text{
                text: button.text
                font: button.font
                opacity: 1.0
                color: button.down ? "#000000" : "#ffffff"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            background: Rectangle{
                implicitWidth: 120
                implicitHeight: 25
                opacity: 1.0
                radius: 5
                color: button.down? "#aa22aa":"#0055cc"
                border.width: 2
            }

            onClicked: {
                console.log("Slider 6 val now is :" + slider6.val)
                myMessager.say(slider6.val);
                myMessager.hello()
            }
        }
    }
}
