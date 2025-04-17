import Felgo 4.0
import QtQuick 2.0
import Qt5Compat.GraphicalEffects

import SlippiLive

import "../controls"
import "../model"

FlickablePage {
  title: "Slippi Live Display"

  readonly property real iconSize: dp(40)
  readonly property color pinkColor: "#FF51AD"

  flickable.contentHeight: contentCol.height

  Column {
    id: contentCol
    width: parent.width

    SimpleSection {
      title: "Status"
    }

    AppListItem {
      enabled: false
      backgroundColor: Theme.backgroundColor
      text: "Connected to Slippi: " +
            (parser.connected ? "%1 / %2".arg(parser.slippiNick).arg(parser.slippiVersion) : "No")
    }

    AppListItem {
      enabled: false
      backgroundColor: Theme.backgroundColor
      text: "Slippi game running: %1, version: %2"
      .arg(parser.gameInfo ? "Yes" : "No")
      .arg(parser.gameInfo?.version ?? "unknown")

      detailText: parser.gameInfo ? "AL: %2, frozen PS: %3, minor scene: %4, major scene: %5"
                                    .arg(parser.gameInfo.isPal)
                                    .arg(parser.gameInfo.isFrozenPS)
                                    .arg(parser.gameInfo.minorScene)
                                    .arg(parser.gameInfo.majorScene)
                                  : "No game running"
    }

    AppListItem {
      enabled: false
      backgroundColor: Theme.backgroundColor
      text: "Current match ID: " + parser.gameInfo?.matchId ?? "unknown"
      detailText: "Scores: " + dataModel.playerScores
        .filter((item, index) => parser.players[index]?.playerType !== PlayerInformation.Empty)
        .join(" - ")
    }

    SimpleSection {
      title: "Players"
      visible: parser.gameRunning
    }

    Repeater {
      model: parser.gameRunning ? parser.players : ""

      AppListItem {
        property var profile: dataModel.netplayProfiles[modelData.slippiCode] || null
        property var rank: profile ? dataModel.getRank(profile.ratingOrdinal) : null

        visible: modelData.playerType !== PlayerInformation.Empty

        enabled: false
        backgroundColor: Theme.backgroundColor

        text: "Player %1: %2 (%3, %4)"
        .arg(index + 1)
        .arg(modelData.nameTag || modelData.slippiName || ("No name"))
        .arg(dataModel.playerTypes[modelData.playerType] || "Unknown")
        .arg(dataModel.charNames[modelData.charId])

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

        Component.onCompleted: dataModel.getSlippiProfile(modelData)
      }
    }

    SimpleSection {
      title: "About"
    }

    AppListItem {
      text: "SlippiLiveDisplay v%1".arg(Constants.versionName)
      detailText: "%1 build (%2)".arg(Constants.buildName).arg(Constants.versionCode)
      enabled: false
      backgroundColor: Theme.backgroundColor
    }

    AppListItem {
      text: "Made by Chrisu"
      detailText: "Feel free to give credit when using this app :)"
      enabled: false
      backgroundColor: Theme.backgroundColor
    }

    CustomListItem {
      text: "Follow me on Twitter"
      detailText: Constants.twitterUrl

      hasExternalLink: true
      backgroundColor: Theme.controlBackgroundColor

      leftItem: Item {
        anchors.verticalCenter: parent.verticalCenter
        height: iconSize
        width: height

        AppIcon {
          iconType: IconType.twitter
          color: "#1DA1F2"
          size: parent.height
          anchors.centerIn: parent
        }
      }

      onSelected: nativeUtils.openUrl(Constants.twitterUrl)
    }

    CustomListItem {
      text: "Source code at GitHub"
      detailText: Constants.githubUrl

      hasExternalLink: true
      backgroundColor: Theme.controlBackgroundColor

      leftItem: Item {
        anchors.verticalCenter: parent.verticalCenter
        height: iconSize
        width: height

        AppIcon {
          iconType: IconType.github
          color: Theme.tintColor
          size: parent.height
          anchors.centerIn: parent
        }
      }
      onSelected: nativeUtils.openUrl(Constants.githubUrl)
    }

    CustomListItem {
      text: "Support me on Patreon"
      detailText: Constants.patreonUrl

      hasExternalLink: true
      backgroundColor: Theme.controlBackgroundColor

      leftItem: Item {
        anchors.verticalCenter: parent.verticalCenter
        height: iconSize
        width: height

        AppImage {
          id: patreonImg
          anchors.fill: parent
          visible: false
          source: "../../assets/img/patreon-logo.png"
          fillMode: Image.PreserveAspectFit
        }

        ColorOverlay {
          anchors.fill: parent
          source: patreonImg
          color: pinkColor
        }
      }

      onSelected: nativeUtils.openUrl(Constants.patreonUrl)
    }

    CustomListItem {
      text: "More of my Stuff"
      detailText: Constants.homeUrl

      hasExternalLink: true
      backgroundColor: Theme.controlBackgroundColor

      leftItem: Item {
        anchors.verticalCenter: parent.verticalCenter
        height: iconSize
        width: height

        AppIcon {
          iconType: IconType.home
          color: pinkColor
          size: parent.height
          anchors.centerIn: parent
        }
      }
      onSelected: nativeUtils.openUrl(Constants.homeUrl)
    }

    SimpleSection {
      title: "Credits"
    }

    CustomListItem {
      text: "Made for use with Project Slippi"
      detailText: Constants.slippiUrl

      hasExternalLink: true
      backgroundColor: Theme.controlBackgroundColor

      leftItem: Item {
        height: iconSize
        width: height
        anchors.verticalCenter: parent.verticalCenter

        AppImage {
          id: slippiImg
          anchors.fill: parent
          visible: false
          source: "../../../assets/img/slippi.svg"
          fillMode: Image.PreserveAspectFit
        }

        ColorOverlay {
          anchors.fill: parent
          source: slippiImg
          color: Theme.tintColor
        }
      }

      onSelected: nativeUtils.openUrl(Constants.slippiUrl)
    }

    CustomListItem {
      text: qsTr("Built with Felgo %1 (based on Qt %2)").arg(Constants.felgoVersion).arg(Constants.qtVersion)
      detailText: Constants.felgoUrl

      hasExternalLink: true
      backgroundColor: Theme.controlBackgroundColor

      leftItem: AppImage {
        source: "../../../assets/img/felgo-logo.png"

        height: iconSize
        width: height
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.PreserveAspectFit
      }

      onSelected: nativeUtils.openUrl(Constants.felgoUrl)
    }

    CustomListItem {
      text: "Shoutouts to DAFT Home <3"
      detailText: "And everyone else who helped test and improve this app"

      hasExternalLink: true
      backgroundColor: Theme.controlBackgroundColor

      leftItem: AppImage {
        source: "../../../assets/img/daft.png"

        height: iconSize
        width: height
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.PreserveAspectFit
      }

      onSelected: nativeUtils.openUrl(Constants.discordUrl)
    }
  }
}
