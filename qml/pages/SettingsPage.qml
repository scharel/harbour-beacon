import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.beacon 1.0

Page {
    id: page

    SilicaFlickable {
        anchors.fill: parent

        Column {
            id: column

            anchors.fill: parent
            spacing: Theme.paddingMedium

            PullDownMenu {
                MenuItem {
                    text: qsTr("About")
                    onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
                }
            }

            PageHeader {
                id: pageHeader
                title: qsTr("Settings")
            }
            ButtonLayout {
                Button {
                    id: bridgesButton
                    icon.source: "../HueIconPack2019/devicesBridgesV2.svg"
                    icon.height: bridgesButton.height
                    icon.width: icon.height
                    icon.fillMode: Image.PreserveAspectFit
                    text: qsTr("Bridges")
                    onClicked: pageStack.push(Qt.resolvedUrl("BridgesPage.qml"))
                }
                Button {
                    id: devicesButton
                    icon.source: "../HueIconPack2019/settingsDevices.svg"
                    icon.height: devicesButton.height
                    icon.width: icon.height
                    icon.fillMode: Image.PreserveAspectFit
                    text: qsTr("Devices")
                    onClicked: pageStack.push(Qt.resolvedUrl("DevicesPage.qml"))
                }
            }
        }
    }
}
