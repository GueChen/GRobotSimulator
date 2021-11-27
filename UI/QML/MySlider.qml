import QtQuick 2.5
import QtQuick.Controls
import QtQuick.Layouts

Rectangle{
    id: pop
    anchors.fill: parent
    Slider {
        id: slider
        x: 0
        y: 0
        width: 100
        height: 20
        value: 50
        from: 0
        to: 100

        handle: Rectangle{
            id: handle
            x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            radius: 7
            border.color: "#b4d2ae"
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#4facfe"
                }

                GradientStop {
                    position: 1
                    color: "#fff2fe"
                }
            }
            implicitHeight: 15
            implicitWidth: 15
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
