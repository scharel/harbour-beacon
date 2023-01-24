import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.beacon 1.0

Page {
    id: page

    property ResourceObject bridgeHome: bridge.resource(HueBridge.ResourceBridgeHome)
    property ResourceObject homeGroup: bridge.resource(HueBridge.ResourceGroupedLight, bridgeHome.rdata.services[0].rid)
    property ResourceModel groupModel: bridge.resourceModel(HueBridge.ResourceGroupedLight)

    onGroupModelChanged: {
        groupModel.dynamicSortFilter = false
        groupModel.sortRole = ResourceModel.RidRole
        groupModel.resourceOrder = appSettings.groupOrder
        groupModel.favoritePath = ["owner", "rtype"]
        groupModel.favoriteRegExp = /^room$/ //appSettings.groupSorting
        groupModel.filterPath = ["owner", "rtype"]
        groupModel.filterRegExp = /^(room|zone)$/
        groupModel.sectionPath = ["owner", "rtype"]
        groupView.model = groupModel
        console.log("Bridge changed", bridge.bridgeid)
    }

    Connections {
        target: groupModel
        onResourceOrderChanged: {
            console.log(order)
            appSettings.groupOrder = order
        }
    }

    SilicaListView {
        id: groupView
        anchors.fill: parent

        //spacing: Theme.paddingMedium

        PullDownMenu {
            busy: bridge == null ? false : bridge.busy
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
        }

        header: PageHeader {
            title: qsTr("Home")
            Switch {
                id: homeSwitch
                anchors.verticalCenter: parent.verticalCenter
                visible: bridge != null
                enabled: homeGroup != null
                automaticCheck: false
                checked: homeGroup != null && typeof(homeGroup.data) !== "undefined" && typeof(homeGroup.data.on) !== "undefined" ? homeGroup.data.on.on : false
                onCheckedChanged: busy = false
                onClicked: {
                    busy = true
                    bridge.setGroup(homeGroup.id, { on: { on: !homeGroup.data.on.on } })
                }
            }
        }

        delegate: ListItem {
            id: listItem
            contentHeight: Theme.itemSizeMedium
            width: parent.width
            //visible: resource.owner.rtype !== "bridge_home"

            property var groupId: rid
            property var ownerId: resource.owner.rid
            property var groupOwner: null

            onGroupIdChanged: {
                switch (resource.owner.rtype) {
                case "room":
                    groupOwner = bridge.resource(HueBridge.ResourceRoom, resource.owner.rid)
                    break
                case "zone":
                    groupOwner = bridge.resource(HueBridge.ResourceZone, resource.owner.rid)
                    break
                }
                if (groupOwner !== null) {
                    groupIcon.source = "../HueIconPack2019/" + roomArchetypeImages[groupOwner.rdata.metadata.archetype]
                    groupLabel.text = groupOwner.rdata.metadata.name
                    groupIdLabel.text = rid
                    ownerId = resource.owner.rid
                    //console.log(rid, groupOwner.rdata.metadata.name)
                }
            }

            //Component.onCompleted: getResourceObject()
            //onRidChanged: getResourceObject()

            onClicked: {
                pageStack.push(Qt.resolvedUrl("GroupPage.qml"),  { grouped_light: bridge.resource(HueBridge.ResourceGroupedLight, rid), group_owner: groupOwner } )
            }
            onPressAndHold: {
                if (typeof(resource.dimming) !== "undefined") {
                    openMenu()
                }
            }

            Switch {
                id: onSwitch
                anchors.verticalCenter: parent.verticalCenter
                //x: Theme.horizontalPageMargin
                //width: Theme.iconSizeSmall
                automaticCheck: false
                checked: resource.on.on
                onCheckedChanged: busy = false
                onClicked: {
                    busy = true
                    bridge.setGroup(rid, { on: { on: !resource.on.on } })
                }
            }
            Icon {
                id: groupIcon
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: onSwitch.right
                width: Theme.iconSizeSmall
                height: Theme.iconSizeSmall
            }
            Label {
                id: groupLabel
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: groupIcon.right
                anchors.leftMargin: Theme.paddingSmall
                anchors.right: parent.right
                truncationMode: TruncationMode.Fade
            }
            Label {
                id: groupIdLabel
                visible: debug
                font.pixelSize: Theme.fontSizeTiny
                color: down ? Theme.secondaryHighlightColor : Theme.secondaryColor
                anchors.top: parent.top
                anchors.left: onSwitch.right
                anchors.right: parent.right
                truncationMode: TruncationMode.Fade
            }
            Slider {
                width: parent.width
                anchors.verticalCenter: parent.bottom
                anchors.verticalCenterOffset: -Theme.paddingMedium
                visible: resource ? resource.dimming ? true : false : false
                opacity: menuOpen ? 0.0 : 1.0
                Behavior on opacity {
                    FadeAnimation {}
                }
                enabled: false
                handleVisible: false
                minimumValue: 0
                maximumValue: 100
                stepSize: 1
                value: resource ? resource.dimming ? resource.dimming.brightness : 0 : 0
            }

            menu: ContextMenu {
                Slider {
                    width: parent.width
                    handleVisible: down || resource.on.on
                    minimumValue: 0
                    maximumValue: 100
                    stepSize: 1
                    value: resource ? resource.dimming ? resource.dimming.brightness : 0 : 0
                    valueText: value.toFixed() + "%"
                    onSliderValueChanged: {
                        if (down) {
                            bridge.setGroup(rid, { on: { on: true }, dimming: { brightness: sliderValue } })
                        }
                    }
                }
                //Row {
                    //width: parent.width
                    MenuItem {
                        //width: parent.width / 2
                        text: qsTr("Move up")
                        enabled: index > 0
                        onClicked: groupModel.moveUp(index) //console.log("Move", rid, "up")
                    }
                    MenuItem {
                        //width: parent.width / 2
                        text: qsTr("Move down")
                        enabled: index < (groupView.count - 1)
                        onClicked: groupModel.moveDown(index) //console.log("Move", rid, "down")
                    }
                //}

                /*ComboBox {
                    width: parent.width
                    label: qsTr("Scene")
                    anchors.horizontalCenter: parent.horizontalCenter
                    currentIndex: -1
                    menu: ContextMenu {
                        Repeater {
                            model: bridge.resourceModel(HueBridge.ResourceScene)
                            delegate: MenuItem {
                                text: resource.metadata.name
                                visible: resource.group.rid ===  listItem.ownerId
                                onClicked: bridge.putResource(HueBridge.ResourceScene, { target: { rid: listItem.groupId }, action: { on: { on: true } } }, rid)
                            }
                        }
                    }
                }*/
            }
        }

        BusyLabel {
            id: busyLabel
            running: bridge == null
            text: bridge == null ? qsTr("Searching Bridge") : !bridge.ready ? qsTr("Getting Data") : ""
        }
        ViewPlaceholder {
            id: noGroupsPlaceholder
            text: qsTr("No light groups defined")
            //hintText: "Pull down to configure"
            enabled: !busyLabel.running && bridge != null && groupView.count === 0
        }

        section.property: "section"
        section.delegate: SectionHeader {
            text: {
                switch(section) {
                case "room":
                    qsTr("Rooms")
                    break;
                case "zone":
                    qsTr("Zones")
                    break;
                default:
                    //section
                    break;
                }
            }
        }
    }
}
