import QtQuick 2.0
import QtQuick.Window
import Qt5Compat.GraphicalEffects
import Felgo 4.0

import SlippiLive

import "../controls"

Rectangle {
  id: playerOverlay

  width: 440
  height: 130

  color: "transparent"
  clip: true

  property int playerNum

  property PlayerInformation player: null
  property var profile
  property var rank
  property bool rtl

  Component.onCompleted: console.log("created overlay:", player, player?.nameTag)
  onPlayerChanged: console.log("player changed:", player, player?.nameTag)

  readonly property url imageUrl: profile && profile.ratingUpdateCount > 0
                                  ? rank ? "https://slippi.gg/" + rank.imageUrl : ""
  : "https://slippi.gg/static/media/rank_Unranked3.0f639e8b73090a7ba4a50f7bcc272f57.svg"

  readonly property int comboCount: player?.comboCount ?? 0

  onComboCountChanged: if(comboCount > 1) comboItemC.createObject(playerOverlay, { comboCount: comboCount })

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
      textItem.width: playerOverlay.width
      textItem.text: rank
                     ? profile.ratingUpdateCount > 1
                       ? "%1 (%2)".arg(rank.rank).arg(profile.ratingOrdinal.toFixed(0))
                       : "(No Rank)"
      : ""
      textItem.font.pixelSize: rank && profile.ratingUpdateCount > 1 ? 54 : 44
      textItem.horizontalAlignment: rtl ? Text.AlignRight : Text.AlignLeft
      textItem.verticalAlignment: Text.AlignVCenter
      shadowItem.radius: 5.0
      shadowItem.samples: 20
      //   height: 54
    }

    CustomText {
      textItem.width: playerOverlay.width
      textItem.text: playerOverlay.player?.slippiCode ?? ""
      textItem.font.pixelSize: 70
      textItem.horizontalAlignment: rtl ? Text.AlignRight : Text.AlignLeft
      textItem.font.family: "VCR OSD Mono"
      shadowItem.radius: 6.0
      shadowItem.samples: 20
    }
  }

  Component {
    id: comboItemC

    Item {
      id: comboItem

      property int comboCount

      Component.onCompleted: {
        console.log("Player combo:", comboCount)
        comboAnim.start()
      }

      anchors.verticalCenterOffset: playerOverlay.height
      anchors.verticalCenter: parent.verticalCenter
      width: parent.width
      height: parent.height

      CustomText {
        anchors.verticalCenter: parent.verticalCenter
        textItem.width: playerOverlay.width
        textItem.text: "Combo x%1".arg(comboItem.comboCount)
        textItem.font.pixelSize: 70
        textItem.horizontalAlignment: rtl ? Text.AlignRight : Text.AlignLeft
        textItem.font.family: "VCR OSD Mono"
        shadowItem.radius: 6.0
        shadowItem.samples: 20
      }

      SequentialAnimation {
        id: comboAnim

        PropertyAnimation {
          target: comboItem
          property: "anchors.verticalCenterOffset"
          easing.type: Easing.OutQuad
          from: -playerOverlay.height
          to: 0
          duration: 250
        }

        PauseAnimation {
          duration: 1000
        }

        PropertyAnimation {
          target: comboItem
          property: "anchors.verticalCenterOffset"
          easing.type: Easing.InQuad
          from: 0
          to: playerOverlay.height
          duration: 250
        }

        ScriptAction {
          script: comboItem.destroy()
        }
      }
    }
  }
}
