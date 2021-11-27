import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts
import QtQuick.Controls

T.GroupBox {
    id: sliderGroup
    property string name: "关节"
    property real val: 0.0


    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    RowLayout{
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        spacing: 15

        Label {
            text: name
            font.family: "YaHei"
            color: "#ffffff"
            font.pointSize: 10
        }


        Slider {
            id: slider
            stepSize: 0.05
            from: -180
            to: 180
            value: 0.00
            Layout.fillWidth: true
            Layout.preferredWidth: 180
            Layout.preferredHeight: 50
            Layout.minimumHeight: 40

            background: Rectangle{
                x:slider.leftPadding
                y:slider.topPadding + slider.availableHeight / 2 - height/2
                implicitHeight: 4
                implicitWidth:  180
                width:slider.availableWidth
                height:implicitHeight
                radius: 2
                color: "#ddffff"

                Rectangle{
                    width: slider.visualPosition * parent.width
                    height: parent.height
                    color: "#eeffcc"
                    radius: 2
                }
            }

            handle: Rectangle {
                id: sliderHandle
                x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
                y: slider.topPadding + slider.availableHeight / 2 - height / 2
                implicitHeight: 17
                implicitWidth: 17
                radius: 8
                gradient: Gradient {
                    GradientStop {
                        position: 0
                        color: "#6c6ed4"
                    }

                    GradientStop {
                        position: 1
                        color: "#e7c8ff"
                    }
                    orientation: Gradient.Horizontal
                }
                border.color: "#dadada"
                border.width: 3
            }
            onValueChanged: {
                val = value
            }
        }
        Label {
            color: "#ffffff"
            text: Number(slider.value).toFixed(2)
            font.pointSize: 10
            Layout.preferredWidth: 80
        }
    }


}
