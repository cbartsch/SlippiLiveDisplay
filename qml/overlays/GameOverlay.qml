import Felgo 4.0
import QtQuick 2.0
import QtQuick.Window 2.0
import Qt5Compat.GraphicalEffects

import "../controls"

Window {
  id: gameOverlay
  width: 440
  height: 130
  visible: true
  title: "Game"
  color: "transparent"

  flags: Qt.Dialog | Qt.FramelessWindowHint

  property string gameType
  property int gameNumber
  property int scoreP1: -1
  property int scoreP2: -1

  Column {
    anchors.centerIn: parent
    spacing: -10

    CustomText {
      anchors.horizontalCenter: parent.horizontalCenter
      textItem.text: gameType || ""
    }

    CustomText {
      anchors.horizontalCenter: parent.horizontalCenter
      textItem.font.pixelSize: 60
      textItem.text: gameNumber > 0 ? "Game %1".arg(gameNumber) : ""
      visible: false
    }

    CustomText {
      anchors.horizontalCenter: parent.horizontalCenter
      textItem.font.pixelSize: 60
      textItem.text: gameNumber > 0 && scoreP1 >= 0 && scoreP2 >= 0 ? "%1 - %2".arg(scoreP1).arg(scoreP2) : ""
    }
  }
}

