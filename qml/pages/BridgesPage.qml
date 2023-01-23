import QtQuick 2.2
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import harbour.beacon 1.0

Page {
    id: page

    property int hueMinVersion: 1948086000
    property int numAuthBridges: 0
    signal setBridge()

    Timer {
        id: discoverTimer
        repeat: true
        interval: 1000
        property int count: 10
        triggeredOnStart: true
        onTriggered: {
            HueDiscovery.abortDiscovery()
            if (numAuthBridges === 1) {
                page.setBridge()
            }
            if (count > 0) {
                count--
                HueDiscovery.discover()
            }
            else {
                stop()
                count = 10
            }
        }
    }

    Timer {
        id: getResourcesTimer
        interval: 100
        repeat: true
        triggeredOnStart: true
        onTriggered: {
            if (bridge != null) {
                if (bridge.ready) {
                    stop()
                    pageStack.replace(Qt.resolvedUrl("HomePage.qml"))
                }
            }
        }
    }

    Connections {
        target: app

        onBridgeChanged: {
            if (bridge != null) {
                discoverTimer.stop()
                bridgeConfig.path = appSettings.path + "/bridges/" + bridge.bridgeid
                bridgeConfig.sync()
                bridge.username = bridgeConfig.username
                getResourcesTimer.start()
                bridge.getResource(HueBridge.ResourceAll)
                bridge.startEventStream()
            }
            else {
                discoverTimer.start()
            }
        }

    }

    Component.onCompleted: {
        discoverTimer.start()
        placeHolder.started = true
    }

    SilicaListView {
        id: bridgeView
        model: HueDiscovery
        anchors.fill: parent

        PullDownMenu {
            busy: HueDiscovery.busy
            /*MenuItem {
                text: qsTr("Add a bridge by address")
                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("AddBridgeDialog.qml"))
                    dialog.accepted.connect(function() {
                        console.log(dialog.address)
                    })
                }
            }*/
            MenuItem {
                text: discoverTimer.running ? qsTr("Abort discovery") : qsTr("Discover bridges")
                onClicked: {
                    if (discoverTimer.running) {
                        discoverTimer.stop()
                    }
                    else {
                        discoverTimer.start()
                    }
                }
            }
        }

        header: PageHeader {
            title: qsTr("Hue Bridges")
        }
        delegate: BackgroundItem {
            id: delegate
            height: Theme.itemSizeLarge

            Component.onCompleted: {
                HueDiscovery.bridge(index).getConfig()
            }

            Connections {
                target: page
                onSetBridge: bridge = HueDiscovery.bridge(index)
            }

            ConfigurationGroup {
                id: thisBridgeConfig
                path: appSettings.path + "/bridges/" + bridgeid
                property string username
                Component.onCompleted: {
                    HueDiscovery.bridge(index).username = username
                    if (username !== "") {
                        numAuthBridges++
                    }
                }
            }

            Label {
                id: idLabel
                x: Theme.horizontalPageMargin
                anchors.bottom: parent.verticalCenter
                text: bridgeid
                font.bold: true
                font.family: Theme.fontFamilyHeading
            }
            Label {
                id: nameLabel
                anchors.left: idLabel.right
                anchors.leftMargin: Theme.paddingMedium
                anchors.right: bridgeBusyIndicator.left
                anchors.bottom: parent.verticalCenter
                text: name
                truncationMode: TruncationMode.Elide
            }
            Label {
                id: addressLabel
                anchors.left: idLabel.left
                anchors.right: bridgeBusyIndicator.left
                anchors.top: parent.verticalCenter
                text: "IP: " + address
                color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                truncationMode: TruncationMode.Elide
            }
            Icon {
                id: bridgeAuthIcon
                visible: username !== ""
                source: "image://theme/icon-m-link"
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
            }
            BusyIndicator {
                id: bridgeBusyIndicator
                running: busy
                size: BusyIndicatorSize.Medium
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
            }

            onClicked: {
                if (username !== "") {
                    bridge = HueDiscovery.bridge(index)
                    HueDiscovery.clearBridges([index])
                    pageStack.pop()
                }
                else {
                    pageStack.push(Qt.resolvedUrl("ConnectBridgePage.qml"), { connectBridge: HueDiscovery.bridge(index) })
                }
            }
        }

        BusyLabel {
            id: busyLabel
            running: bridgeView.model.count === 0 && HueDiscovery.busy
            text: qsTr("Searching Bridges")
        }
        ViewPlaceholder {
            id: placeHolder
            property bool started: false
            enabled: started && bridgeView.model.count === 0 && !HueDiscovery.busy

            text: qsTr("No Hue Bridges found")
            hintText: qsTr("Pull down to add bridges")
        }

        VerticalScrollDecorator {}
    }
}
