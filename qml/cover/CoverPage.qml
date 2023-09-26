import QtQuick 2.5
import Sailfish.Silica 1.0
import harbour.beacon 1.0

CoverBackground {
    CoverPlaceholder {
        text: "Beacon"
        //icon.source: "../hueiconpack/HueIconPack2019/bulbsClassic.svg"
    }

    /*
    CoverActionList {
        id: coverAction
        //enabled: home !== null

        CoverAction {
            iconSource: "../hueiconpack/HueIconPack2019/uicontrolsSwitchOn.svg"
            onTriggered: bridge.putResource(HueBridge.ResourceGroupedLight, { on: { on: !home.data.on.on } })

        }
    }
    */
}
