import Felgo 4.0
import QtQuick 2.0
import QtQuick.Window 2.0
import Qt5Compat.GraphicalEffects

import SlippiLive 1.0

import "model"
import "overlays"
import "pages"

App {
  id: app

  DataModel {
    id: dataModel
  }

  DolphinConnection {
    id: dolphin

    onMessageReceived: (msg) => parser.parseSlippiMessage(msg)
  }

  SlippiEventParser {
    id: parser

    onConnectedChanged: console.log("Connected to Slippi changed:", connected)
    onGameRunningChanged: console.log("Slippi game running changed:", gameRunning, gameInfo.matchId)

    onGameStarted: dataModel.onGameStarted()
    onGameEnded: (gameEndMethod, lrasPlayer, playerPlacements) => dataModel.onGameEnded(gameEndMethod, lrasPlayer, playerPlacements)
  }

  onInitTheme: {
    Theme.colors.tintColor = "#21BA45" // slippi green

    // dark theme
    Theme.colors.textColor = "white"
    Theme.colors.secondaryTextColor = "#888"
    Theme.colors.secondaryBackgroundColor = "#222"
    Theme.colors.controlBackgroundColor = "#111"
    Theme.colors.dividerColor = "#222"
    Theme.colors.selectedBackgroundColor = "#888"
    Theme.colors.backgroundColor = "black"

    Theme.colors.inputCursorColor = "white"

    Theme.tabBar.backgroundColor = Theme.backgroundColor

    Theme.listItem.backgroundColor = Theme.controlBackgroundColor

    Theme.navigationTabBar.titleOffColor= "white"
    Theme.navigationTabBar.backgroundColor = Theme.controlBackgroundColor

    Theme.appButton.rippleEffect = true
    Theme.appButton.horizontalMargin = 0
    Theme.appButton.horizontalPadding = dp(2)
  }

  NavigationStack {
    InfoPage { }
  }

  GameOverlay {
    gameType: dataModel.gameType
    gameNumber: parser.gameInfo.gameNumber
    scoreP1: dataModel.playerScores[0]
    scoreP2: dataModel.playerScores[1]

    x: app.x + app.width
    y: app.y
  }

  Repeater {
    model: 2

    PlayerOverlay {
      playerNum: index + 1

      player: [parser.gameInfo.player1, parser.gameInfo.player2][index]
      profile: dataModel.netplayProfiles && dataModel.netplayProfiles[player.slippiCode] || null
      rank: profile ? dataModel.getRank(profile.ratingOrdinal) : null

      rtl: playerNum === 2

      x: app.x + app.width
      y: app.y + (height + 50) * (index + 1)
    }
  }
}

