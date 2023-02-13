import Felgo 4.0
import QtQuick 2.0

import SlippiLive

AppPage {
  title: "Slippi Live Display"

  Column {
    id: contentCol
    width: parent.width

    AppListItem {
      enabled: false
      text: "Connected to Slippi: " +
            (parser.connected ? "%1 / %2".arg(parser.slippiNick).arg(parser.slippiVersion) : "No")
    }

    AppListItem {
      enabled: false
      text: "Slippi game running: %1, version: %2"
      .arg(parser.gameRunning ? "Yes" : "No")
      .arg(parser.gameInfo.version)

      detailText: "%1, PAL: %2, frozen PS: %3, minor scene: %4, major scene: %5"
      .arg(parser.gameInfo.matchId)
      .arg(parser.gameInfo.isPal)
      .arg(parser.gameInfo.isFrozenPS)
      .arg(parser.gameInfo.minorScene)
      .arg(parser.gameInfo.majorScene)
    }

      Repeater {
        model: parser.gameRunning
               ? [
                   parser.gameInfo.player1,
                   parser.gameInfo.player2,
                   parser.gameInfo.player3,
                   parser.gameInfo.player4
                 ]
               : ""

        AppListItem {
          property var profile: netplayProfiles[modelData.slippiCode] || null
          property var rank: profile ? getRank(profile.ratingOrdinal) : null

          visible: modelData.playerType !== PlayerInformation.Empty

          text: "Player %1: %2 (%3, %4)"
          .arg(index + 1)
          .arg(modelData.nameTag || modelData.slippiName || ("No name"))
          .arg(playerTypes[modelData.playerType] || "Unknown")
          .arg(charNames[modelData.charId])

          detailText: "Slippi: %1 (%2)%3"
          .arg(modelData.slippiName)
          .arg(modelData.slippiCode)
          .arg(profile
          ? " %1 (%2)".arg(rank.rank).arg(profile.ratingOrdinal)
          : "")

          rightItem: AppImage {
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: dp(Theme.contentPadding)
            height: dp(48)
            width: height
            source: rank ? "https://slippi.gg/" + rank.imageUrl : ""
            visible: !!rank
          }

          enabled: false
          Component.onCompleted: getSlippiProfile(modelData)
        }
      }
    }
  }
