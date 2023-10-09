import QtQuick 2.5
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import harbour.beacon 1.0
import "pages"

ApplicationWindow {
    id: app
    initialPage: Component { BridgesPage { appStart: true } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    property HueBridge bridge: null
    onBridgeChanged: {
        if (bridge != null) {
            bridgeConfig.path = appSettings.path + "/bridges/" + bridge.bridgeid
            bridgeConfig.sync()
            bridge.username = bridgeConfig.username
            bridge.startEventStream()
            appSettings.lastUsedBridge = bridge.bridgeid
            pageStack.replaceAbove(null, Qt.resolvedUrl("pages/HomePage.qml"))
        }
        else {
            bridgeConfig.path = ""
            bridgeConfig.clear()
        }
    }

    Connections {
        target: Qt.application
        onStateChanged: {
            if (bridge != null) {
                if (state === Qt.ApplicationActive) {
                    bridge.getResource()
                    bridge.startEventStream()
                }
                if (state === Qt.ApplicationInactive) {
                    bridge.stopEventStream()
                }
            }
        }
    }

    ConfigurationGroup {
        id: appSettings
        path: "/apps/harbour-beacon"
        property string lastUsedBridge: value("lastUsedBridge", "", String)
        property var groupSorting: value("groupSorting", /^room$/, RegExp)
        property var groupOrder: value("groupOrder", Array(), Array)
    }

    ConfigurationGroup {
        id: bridgeConfig
        property string username
    }

    property var roomArchetypeImages: {
        "living_room": "roomsLiving.svg",
        "kitchen": "roomsKitchen.svg",
        "dining": "roomsDining.svg",
        "bedroom": "roomsBedroom.svg",
        "kids_bedroom": "roomsKidsbedroom.svg",
        "bathroom": "roomsBathroom.svg",
        "nursery": "roomsNursery.svg",
        "recreation": "roomsRecreation.svg",
        "office": "roomsOffice.svg",
        "gym": "roomsGym.svg",
        "hallway": "roomsHallway.svg",
        "toilet": "roomsToilet.svg",
        "front_door": "roomsFrontdoor.svg",
        "garage": "roomsGarage.svg",
        "terrace": "roomsTerrace.svg",
        "garden": "roomsOutdoor.svg",
        "driveway": "roomsDriveway.svg",
        "carport": "roomsCarport.svg",
        "home": "tabbarHome.svg",
        "downstairs": "zonesAreasGroundfloor.svg",
        "upstairs": "zonesAreasFirstfloor.svg",
        "top_floor": "zonesAreasSecondfloor.svg",
        "attic": "roomsAttic.svg",
        "guest_room": "roomsGuestroom.svg",
        "staircase": "roomsStaircase.svg",
        "lounge": "roomsLounge.svg",
        "man_cave": "roomsMancave.svg",
        "computer": "roomsComputer.svg",
        "studio": "roomsStudio.svg",
        "music": "otherMusic.svg",
        "tv": "otherWatchingMovie.svg",
        "reading": "otherReading.svg",
        "closet": "roomsCloset.svg",
        "storage": "roomsStorage.svg",
        "laundry_room": "roomsLaundryroom.svg",
        "balcony": "roomsBalcony.svg",
        "porch": "roomsPorch.svg",
        "barbecue": "roomsOutdoorSocialtime.svg",
        "pool": "roomsPool.svg",
        "other": "roomsOther.svg"
    }

    property var deviceArchetypeImages: {
        "bridge_v2": "devicesBridgesV2.svg",
        "unknown_archetype": "otherStar.svg",
        "classic_bulb": "bulbsClassic.svg",
        "sultan_bulb": "bulbsSultan.svg",
        "flood_bulb": "bulbFlood.svg",
        "spot_bulb": "bulbsSpot.svg",
        "candle_bulb": "bulbCandle.svg",
        "luster_bulb": "bulbGeneralGroup.svg",
        "pendant_round": "archetypesPendantRound.svg",
        "pendant_long": "archetypesPendantLong.svg",
        "ceiling_round": "archetypesCeilingRound.svg",
        "ceiling_square": "archetypesCeilingSquare.svg",
        "floor_shade": "archetypesFloorShade.svg",
        "floor_lantern": "archetypesFloorLantern.svg",
        "table_shade": "archetypesTableShade.svg",
        "recessed_ceiling": "archetypesRecessedCeiling.svg",
        "recessed_floor": "archetypesRecessedFloor.svg",
        "single_spot": "archetypesSingleSpot.svg",
        "double_spot": "archetypesDoubleSpot.svg",
        "table_wash": "archetypesTableWash.svg",
        "wall_lantern": "archetypesWallLantern.svg",
        "wall_shade": "archetypesWallShade.svg",
        "flexible_lamp": "archetypesDeskLamp.svg",
        "ground_spot": "archetypesFloorSpot.svg",
        "wall_spot": "archetypesWallSpot.svg",
        "plug": "devicesPlug.svg",
        "hue_go": "heroesHuego.svg",
        "hue_lightstrip": "heroesLightstrip.svg",
        "hue_iris": "heroesIris.svg",
        "hue_bloom": "heroesBloom.svg",
        "bollard": "archetypesBollard.svg",
        "wall_washer": "bulbsSpot.svg",
        "hue_play": "heroesHueplay.svg",
        "vintage_bulb": "bulbsFilament.svg",
        "vintage_candle_bulb": "bulbCandle.svg",
        "ellipse_bulb": "bulbsClassic.svg",
        "triangle_bulb": "bulbsClassic.svg",
        "small_globe_bulb": "bulbsClassic.svg",
        "large_globe_bulb": "bulbsClassic.svg",
        "edison_bulb": "bulbsFilament.svg",
        "christmas_tree": "otherChristmasTree.svg",
        "string_light": "bulbGeneralGroup.svg",
        "hue_centris": "archetypesDoubleSpot.svg",
        "hue_lightstrip_tv": "heroesLightstrip.svg",
        "hue_lightstrip_pc": "heroesLightstrip.svg",
        "hue_tube": "heroesLightstrip.svg",
        "hue_signe": "heroesLightstrip.svg",
        "pendant_spot": "archetypesPendantRound.svg",
        "ceiling_horizontal": "archetypesRecessedCeiling.svg",
        "ceiling_tube": "archetypesRecessedCeiling.svg",
    }
}
