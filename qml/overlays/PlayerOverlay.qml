import QtQuick 2.0
import QtQuick.Window
import Qt5Compat.GraphicalEffects
import Felgo 4.0

import SlippiLive

import "../controls"

Item {

  property int playerNum

  property PlayerInformation player
  property var profile
  property var rank
  property bool rtl

  readonly property url imageUrl: profile && profile.ratingUpdateCount > 0
  ? rank ? "https://slippi.gg/" + rank.imageUrl : ""
  : "https://slippi.gg/static/media/rank_Unranked3.0f639e8b73090a7ba4a50f7bcc272f57.svg"

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
      height: 96
      width: height
      source: imageUrl
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

      CustomText {
        textItem.width: overlay.width
        textItem.text: rank
                       ? profile.ratingUpdateCount > 1
                         ? "%1 (%2)".arg(rank.rank).arg(profile.ratingOrdinal.toFixed(0))
                         : "Not ranked"
                       : ""
        textItem.font.pixelSize: 54
        textItem.horizontalAlignment: rtl ? Text.AlignRight : Text.AlignLeft
        shadowItem.radius: 5.0
        shadowItem.samples: 20
      }

      CustomText {
        textItem.width: overlay.width
        textItem.text: player.slippiCode || ""
        textItem.font.pixelSize: 70
        textItem.horizontalAlignment: rtl ? Text.AlignRight : Text.AlignLeft
        textItem.font.family: "VCR OSD Mono"
        shadowItem.radius: 5.0
        shadowItem.samples: 20
      }
    }
  }
}
