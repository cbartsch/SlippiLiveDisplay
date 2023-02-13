import Felgo 4.0
import QtQuick 2.0
import QtQuick.Window 2.0
import Qt5Compat.GraphicalEffects

Window {
  id: gameOverlay
  width: 440
  height: 160
  visible: true
  title: "Game"
  color: "transparent"

  property string gameType

  AppText {
    id: gameText
    anchors.centerIn: parent

    text: gameType

    font.pixelSize: 80
    font.family: "Arial"
    style: Text.Outline
    styleColor: "black"
    antialiasing: true
    visible: false
  }

  DropShadow {
    anchors.fill: gameText
    horizontalOffset: 0
    verticalOffset: 0
    radius: 4.0
    spread: 1
    color: "black"
    source: gameText
  }
}

