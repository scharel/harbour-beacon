import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.beacon 1.0

CoverBackground {
    CoverPlaceholder {
        text: "Beacon"
        //icon.source: "../HueIconPack2019/bulbsClassic.svg"
    }

    /*
    CoverActionList {
        id: coverAction
        //enabled: home !== null

        CoverAction {
            iconSource: "../HueIconPack2019/uicontrolsSwitchOn.svg"
            onTriggered: bridge.putResource(HueBridge.ResourceGroupedLight, { on: { on: !home.data.on.on } })

        }
    }
    */
}
