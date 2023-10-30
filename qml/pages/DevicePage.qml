import QtQuick 2.5
import Sailfish.Silica 1.0
import harbour.beacon 1.0

Dialog {
    id: page

    property ResourceObject device
    property string name: device.rdata.metadata.name
    property string archeType: device.rdata.metadata.archetype

    Column {
        anchors.fill: parent
        spacing: Theme.paddingLarge

        DialogHeader {}

        Row {
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            BackgroundItem {
                width: Theme.iconSizeLarge
                height: Theme.iconSizeLarge
                Icon {
                    id: archeIcon
                    property string archeType: page.archeType
                    source: "../hueiconpack/HueIconPack2019/" + deviceArchetypeImages[archeType]
                    sourceSize.width: Theme.iconSizeLarge
                    sourceSize.height: Theme.iconSizeLarge
                    //source: "../hueiconpack/ApiV2Archetype/" + archeType + ".svg"
                }
                onClicked: Dialog
            }
            TextField {
                id: nameField
                width: parent.width - archeIcon.width
                font.pixelSize: Theme.fontSizeLarge
                text: name
                label: qsTr("Name")
            }
        }

        Separator {
            width: parent.width
            color: Theme.secondaryColor
            horizontalAlignment: Qt.AlignHCenter
        }

        Column {
            id: detailsColumn
            width: parent.width
            DetailItem {
                label: qsTr("Product")
                value: device.rdata.product_data.product_name
            }
            DetailItem {
                label: qsTr("Manufacturer")
                value: device.rdata.product_data.manufacturer_name
            }
            DetailItem {
                label: qsTr("Model")
                value: device.rdata.product_data.model_id
            }
            DetailItem {
                label: qsTr("Hardwareplatform")
                value: device.rdata.product_data.hardware_platform_type
            }
            DetailItem {
                label: qsTr("Software")
                value: device.rdata.product_data.software_version
            }
        }

//        SilicaFlickable {
//            visible: debug
//            width: parent.width
//            anchors.top: detailsColumn.bottom
//            height: Theme.itemSizeExtraLarge
//            contentHeight: rawText.height
//            TextArea {
//                id: rawText
//                readOnly: true
//                label: qsTr("Raw data")
//                font.pixelSize: Theme.fontSizeSmall
//                text: JSON.stringify(device.rdata)
//            }
//        }
    }

    onDone: {
        if (result == DialogResult.Accepted) {
            name = nameField.text
            archeType = archeIcon.archeType
        }
    }
}
