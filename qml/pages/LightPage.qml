import QtQuick 2.5
import Sailfish.Silica 1.0
import harbour.beacon 1.0

import "../components"

Page {
    id: page

    property ResourceObject device
    property ResourceObject light
    property string name: light.rdata.metadata.name
    property string archeType: light.rdata.metadata.archetype

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                text: qsTr("Identify")
                onClicked: bridge.putResource(HueBridge.ResourceDevice, { identify: { action: "identify" } }, device.rid)
            }
        }

        Column {
            id: column
            width: parent.width

            PageHeader {
                id: pageHeader

                Switch {
                    id: lightSwitch
                    anchors.verticalCenter: pageHeader.extraContent.verticalCenter
                    anchors.right: pageHeader.extraContent.right
                    anchors.rightMargin: Theme.paddingSmall
                    automaticCheck: false
                    checked: light.rdata.on.on
                    onCheckedChanged: busy = false
                    onClicked: {
                        busy = true
                        bridge.setLight(light.rid, { on: { on: !light.rdata.on.on } } )
                    }
                }
                title: name
                description: device.rdata.product_data.product_name
            }

            ColorPicker {
                id: colorPicker
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                rgbaVisible: typeof(light.rdata.color) !== "undefined"
                ctVisible: typeof(light.rdata.color_temperature) !== "undefined"
                visible: rgbaVisible && ctVisible
            }
            Slider {
                id: dimmSlider
                anchors.left: parent.left
                anchors.right: parent.right
                handleVisible: down || light.rdata.on.on
                minimumValue: 0
                maximumValue: 100
                stepSize: 1
                value: typeof(light.rdata.dimming) !== "unedefined" ? light.rdata.dimming.brightness : 0
                valueText: value.toFixed() + "%"
                onSliderValueChanged: {
                    if (down) {
                        bridge.setLight(light.rid, { on: { on: true }, dimming: { brightness: sliderValue } })
                    }
                }
                label: qsTr("Brightness")
            }
        }
    }
}
