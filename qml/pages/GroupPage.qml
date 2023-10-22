import QtQuick 2.5
import Sailfish.Silica 1.0
import harbour.beacon 1.0

import "../components"
import "../js/hue-colors.js" as Color

Page {
    id: page

    property var grouped_light
    property var group_owner

    SilicaFlickable {
        anchors.fill: parent

        Column {
            id: column

            anchors.fill: parent
            PageHeader {
                id: pageHeader
                title: group_owner.rdata.metadata.name
            }

            Switch {
                id: groupIconSwitch
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                icon.source: "../hueiconpack/HueIconPack2019/" + roomArchetypeImages[group_owner.rdata.metadata.archetype]
                icon.sourceSize.width: Theme.iconSizeMedium
                icon.sourceSize.height: Theme.iconSizeMedium
                automaticCheck: false
                checked: grouped_light.rdata.on.on
                onCheckedChanged: busy = false
                function toggleGroup() {
                    busy = true
                    bridge.putResource(HueBridge.ResourceGroupedLight, { on: { on: !grouped_light.rdata.on.on } }, grouped_light.rid)
                }
                onClicked: {
                    if (appSettings.remorseSetting > 1 && grouped_light.rdata.on.on) {
                        Remorse.popupAction(page, group_owner.rdata.metadata.name + " " + qsTr("off"), function() { toggleGroup() }, appSettings.remorseTimeout*1000 )
                    }
                    else {
                        toggleGroup()
                    }
                }
            }

            Slider {
                id: groupSlider
                anchors.left: parent.left
                anchors.right: parent.right
                handleVisible: down || grouped_light.rdata.on.on
                minimumValue: 0
                maximumValue: 100
                stepSize: 1
                value: grouped_light ? grouped_light.rdata.dimming ? grouped_light.rdata.dimming.brightness : 0 : 0
                valueText: value.toFixed() + "%"
                onSliderValueChanged: {
                    if (down) {
                        bridge.setGroup(grouped_light.rid, { on: { on: true }, dimming: { brightness: sliderValue } })
                    }
                }
                label: qsTr("Brightness")
            }
//            Separator {
//                width: parent.width
//                color: Theme.primaryColor
//                horizontalAlignment: Qt.AlignHCenter
//            }

            SectionHeader {
                id: scenesHeader
                text: qsTr("Scenes")
            }
            ComboBox {
                id: sceneComboBox
                width: parent.width
                label: qsTr("Select scene")
                enabled: sceneRepeater.count > 0
                currentIndex: -1
                menu: ContextMenu {
                    Repeater {
                        id: sceneRepeater
                        model: bridge.resourceModel(HueBridge.ResourceScene)
                        delegate: MenuItem {
                            text: resource.metadata.name
                            visible: resource.group.rtype === group_owner.rtype && resource.group.rid === group_owner.rid
                            onClicked: {
                                console.log("Activate scene", resource.metadata.name, rid)
                                bridge.putResource(HueBridge.ResourceScene, { recall: { action: "active" } }, rid)
                            }
                            Component.onCompleted: {
                               if (resource.status.active !== "inactive" && visible) {
                                   console.log(resource.metadata.name, resource.status.active)
                                   sceneComboBox.currentIndex = index
                               }
                            }
                        }
                    }
                }
            }

            SectionHeader {
                id: ligtsHeader
                text: qsTr("Lights")
            }
            Repeater {
                model: group_owner.rdata.children
                delegate: LightListItem {
                    id: lightItem
                    width: parent.width
                    property string rid: group_owner.rdata.children[index].rid
                    property string rtype: group_owner.rdata.children[index].rtype
                    property ResourceObject device: null
                    Component.onCompleted: {
                        if (rtype === "device") {
                            device = bridge.resource(HueBridge.ResourceDevice, rid)
                            for (var i = 0; i < device.rdata.services.length || light == null; ++i) {
                                if (device.rdata.services[i].rtype === "light") {
                                    light = bridge.resource(HueBridge.ResourceLight, device.rdata.services[i].rid)
                                }
                            }
                        }
                        if (rtype === "light") {
                            light = bridge.resource(HueBridge.ResourceLight, rid)
                        }
                    }
                    onOnChanged: bridge.setLight(light.rid, { on: { on: on } })
                    onBrightnessChanged: bridge.setLight(light.rid, { on: { on: true }, dimming: { brightness: brightness } })
                }
            }
        }
    }
}
