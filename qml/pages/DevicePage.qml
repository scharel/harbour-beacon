import QtQuick 2.5
import Sailfish.Silica 1.0
import harbour.beacon 1.0

Dialog {
    id: page

    property ResourceObject device
    property string name: device.rdata.metadata.name
    property string archeType: device.rdata.metadata.archetype

    Column {
        width: parent.width

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

        Item {
            width: parent.width
            height: Theme.itemSizeMedium
        }
        Separator {
            width: parent.width
            color: Theme.secondaryColor
            horizontalAlignment: Qt.AlignHCenter
        }
        Item {
            width: parent.width
            height: Theme.itemSizeMedium
        }

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

    onDone: {
        if (result == DialogResult.Accepted) {
            name = nameField.text
            archeType = archeIcon.archeType
        }
    }
}
