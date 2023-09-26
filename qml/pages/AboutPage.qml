import QtQuick 2.5
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaFlickable {
        anchors.fill: parent

        Column {
            id: column

            anchors.fill: parent

            PageHeader {
                id: pageHeader
                title: qsTr("About")
            }
        }
    }
}
