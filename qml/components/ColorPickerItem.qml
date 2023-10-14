import QtQuick 2.5
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0

Item {
    id: colorPicker
    property color color: '#00000000'
    property double mirek: 0
    readonly property int minMirek: 153
    readonly property int maxMirek: 500
    readonly property color coldCt: '#FFFEFA'
    readonly property color warmCt: '#FF890E'
    property bool rgbaVisible: true
    property bool ctVisible: true
    height: pickerRectangle.height

    Component.onCompleted: {
        if (color != '#00000000') {
            setColor(color, true)
        }
        else if (mirek >= minMirek && mirek <= maxMirek){
            setMirek(mirek, true)
        }
        else {
            setMirek(minMirek + (maxMirek - minMirek) / 2, true)
        }
    }

    onColorChanged: {
        //setColor(color, false)
        if (debug) {
            console.log(color)
        }
    }
    onMirekChanged: {
        //setMirek(mirek, false)
        if (debug) {
            console.log(mirek)
        }
    }

    function pickColor() {
        var r = 1.0
        var g = 1.0
        var b = 1.0
        var a = 1.0
        var imgData
        if (dropArea.drag.y <= colorCanvas.height) {
            // rgb color
            imgData = colorCanvas.context.getImageData(Math.min(dropArea.drag.x, colorCanvas.width-1), Math.min(dropArea.drag.y, colorCanvas.height-1), 1, 1)
            r = imgData.data[0]/255.0
            g = imgData.data[1]/255.0
            b = imgData.data[2]/255.0
            a = 1.0 - dropArea.drag.y / colorCanvas.height
            color = Qt.rgba(r, g, b, a)
            mirek = 0
        }
        else {
            // white color temprerature
            imgData = ctCanvas.context.getImageData(Math.min(dropArea.drag.x, ctCanvas.width-1), 0, 1, 1)
            r = imgData.data[0]/255.0
            g = imgData.data[1]/255.0
            b = imgData.data[2]/255.0
            a = imgData.data[3]/255.0
            color = '#00000000'
            mirek = minMirek + (dropArea.drag.x/ctCanvas.width)*(maxMirek-minMirek)
        }
    }

    function setColor(color, updatePickerPos) {
        updatePickerPos = typeof(updatePickerPos) === "undefined" ? true : updatePickerPos
        //console.log("Set color", color)
        //console.log(color.r, color.g, color.b, color.a)
        var min = Math.min(color.r, color.g, color.b)
        var r = color.r - (1-min)
        var g = color.g - (1-min)
        var b = color.b - (1-min)
        var w = colorPicker.width/5
        mirek = 0
        if (updatePickerPos) {
            pickerIcon.Drag.start()
            pickerIcon.y = (1.0 - color.a - min) * colorPicker.width
            if (r === 1.0 && b === 0.0) {
                pickerIcon.x = w * (0.0 + (g))
            }
            else if (g === 1.0 && b === 0.0) {
                pickerIcon.x = w * (1.0 + (1.0-r))
            }
            else if (r === 0.0 && g === 1.0) {
                pickerIcon.x = w * (2.0 + (b))
            }
            else if (r === 0.0 && b === 1.0) {
                pickerIcon.x = w * (3.0 + (1.0-g))
            }
            else if (g === 0.0 && b === 1.0) {
                pickerIcon.x = w * (4.0 + (r))
            }
            pickerIcon.Drag.drop()
        }
    }

    function setMirek(mirek, updatePickerPos) {
        updatePickerPos = typeof(updatePickerPos) === "undefined" ? true : updatePickerPos
        if (mirek >= minMirek && mirek <= maxMirek && updatePickerPos) {
            pickerIcon.Drag.start()
            pickerIcon.x = ((mirek - minMirek) / (maxMirek - minMirek)) * ctCanvas.width - pickerIcon.width/2
            pickerIcon.y = ctCanvas.y + ctCanvas.height/2 - pickerIcon.height/2
            pickerIcon.Drag.drop()
        }
        else {
            pickerIcon.y = ctCanvas.y + ctCanvas.height/2 - pickerIcon.height/2
        }
    }

    Rectangle {
        id: pickerRectangle
        width: parent.width
        height: canvasColumn.height + pickerIcon.height
        color: 'white'

        Rectangle {
            id: canvasRectangle
            anchors.fill: parent
            color: colorPicker.color

            DropArea {
                id: dropArea
                width: parent.width - pickerIcon.width
                height: canvasColumn.height
                anchors.centerIn: parent
                onPositionChanged: {
                    pickColor()
                }

                Column {
                    id: canvasColumn
                    width: parent.width

                    Canvas {
                        id: colorCanvas
                        width: parent.width
                        height: width
                        visible: rgbaVisible
                        onVisibleChanged: requestPaint()
                        onPaint: {
                            var ctx = getContext('2d')
                            ctx.rect(0, 0, parent.width, parent.height)
                            var grd = ctx.createLinearGradient(0, 0, width, 0)
                            grd.addColorStop(0/5, '#ff0000')
                            grd.addColorStop(1/5, '#ffff00')
                            grd.addColorStop(2/5, '#00ff00')
                            grd.addColorStop(3/5, '#00ffff')
                            grd.addColorStop(4/5, '#0000ff')
                            grd.addColorStop(5/5, '#ff00ff')
                            ctx.fillStyle = grd
                            ctx.fill()
                        }
                        Canvas {
                            id: alphaCanvas
                            anchors.fill: parent
                            onVisibleChanged: requestPaint()
                            onPaint: {
                                var ctx = getContext('2d')
                                ctx.rect(0, 0, parent.width, parent.height)
                                var grd = ctx.createLinearGradient(0, 0, 0, height)
                                grd.addColorStop(0, 'transparent')
                                grd.addColorStop(1, 'white')
                                ctx.fillStyle = grd
                                ctx.fill()
                            }
                        }
                    }
                    Separator {
                        width: parent.width
                        horizontalAlignment: Qt.AlignHCenter
                        color: Theme.darkPrimaryColor
                        visible: colorCanvas.visible && ctCanvas.visible
                    }
                    Canvas {
                        id: ctCanvas
                        width: parent.width
                        height: Theme.itemSizeSmall
                        visible: ctVisible
                        onVisibleChanged: requestPaint()
                        onPaint: {
                            var ctx = getContext('2d')
                            ctx.rect(0, 0, parent.width, parent.height)
                            var grd = ctx.createLinearGradient(0, 0, width, 0)
                            grd.addColorStop(0.0, coldCt)
                            grd.addColorStop(1.0, warmCt)
                            ctx.fillStyle = grd
                            ctx.fill()
                        }
                    }
                }

                Icon {
                    id: pickerIcon
                    source: "image://theme/icon-m-dot?" + (pickerArea.pressed ? Theme.darkSecondaryColor : Theme.darkPrimaryColor)

                    width: Theme.iconSizeMedium
                    height: Theme.iconSizeMedium

                    Drag.active: pickerArea.drag.active
                    Drag.hotSpot.x: width/2
                    Drag.hotSpot.y: height/2

                    MouseArea {
                        id: pickerArea
                        anchors.fill: parent
                        drag.target: parent
                        drag.minimumX: dropArea.x - pickerIcon.width
                        drag.maximumX: dropArea.x + dropArea.width - pickerIcon.width
                        drag.minimumY: dropArea.y - pickerIcon.height
                        drag.maximumY: dropArea.y + dropArea.height - pickerIcon.height
                        onReleased: {
                            if (dropArea.drag.y > colorCanvas.height) {
                                pickerIcon.y = ctCanvas.y + ctCanvas.height/2 - pickerIcon.height/2
                            }
                        }
                    }
                }
            }
        }
    }
}
