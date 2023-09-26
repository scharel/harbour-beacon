import QtQuick 2.5
import Sailfish.Silica 1.0

Dialog {
    property string address

    Column {
        width: parent.width

        DialogHeader {
        }

        TextField {
            id: addressField
            width: parent.width
            placeholderText: "Enter the address of the Hue Bridge"
            label: "Bridge address"
        }
    }

    onDone: {
        if (result == DialogResult.Accepted) {
            address = addressField.text
        }
    }
}
