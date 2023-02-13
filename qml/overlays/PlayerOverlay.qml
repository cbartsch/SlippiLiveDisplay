import QtQuick 2.0
import QtQuick.Window
import Qt5Compat.GraphicalEffects
import Felgo 4.0

import SlippiLive

Item {

  property int playerNum

  property PlayerInformation player
  property var profile
  property var rank
  property bool rtl

  property real margins: overlay.height / 12

  Window {
    id: overlay
    width: 440
    height: 160
    visible: true
    title: "Player " + playerNum
    color: "transparent"

    AppImage {
      id: rankImg
      anchors.verticalCenter: parent.verticalCenter
      anchors.left: rtl ? parent.left : undefined
      anchors.right: rtl ? undefined : parent.right
      anchors.margins: margins
      height: parent.height - 5 * margins
      width: height
      source: rank ? "https://slippi.gg/" + rank.imageUrl : ""
      visible: false
      antialiasing: false
    }

    DropShadow {
      anchors.fill: rankImg
      horizontalOffset: 0
      verticalOffset: 0
      radius: 4.0
      samples: 20
      spread: 1
      color: "black"
      source: rankImg
      visible: !!rank
    }

    Column {
      anchors.verticalCenter: parent.verticalCenter
      anchors.left: rtl ? undefined : parent.left
      anchors.right: rtl ? parent.right : undefined
      anchors.margins: margins
      spacing: margins

      Item {
        width: overlay.width - 2 * margins
        height: ratingText.height

        AppText {
          id: ratingText
          width: parent.width
          horizontalAlignment: rtl ? Text.AlignRight : Text.AlignLeft
          text: rank ? "%1 (%2)".arg(rank.rank).arg(profile.ratingOrdinal.toFixed(0)) : ""
          font.pixelSize: 48
          font.family: "Arial"
          style: Text.Outline
          styleColor: "black"
          antialiasing: true
          visible: false
        }

        DropShadow {
          anchors.fill: ratingText
          horizontalOffset: 0
          verticalOffset: 0
          radius: 4.0
          samples: 20
          spread: 1
          color: "black"
          source: ratingText
        }
      }

      Item {
        width: overlay.width - 2 * margins
        height: codeText.height

        AppText {
          id: codeText
          width: parent.width
          horizontalAlignment: rtl ? Text.AlignRight : Text.AlignLeft
          text: player.slippiCode || ""
          font.pixelSize: 64
          font.family: "VCR OSD Mono"
          style: Text.Outline
          styleColor: "black"
          antialiasing: false
          visible: false
        }

        DropShadow {
          anchors.fill: codeText
          horizontalOffset: 0
          verticalOffset: 0
          radius: 4.0
          samples: 20
          spread: 1
          color: "black"
          source: codeText
        }
      }
    }
  }
}
