import QtQuick 2.5
import Sailfish.Silica 1.0
import harbour.beacon 1.0
import "../js/hue-colors.js" as Color

ListItem {
    property ResourceObject light: null
    contentHeight: Theme.itemSizeMedium
    signal onChanged(bool on)
    signal brightnessChanged(int brightness)
    onPressAndHold: openMenu()
    Switch {
        id: lightSwitch
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        automaticCheck: false
        checked: light.rdata.on.on
        onCheckedChanged: busy = false
        onClicked: {
            busy = true
            onChanged(!light.rdata.on.on)
        }
    }
    Label {
        id: lightName
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: lightSwitch.right
        text: light.rdata.metadata.name
    }
    Item {
        id: lightIcon
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        //anchors.rightMargin: Theme.horizontalPageMargin
        width: height
        height: parent.height
        GlassItem {
            id: lightColor
            anchors.fill: parent
            radius: 1.0
            falloffRadius: 0.2
            visible: light.rdata.on.on && typeof(light.rdata.color) !== "undefined"
            color: visible ? Color.lightToColor(light) : "white"
        }
        Icon {
            anchors.centerIn: parent
            source: "../hueiconpack/ApiV2Archetype/" + light.rdata.metadata.archetype + ".svg"
            sourceSize.width: Theme.iconSizeSmallPlus
            sourceSize.height: Theme.iconSizeSmallPlus
        }
    }
    menu: ContextMenu {
        Slider {
            width: parent.width
            handleVisible: down || light.rdata.on.on
            minimumValue: 0
            maximumValue: 100
            stepSize: 1
            value: light.rdata.dimming.brightness
            valueText: value.toFixed() + "%"
            onSliderValueChanged: {
                if (down) {
                    brightnessChanged(sliderValue)
                }
            }
        }
    }
}
