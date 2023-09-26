import QtQuick 2.5
import Sailfish.Silica 1.0
import harbour.beacon 1.0

Page {
    id: page

    property var grouped_light
    property var group_owner

    SilicaFlickable {
        anchors.fill: parent

        Column {
            id: column

            anchors.fill: parent
            //spacing: Theme.paddingLarge
            PageHeader {
                id: pageHeader

//                Switch {
//                    id: groupSwitch
//                    anchors.verticalCenter: parent.verticalCenter
//                    //anchors.left: pageHeader.extraContent.left
//                    anchors.right: groupIcon.left
//                    //icon.source: "../hueiconpack/HueIconPack2019/" + archetypeImages[group_owner.rdata.metadata.archetype]
//                    automaticCheck: false
//                    checked: grouped_light.rdata.on.on
//                    onCheckedChanged: busy = false
//                    onClicked: {
//                        busy = true
//                        bridge.putResource(HueBridge.ResourceGroupedLight, { on: { on: !grouped_light.rdata.on.on } }, grouped_light.rid)
//                    }
//                }
                Icon {
                    id: groupIcon
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: pageHeader.extraContent.right
                    anchors.rightMargin: Theme.paddingSmall
                    sourceSize.width: Theme.iconSizeSmall
                    sourceSize.height: Theme.iconSizeSmall
                    source: "../hueiconpack/HueIconPack2019/" + roomArchetypeImages[group_owner.rdata.metadata.archetype]
                    //source: "../hueiconpack/ApiV2Archetype/" + group_owner.rdata.metadata.archetype + ".svg"
                }
                title: group_owner.rdata.metadata.name
            }

            Switch {
                id: groupIconSwitch
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                //text: checked ? qsTr("Turn off") : qsTr("Turn off")
                //icon.source: "image://theme/icon-m-light-contrast"
                icon.source: "../hueiconpack/HueIconPack2019/bulbGroup.svg"
                icon.sourceSize.width: Theme.iconSizeMedium
                icon.sourceSize.height: Theme.iconSizeMedium
                automaticCheck: false
                checked: grouped_light.rdata.on.on
                onCheckedChanged: busy = false
                onClicked: {
                    busy = true
                    bridge.putResource(HueBridge.ResourceGroupedLight, { on: { on: !grouped_light.rdata.on.on } }, grouped_light.rid)
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
                currentIndex: -1
                menu: ContextMenu {
                    Repeater {
                        model: bridge.resourceModel(HueBridge.ResourceScene)
                        delegate: MenuItem {
                            text: resource.metadata.name
                            visible: resource.group.rtype === group_owner.rtype && resource.group.rid === group_owner.rid
                            onClicked: {
                                console.log("Activate scene", resource.metadata.name, rid)
                                bridge.putResource(HueBridge.ResourceScene, { recall: { action: "active" } }, rid)
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
                delegate: BackgroundItem {
                    id: ownerItem
                    width: page.width
                    property string rid: group_owner.rdata.children[index].rid
                    property string rtype: group_owner.rdata.children[index].rtype
                    property ResourceObject device: null
                    property ResourceObject light: null
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
                            device = bridge.resource(HueBridge.ResourceDevice, light.rdata.owner.rid)
                        }
                    }
                    IconTextSwitch {
                        enabled: light != null
                        automaticCheck: false
                        checked: light != null ? light.rdata.on.on : false
                        onCheckedChanged: busy = false
                        onClicked: {
                            busy = true
                            bridge.setLight(light.rid, { on: { on: !light.rdata.on.on } })
                        }
                        text: light != null ? light.rdata.metadata.name : ""
                        icon.source: "../hueiconpack/ApiV2Archetype/" + light.rdata.metadata.archetype + ".svg"
                        icon.sourceSize.width: Theme.iconSizeSmallPlus
                        icon.sourceSize.height: Theme.iconSizeSmallPlus
                    }

//                    Row {
//                        Switch {
//                            id: onSwitch
//                            anchors.verticalCenter: parent.verticalCenter
//                            enabled: light != null
//                            automaticCheck: false
//                            checked: light != null ? light.rdata.on.on : false
//                            onCheckedChanged: busy = false
//                            onClicked: {
//                                busy = true
//                                bridge.setLight(light.rid, { on: { on: !light.rdata.on.on } })
//                            }
//                        }
//                        Label {
//                            anchors.verticalCenter: parent.verticalCenter
//                            color: Theme.secondaryColor
//                            text: light != null ? light.rdata.metadata.name : ""
//                        }
//                    }
                    onClicked: pageStack.push(Qt.resolvedUrl("LightPage.qml"), { device: device, light: light } )
                }
            }
        }
    }
}
