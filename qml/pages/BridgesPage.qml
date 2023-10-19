import QtQuick 2.5
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import harbour.beacon 1.0

Page {
    id: page

    property bool appStart: false
    property int hueMinVersion: 1948086000
    property int hueRelVersion: 1955082050
    //property int numAuthBridges: 0
    //signal setBridge()

    Timer {
        id: discoverTimer
        repeat: true
        interval: 5000
        property int count: 6
        triggeredOnStart: true
        onTriggered: {
            HueDiscovery.abortDiscovery()
            /*if (numAuthBridges === 1) {
                page.setBridge()
            }*/
            if (count > 0) {
                count--
                HueDiscovery.discover()
            }
            else {
                stop()
            }
        }
        onRunningChanged: {
            if (!running) {
                count = 6
            }
        }
    }

    Timer {
        id: connectTimer
        property int bridgeIndex
        interval: 2000
        onTriggered: {
            if (HueDiscovery.bridge(bridgeIndex).ready) {
                pageStack.replace(Qt.resolvedUrl("HomePage.qml"))
            }
            else {
                pageStack.push(Qt.resolvedUrl("ConnectBridgePage.qml"), { connectBridge: HueDiscovery.bridge(bridgeIndex) })
            }
        }
    }

    Timer {
        id: appStartTimer
        interval: 2000
        running: true
    }

    Connections {
        target: app

        onBridgeChanged: {
            if (bridge != null) {
                discoverTimer.stop()
            }
            else {
                discoverTimer.start()
            }
        }

    }

    Component.onCompleted: {
        //HueDiscovery.clearBridges()

        console.log(bridgeConfig.lastAddress)
        if (bridgeConfig.lastAddress) {
            HueDiscovery.addBridge(appSettings.lastUsedBridge, bridgeConfig.lastAddress)
        }
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
                text: qsTr("Clear list")
                visible: debug
                enabled: !discoverTimer.running && HueDiscovery.count > 0
                onClicked: {
                    discoverTimer.stop()
                    HueDiscovery.clearBridges()
                }
            }
            MenuItem {
                text: qsTr("Abort discovery")
                visible: discoverTimer.running
                onClicked: {
                    discoverTimer.stop()
                    HueDiscovery.abortDiscovery()
                }
            }
            MenuItem {
                text: qsTr("Discover bridges")
                visible: !discoverTimer.running
                onClicked: {
                    discoverTimer.start()
                }
            }
        }

        header: PageHeader {
            title: qsTr("Hue Bridges")
        }
        delegate: BackgroundItem {
            id: bridge
            height: Theme.itemSizeLarge


            Component.onCompleted: {
                HueDiscovery.bridge(index).getConfig()
                bridgeAuthIcon.visible = ready
                bridgeBusyIndicator.running = busy
            }

            Connections {
                target: HueDiscovery.bridge(index)
                onReadyChanged: {
                    console.log("Bridge ready", bridgeid, ready)
                    bridgeAuthIcon.visible = ready
                    if (bridgeid === appSettings.lastUsedBridge) {
                        app.bridge = HueDiscovery.bridge(0)
                        HueDiscovery.clearBridges([index])
                    }
                }
                onBusyChanged: {
                    bridgeBusyIndicator.running = busy
                }
            }

            ConfigurationGroup {
                id: thisBridgeConfig
                path: appSettings.path + "/bridges/" + bridgeid
                property string username
                property string lastAddress: address
                Component.onCompleted: {
                    //console.log(bridgeid, username)
                    if (username !== "") {
                        HueDiscovery.bridge(index).username = thisBridgeConfig.username
                        HueDiscovery.bridge(index).getResource()
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
                text: qsTr("Address") + ": " + (address ? address : thisBridgeConfig.lastAddress)
                color: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                truncationMode: TruncationMode.Elide
            }
            Icon {
                id: bridgeAuthIcon
                visible: false
                source: "image://theme/icon-m-link"
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
            }
            BusyIndicator {
                id: bridgeBusyIndicator
                running: false
                size: BusyIndicatorSize.Small
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
            }

            onClicked: {
                if (bridge.ready) {
                    app.bridge = HueDiscovery.bridge(0)
                    HueDiscovery.clearBridges([index])
                }
                else if (thisBridgeConfig.username !== "") {
                    HueDiscovery.bridge(index).getResource()
                    connectTimer.bridgeIndex = index
                    connectTimer.restart()
                }
                else {
                    pageStack.push(Qt.resolvedUrl("ConnectBridgePage.qml"), { connectBridge: HueDiscovery.bridge(index) })
                }
            }
        }

//        Column {
//            anchors.centerIn: parent
//            width: parent.width
//            visible: bridgeView.model.count === 0 && HueDiscovery.busy || appStart && appStartTimer.running
//            BusyIndicator {
//                anchors.centerIn: parent
//                size: BusyIndicatorSize.Large
//                running: parent.visible
//            }
//        }

        PageBusyIndicator {
            id: busyLabel
            running: (HueDiscovery.busy && bridgeView.model.count === 0) || (appStart && appStartTimer.running && bridgeView.model.count <= 2)
            //text: qsTr("Discovering Bridges")
            Icon {
                anchors.centerIn: parent
                source: "../hueiconpack/HueIconPack2019/devicesBridgesV2.svg"
                sourceSize.width: Theme.iconSizeLarge
                sourceSize.height: Theme.iconSizeLarge
            }
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
