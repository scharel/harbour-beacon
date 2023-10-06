import QtQuick 2.5
import Sailfish.Silica 1.0
import harbour.beacon 1.0

CoverBackground {
//    property ResourceModel groupModel: bridge.resourceModel(HueBridge.ResourceGroupedLight)
//    property ResourceObject coverGroup: null
//    Component.onCompleted: {
//        if (groupModel.rowCount() > 0) {
//            var resources = groupModel.data(0, resources)
//            switch (resources.owner.rtype) {
//            case "room":
//                coverGroup = bridge.resource(HueBridge.ResourceRoom, resource.owner.rid)
//                break
//            case "zone":
//                coverGroup = bridge.resource(HueBridge.ResourceZone, resource.owner.rid)
//                break
//            }
//        }
//    }

    CoverPlaceholder {
        text: "Beacon"
        icon.source: "../hueiconpack/HueIconPack2019/bulbsClassic.svg?" + Theme.col
        icon.sourceSize.width: Theme.iconSizeExtraLarge
        icon.sourceSize.height: Theme.iconSizeExtraLarge
    }

//    CoverActionList {
//        id: coverAction
//        enabled: bridge !== null && groupModel.rowCount()

//        CoverAction {
//            iconSource: "image://theme/icon-cover-subview"
//            //onTriggered: bridge.putResource(HueBridge.ResourceGroupedLight, { on: { on: !home.data.on.on } })

//        }
//    }
}
