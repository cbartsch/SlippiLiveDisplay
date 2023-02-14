import Felgo 4.0
import QtQuick 2.0
import QtQuick.Window 2.0
import Qt5Compat.GraphicalEffects

Item {
  id: customText

  property alias textItem: textItem
  property alias shadowItem: shadowItem

  width: textItem.width
  height: textItem.height

  AppText {
    id: textItem
    anchors.centerIn: parent

    font.pixelSize: 80
    font.family: "Calibri"
    antialiasing: true
    visible: false
  }

  DropShadow {
    id: shadowItem
    anchors.fill: textItem
    horizontalOffset: 0
    verticalOffset: 0
    radius: 4.0
    spread: 1
    color: "black"
    source: textItem
  }
}

