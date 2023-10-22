import QtQuick 2.5
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
                    icon.source: "../hueiconpack/HueIconPack2019/devicesBridgesV2.svg"
                    icon.sourceSize.height: bridgesButton.height
                    icon.sourceSize.width: icon.height
                    icon.fillMode: Image.PreserveAspectFit
                    text: qsTr("Bridges")
                    onClicked: pageStack.push(Qt.resolvedUrl("BridgesPage.qml"))
                }
                Button {
                    id: devicesButton
                    icon.source: "../hueiconpack/HueIconPack2019/settingsDevices.svg"
                    icon.sourceSize.height: devicesButton.height
                    icon.sourceSize.width: icon.height
                    icon.fillMode: Image.PreserveAspectFit
                    text: qsTr("Devices")
                    onClicked: pageStack.push(Qt.resolvedUrl("DevicesPage.qml"))
                }
            }
            SectionHeader {
                text: qsTr("Remorse")
            }
            ComboBox {
                label: qsTr("Turn off remorse")
                description: qsTr("Show a remorse timer when turning off devices")
                currentIndex: appSettings.remorseSetting
                onCurrentIndexChanged: appSettings.remorseSetting = currentIndex

                menu: ContextMenu {
                    MenuItem { text: qsTr("None") }                     // == 0
                    MenuItem { text: qsTr("Home only") }                // > 0
                    MenuItem { text: qsTr("Home and groups") }          // > 1
                    MenuItem { text: qsTr("All groups and devices") }   // > 2
                }
            }
            Slider {
                width: parent.width
                label: qsTr("Remorse timeout")
                minimumValue: 1
                maximumValue: 10
                stepSize: 1
                value: appSettings.remorseTimeout
                valueText: value + "s"
                onValueChanged: appSettings.remorseTimeout = value
            }
        }
    }
}
