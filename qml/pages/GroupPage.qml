import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.beacon 1.0

Page {
    id: page

    property var grouped_light_id

    property var grouped_light
    property var group

    Component.onCompleted: {
        grouped_light = bridge.resource(HueBridge.ResourceGroupedLight, grouped_light_id)
        switch (grouped_light.data.owner.rtype) {
        case "room":
            //bridge.getResource(HueBridge.ResourceRoom, grouped_light.owner.rid)
            group = bridge.resource(HueBridge.ResourceRoom, grouped_light.data.owner.rid)
            break
        case "zone":
            //bridge.getResource(HueBridge.ResourceZone, grouped_light.owner.rid)
            group = bridge.resource(HueBridge.ResourceZone, grouped_light.data.owner.rid)
            break;
        }
    }

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
                    //icon.source: "../HueIconPack2019/" + archetypeImages[group.data.metadata.archetype]
                    automaticCheck: false
                    checked: grouped_light.data.on.on
                    onCheckedChanged: busy = false
                    onClicked: {
                        busy = true
                        bridge.putResource(HueBridge.ResourceGroupedLight, { on: { on: !grouped_light.data.on.on } }, grouped_light.id)
                    }
                }
                Icon {
                    id: groupIcon
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: pageHeader.extraContent.right
                    anchors.rightMargin: Theme.paddingSmall
                    width: Theme.iconSizeSmall
                    height: Theme.iconSizeSmall
                    source: "../HueIconPack2019/" + archetypeImages[group.data.metadata.archetype]
                }
                title: group.data.metadata.name
            }


            Slider {
                id: groupSlider
                anchors.left: parent.left
                anchors.right: parent.right
                handleVisible: down || grouped_light.data.on.on
                minimumValue: 0
                maximumValue: 100
                stepSize: 1
                value: grouped_light ? grouped_light.data.dimming ? grouped_light.data.dimming.brightness : 0 : 0
                valueText: value.toFixed() + "%"
                onSliderValueChanged: {
                    if (down) {
                        bridge.setGroup(grouped_light.id, { on: { on: true }, dimming: { brightness: sliderValue } })
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
                width: parent.width
                label: qsTr("Scene")
                currentIndex: -1
                menu: ContextMenu {
                    Repeater {
                        model: bridge.resourceModel(HueBridge.ResourceScene)
                        delegate: MenuItem {
                            text: resource.metadata.name
                            visible: resource.group.rid === group.id
                            onClicked: bridge.putResource(HueBridge.ResourceScene, { target: { rid: group.id }, action: { on: { on: true } } }, resource.id)
                        }
                    }
                }
            }

            SectionHeader {
                id: ligtsHeader
                text: qsTr("Lights")
            }
        }
    }
}
