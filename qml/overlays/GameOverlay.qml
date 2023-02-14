import Felgo 4.0
import QtQuick 2.0
import QtQuick.Window 2.0
import Qt5Compat.GraphicalEffects

import "../controls"

Window {
  id: gameOverlay
  width: 440
  height: 160
  visible: true
  title: "Game"
  color: "transparent"

  property string gameType
  property int gameNumber

  Column {
    anchors.centerIn: parent

    CustomText {
      anchors.horizontalCenter: parent.horizontalCenter
      textItem.text: gameType || ""
    }

    CustomText {
      anchors.horizontalCenter: parent.horizontalCenter
      textItem.font.pixelSize: 48
      textItem.text: gameNumber > 0 ? "Game %1".arg(gameNumber) : ""
    }
  }
}

