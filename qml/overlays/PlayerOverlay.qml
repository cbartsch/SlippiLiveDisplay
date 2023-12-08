import QtQuick 2.0
import QtQuick.Window
import Qt5Compat.GraphicalEffects
import Felgo 4.0

import SlippiLive

import "../controls"

Rectangle {
  id: playerOverlay

  width: 440 + 2*border.width
  height: 130 + 2*border.width

  color: "transparent"
  border.color: "white"
  border.width: 2

  clip: true

  property int playerNum

  property PlayerInformation player: null
  property var profile
  property var rank
  property bool rtl

  readonly property url imageUrl: profile && profile.ratingUpdateCount > 1
                                  ? rank ? "https://slippi.gg/" + rank.imageUrl : ""
  : "https://slippi.gg/static/media/rank_Unranked3.0f639e8b73090a7ba4a50f7bcc272f57.svg"

  readonly property int comboCount:          player?.comboCount          ?? 0
  readonly property int lCancelState:        player?.lCancelState        ?? PlayerInformation.Unknown
  readonly property int lCancelFrames:       player?.lCancelFrames       || 0
  readonly property int wavedashFrame:       player?.wavedashFrame       || 0
  readonly property int intangibilityFrames: player?.intangibilityFrames || 0
  readonly property int cycloneBPresses:     player?.cycloneBPresses     || 0
  readonly property bool isFastFalling:      player?.isFastFalling       ?? false

  readonly property int galintFrames: Math.max(0, intangibilityFrames - 10) // subtract 10 frames of landing lag
  readonly property bool isLedgedash: intangibilityFrames > 0

  readonly property var allOverlays: []

  signal showingOverlay(Item overlay)
  signal removingOverlay(Item overlay)

  onComboCountChanged:    if(comboCount > 1)                                       showOverlay({
                                                                                                 text: "Combo x%1".arg(comboCount),
                                                                                                 duration: 1000,
                                                                                                 show: settings.showComboOverlay
                                                                                               })

  onLCancelStateChanged:  if(lCancelState === PlayerInformation.Successful || lCancelState === PlayerInformation.Unsuccessful)
                                                                                   showOverlay({
                                                                                                 text: "L-Cancel:\n%1/7".arg(lCancelFrames),
                                                                                                 color: lCancelState === PlayerInformation.Successful
                                                                                                        ? Qt.hsva(0.33, 0.4, 1) : Qt.hsva(0.00, 0.4, 1),
                                                                                                 show: settings.showLCancelOverlay
                                                                                               })

  onIsFastFallingChanged: if(isFastFalling)                                        showOverlay({
                                                                                                 text: "Fastfall\nFrame %1".arg(player.fastFallFrame),
                                                                                                 color: Qt.hsva(0.33, 0.4 * Math.max(0, (6 - player.fastFallFrame) / 5), 1),
                                                                                                 show: settings.showFastfallOverlay
                                                                                               })

  onWavedashFrameChanged: if(wavedashFrame > 0)                                    showOverlay({
                                                                                                 text: isLedgedash
                                                                                                       ? "Ledgedash\nGALINT: %3".arg(galintFrames)
                                                                                                       : "Wavedash\nFrame %1 %2°".arg(wavedashFrame).arg(player.wavedashAngle.toFixed(1)),
                                                                                                 duration: 1000,
                                                                                                 color: Qt.hsva(0.33, 0.4 * Math.max(0, isLedgedash
                                                                                                                                     ? (galintFrames) / 10
                                                                                                                                     : (6 - wavedashFrame) / 5),
                                                                                                                1),
                                                                                                 show: settings.showWavedashOverlay
                                                                                               })

  onCycloneBPressesChanged:  if(cycloneBPresses > 1)                               showOverlay({
                                                                                                 text: "Cyclone\nmash: %1".arg(cycloneBPresses),
                                                                                                 color: Qt.hsva(0.33, 0.8 * cycloneBPresses / 19, 1),
                                                                                                 show: settings.showComboOverlay
                                                                                               })

  function showOverlay(properties) {
    if(properties.show) {
      delete properties.show
      var overlayItem = overlayItemC.createObject(playerOverlay, properties)
      showingOverlay(overlayItem)
    }
    else {
      console.debug("Overlay disabled:", properties.text)
    }
  }

  onShowingOverlay: item => {
    allOverlays.push(item)
    allOverlaysChanged()
  }

  onRemovingOverlay: item => {
    allOverlays.shift()
    allOverlaysChanged()
  }

  Item {
    id: overlayContent
    anchors.fill: parent
    anchors.leftMargin: 5
    anchors.rightMargin: 5

    opacity: allOverlays.length === 0 ? 1 : 0.5

    Behavior on opacity {
      PropertyAnimation {
        duration: 200
        easing.type: Easing.OutQuad
      }
    }

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
        textItem.text: playerOverlay.player?.slippiCode || ""
        textItem.font.pixelSize: 70
        textItem.horizontalAlignment: rtl ? Text.AlignRight : Text.AlignLeft
        textItem.font.family: "VCR OSD Mono"
        shadowItem.radius: 6.0
        shadowItem.samples: 20
      }
    }
  }

  Component {
    id: overlayItemC

    Item {
      id: overlayItem

      property alias text: textItem.textItem.text
      property alias font: textItem.textItem.font
      property alias color: textItem.textItem.color
      property alias duration: pauseTimer.duration

      Component.onCompleted: comboAnim.start()

      Connections {
        target: playerOverlay

        // when starting another overlay, fade out this one
        function onShowingOverlay(other) {
          if(other !== overlayItem) {
            fadeOutAnim.start()
          }
        }
      }

      anchors.verticalCenterOffset: playerOverlay.height
      anchors.verticalCenter: parent.verticalCenter
      width: parent.width
      height: parent.height

      CustomText {
        id: textItem
        anchors.verticalCenter: parent.verticalCenter
        textItem.width: playerOverlay.width - 10

        textItem.maximumLineCount: 2
        textItem.elide: Text.ElideRight
        textItem.font.pixelSize: 50
        textItem.horizontalAlignment: rtl ? Text.AlignRight : Text.AlignLeft
        textItem.font.family: "VCR OSD Mono"
        textItem.leftPadding: 4
        textItem.rightPadding: 4
        textItem.color: "white"
        shadowItem.radius: 6.0
        shadowItem.samples: 20
      }

      PropertyAnimation {
        id: fadeOutAnim
        target: overlayItem
        property: "opacity"
        easing.type: Easing.OutQuad
        from: 1
        to: 0.5
        duration: 200
      }

      SequentialAnimation {
        id: comboAnim

        PropertyAnimation {
          target: overlayItem
          property: "anchors.verticalCenterOffset"
          easing.type: Easing.OutQuad
          from: -playerOverlay.height
          to: 0
          duration: 200
        }

        PauseAnimation {
          id: pauseTimer
          duration: 500
        }

        ScriptAction {
          script: playerOverlay.removingOverlay(overlayItem)
        }

        PropertyAnimation {
          target: overlayItem
          property: "anchors.verticalCenterOffset"
          easing.type: Easing.InQuad
          from: 0
          to: playerOverlay.height
          duration: 200
        }

        ScriptAction {
          script: overlayItem.destroy()
        }
      }
    }
  }
}
