import QtQuick 2.5
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import harbour.beacon 1.0

Page {
    id: page

    ConfigurationValue {
        id: timeoutConfig
        key: appSettings.path + "/remorseTimeout"
        defaultValue: [4, 2, 2]
    }

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
                id: remorseComboBox
                property var remorseGroups: [
                    qsTr("Home only"),              // > 0
                    qsTr("Home and groups"),        // > 1
                    qsTr("All groups and devices")  // > 2
                ]
                label: qsTr("Power off remorse")
                description: qsTr("Show a remorse timer when powering off devices")
                currentIndex: appSettings.remorseSetting
                onCurrentIndexChanged: appSettings.remorseSetting = currentIndex

                menu: ContextMenu {
                    MenuItem { text: qsTr("None") }
                    Repeater {
                        model: remorseComboBox.remorseGroups
                        delegate: MenuItem { text: modelData }
                    }
                }
            }
            Repeater {
                id: remorseSliders
                property var remorseSliderNames: [
                    qsTr("Home"),
                    qsTr("Groups"),
                    qsTr("Devices")
                ]
                model: remorseSliderNames
                delegate: Slider {
                    width: parent.width
                    opacity: index < appSettings.remorseSetting ? 1.0 : 0.0
                    Behavior on opacity { FadeAnimator {} }
                    label: qsTr("%1 timeout").arg(modelData)
                    minimumValue: 1
                    maximumValue: 10
                    stepSize: 1
                    Component.onCompleted: {
                        value = timeoutConfig.value[index]
                    }
                    valueText: value + "s"
                    onValueChanged: {
                        var timeouts = timeoutConfig.value
                        timeouts[index] = value.toFixed()
                        timeoutConfig.value = timeouts
                    }
                }
            }
        }
    }
}
