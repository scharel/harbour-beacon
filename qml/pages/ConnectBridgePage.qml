import QtQuick 2.2
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Page {
    id: page

    property var connectBridge

    Timer {
        id: pollTimer
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: connectBridge.createUser()
    }

    Connections {
        target: connectBridge
        onUsernameChanged: {
            pollTimer.stop()
            console.log(username)
            bridgeConfig.username = username
            bridgeConfig.sync()
            pageStack.pop()
        }
    }

    ConfigurationGroup {
        id: bridgeConfig
        path: appSettings.path + "/bridges/" + connectBridge.bridgeid

        property string username: connectBridge.username
    }

    SilicaFlickable {
        anchors.fill: parent

        Column {
            id: column

            anchors.fill: parent
            spacing: Theme.paddingLarge * 2
            PageHeader {
                title: qsTr("Connect Bridge")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                text: qsTr("Press the link button on your Hue Bridge")
                font.pixelSize: Theme.fontSizeExtraLarge
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Rectangle {
                height: Theme.itemSizeHuge
                width: parent.width
                color: "transparent"

                PageBusyIndicator {
                    id: busyIndicator
                    anchors.centerIn: parent
                    size: BusyIndicatorSize.Large
                    running: true
                }
                Icon {
                    anchors.centerIn: parent
                    source: busyIndicator.running ? "image://theme/icon-l-transfer" : "image://theme/icon-l-check"
                    //source: Qt.resolvedUrl("../../icons/HueIconPack2019/devicesBridgesV2.svg")
                    color: busyIndicator.running ? palette.secondaryColor : "green"
                }
            }
        }
    }
}
