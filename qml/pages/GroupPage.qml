import QtQuick 2.2
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
            spacing: Theme.paddingLarge
            PageHeader {
                id: pageHeader

                Switch {
                    id: groupSwitch
                    anchors.verticalCenter: parent.verticalCenter
                    //anchors.left: pageHeader.extraContent.left
                    anchors.right: groupIcon.left
                    //icon.source: "../HueIconPack2019/" + archetypeImages[group_owner.rdata.metadata.archetype]
                    automaticCheck: false
                    checked: grouped_light.rdata.on.on
                    onCheckedChanged: busy = false
                    onClicked: {
                        busy = true
                        bridge.putResource(HueBridge.ResourceGroupedLight, { on: { on: !grouped_light.rdata.on.on } }, grouped_light.id)
                    }
                }
                Icon {
                    id: groupIcon
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: pageHeader.extraContent.right
                    anchors.rightMargin: Theme.paddingSmall
                    width: Theme.iconSizeSmall
                    height: Theme.iconSizeSmall
                    source: "../HueIconPack2019/" + roomArchetypeImages[group_owner.rdata.metadata.archetype]
                }
                title: group_owner.rdata.metadata.name
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
            }
            Separator {
                width: parent.width
                color: Theme.primaryColor
                horizontalAlignment: Qt.AlignHCenter
            }

            SectionHeader {
                id: scenesHeader
                text: qsTr("Scenes")
            }
            ComboBox {
                id: sceneComboBox
                width: parent.width
                label: qsTr("Scene")
                currentIndex: -1
                menu: ContextMenu {
                    Repeater {
                        model: bridge.resourceModel(HueBridge.ResourceScene)
                        delegate: MenuItem {
                            text: resource.metadata.name
                            visible: resource.group.rtype === group_owner.rtype && resource.group.rid === group_owner.rid
                            onClicked: bridge.putResource(HueBridge.ResourceScene, { target: { rid: group_owner.rid }, action: { on: { on: true } } }, resource.rid)
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
                delegate: Label {
                    property string rid: group_owner.rdata.children[index].rid
                    property string rtype: group_owner.rdata.children[index].rtype
                    property ResourceObject device: bridge.resource(HueBridge.ResourceDevice, rid)
                    x: Theme.horizontalPageMargin
                    width: page.width - 2*x
                    visible: rtype === "device"
                    color: Theme.secondaryColor
                    text: device.rdata.metadata.name
                }
            }
        }
    }
}
