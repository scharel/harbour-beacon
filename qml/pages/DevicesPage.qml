import QtQuick 2.5
import Sailfish.Silica 1.0
import harbour.beacon 1.0

Page {
    id: page

    property ResourceModel deviceModel: bridge.resourceModel(HueBridge.ResourceDevice)

    Connections {
        target: app
        onBridgeChanged: {
            if (bridge != null && typeof(bridge.bridgeid) !== "undefined") {
                deviceModel = bridge.resourceModel(HueBridge.ResourceDevice)
                deviceView.model = deviceModel
                console.log("Bridge changed", bridge.bridgeid)
            }
            else {
                deviceView.model = 0
                home = null
                console.log("No bridge set")
            }
        }
    }

    SilicaListView {
        id: deviceView
        anchors.fill: parent

        model: deviceModel

        header: PageHeader {
            title: qsTr("Devices")
        }

        delegate: ListItem {
            id: listItem
            contentHeight: Theme.itemSizeMedium
            width: parent.width

            Icon {
                id: deviceIcon
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: Theme.horizontalPageMargin
                sourceSize.width: Theme.iconSizeMedium
                sourceSize.height: Theme.iconSizeMedium
                source: resource.product_data.product_archetype !== "unknown_archetype" ?  "../hueiconpack/HueIconPack2019/" + deviceArchetypeImages[resource.product_data.product_archetype] : "image://theme/icon-m-question"
                //source: "../hueiconpack/ApiV2Archetype/" + resource.product_data.product_archetype + ".svg"
            }
            Label {
                id: nameLabel
                anchors.bottom: parent.verticalCenter
                anchors.left: deviceIcon.right
                anchors.leftMargin: Theme.paddingSmall
                anchors.right: parent.right
                truncationMode: TruncationMode.Fade
                text: resource.metadata.name
            }
            Label {
                id: productLabel
                anchors.top: parent.verticalCenter
                anchors.left: deviceIcon.right
                anchors.leftMargin: Theme.paddingSmall
                anchors.right: parent.right
                truncationMode: TruncationMode.Fade
                font.pixelSize: Theme.fontSizeSmall
                color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                text: resource.product_data.product_name
            }
            Label {
                anchors.bottom: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                font.pixelSize: Theme.fontSizeExtraSmall
                color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                text: qsTr("Version")
            }
            Label {
                id: versionLabel
                anchors.top: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                font.pixelSize: Theme.fontSizeSmall
                color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                text: resource.product_data.software_version
            }

            RemorseItem { id: remorse }

            function showRemorseItem() {
                var idx = index
                remorse.execute(listItem, qsTr("Deleting") + " " + resource.metadata.name, function() { bridge.delResource(HueBridge.ResourceDevice, rid) } )
            }

            onClicked: {
                bridge.putResource(HueBridge.ResourceDevice, { identify: { action: "identify" } }, rid)
                var dialog = pageStack.push(Qt.resolvedUrl("DevicePage.qml"), { device: bridge.resource(HueBridge.ResourceDevice, rid) } )
                dialog.accepted.connect(function() {
                    console.log("Changing device", dialog.name, dialog.archeType)
                    bridge.putResource(HueBridge.ResourceDevice, { metadata: { name: dialog.name, archetype: dialog.archeType } }, rid)
                })
            }
            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Delete")
                    enabled: !/^bridge/.test(resource.product_data.product_archetype)
                    onClicked: showRemorseItem()
                }
            }
        }

        BusyLabel {
            id: busyLabel
            running: bridge == null
            text: bridge == null ? qsTr("Searching Bridge") : !bridge.ready ? qsTr("Getting Data") : ""
        }
        ViewPlaceholder {
            id: noDeviicesPlaceholder
            text: qsTr("No devices found")
            enabled: !busyLabel.running && bridge != null && deviceView.count === 0
        }
    }
}
