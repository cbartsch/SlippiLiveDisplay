import Felgo 4.0
import QtQuick 2.0

import SlippiLive 1.0

App {

  DolphinConnection {
    onMessageReceived: (msg) => parser.parseSlippiMessage(msg)
  }

  SlippiEventParser {
    id: parser

    onConnectedChanged: console.log("Connected to Slippi changed:", connected)
    onGameRunningChanged: console.log("Slippi game running changed:", gameRunning)

  }

  NavigationStack {

    AppPage {
      title: qsTr("Main Page")

      Column {
        id: contentCol
        width: parent.width

        AppText {
          text: "Connected to Slippi: " +
                (parser.connected ? "%1/%2".arg(parser.slippiNick).arg(parser.slippiVersion) : "No")
        }

        AppText {
          text: "Slippi game running: " + (parser.gameRunning ? "Yes" : "No")
        }

        AppText {
          text: "Slippi version: " + parser.gameInfo.version
        }
      }
    }

  }
}
