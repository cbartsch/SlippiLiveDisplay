import Felgo 4.0
import QtQuick 2.0

import SlippiLive 1.0

Item {
  property string currentMatchId: ""

  property var playerScores: [0, 0, 0, 0]

  property var netplayProfiles: ({})

  readonly property var playerTypes: ({
    [PlayerInformation.Human]: "Human",
    [PlayerInformation.CPU]: "CPU",
    [PlayerInformation.Demo]: "Demo",
    [PlayerInformation.Empty]: "Empty",
  })

  readonly property var charNames: [
    "Captain Falcon", "Donkey Kong", "Fox", "Mr. Game & Watch", "Kirby",
    "Bowser", "Link", "Luigi", "Mario", "Marth", "Mewtwo", "Ness", "Peach",
    "Pikachu", "Ice Climbers", "Jigglypuff", "Samus", "Yoshi",
    "Zelda/Sheik", // zelda and sheik are considered the same in stats
    "Sheik", "Falco", "Young Link", "Dr. Mario", "Roy", "Pichu",
    "Ganondorf", "Master Hand", "Fighting Wire Frame ♂", "Fighting Wire Frame ♀",
    "Giga Bowser", "Crazy Hand", "Sandbag", "SoPo", "NONE"
  ]

  readonly property var rankThresholds: [
    { minRating: 2350,    imageUrl: "static/media/rank_Master_III.5075fd077bf77bfa6c59985252e0cb04.svg",    rank: "Master 3"},
    { minRating: 2275,    imageUrl: "static/media/rank_Master_II.c0b5472d49d391d2063d8e2a19c9ea17.svg",     rank: "Master 2"},
    { minRating: 2191.75, imageUrl: "static/media/rank_Master_I.0ce2459fedf9e33ebee0cb3520a17e2f.svg",      rank: "Master 1"},
    { minRating: 2136.28, imageUrl: "static/media/rank_Diamond_III.ae3a5720a6ed48594efef54249095001.svg",   rank: "Diamond 3"},
    { minRating: 2073.67, imageUrl: "static/media/rank_Diamond_II.2f26cd8248bcf6c34ea1efe7f835b123.svg",    rank: "Diamond 2"},
    { minRating: 2003.92, imageUrl: "static/media/rank_Diamond_I.bcc6237a1e6be861f22f330bbff96964.svg",     rank: "Diamond 1"},
    { minRating: 1927.03, imageUrl: "static/media/rank_Platinum_III.cd9d7a413a1de2182caaae563b4e5023.svg",  rank: "Plat 3"},
    { minRating: 1843,    imageUrl: "static/media/rank_Platinum_II.ec1c571c896ed47ef2b14d8e2dd79fef.svg",   rank: "Plat 2"},
    { minRating: 1751.83, imageUrl: "static/media/rank_Platinum_I.7a22c1a7c7640af6b6bf2f7b5b439fc6.svg",    rank: "Plat 1"},
    { minRating: 1653.52, imageUrl: "static/media/rank_Gold_III.38643ad9dbef534920fc2361fd736d7a.svg",      rank: "Gold 3"},
    { minRating: 1548.07, imageUrl: "static/media/rank_Gold_II.951fc625063425ed048c864988e8d7b7.svg",       rank: "Gold 2"},
    { minRating: 1435.48, imageUrl: "static/media/rank_Gold_I.523b488f06ff22aaa013e94b6178f157.svg",        rank: "Gold 1"},
    { minRating: 1315.75, imageUrl: "static/media/rank_Silver_III.93588af0e9a6bc9406209d5ef3cc9dc7.svg",    rank: "Silver 3"},
    { minRating: 1188.88, imageUrl: "static/media/rank_Silver_II.7a97ee32770c36e78d9d7e9279c7ce38.svg",     rank: "Silver 2"},
    { minRating: 1054.87, imageUrl: "static/media/rank_Silver_I.b8069dd847a639127f6d3ff5363623f7.svg",      rank: "Silver 1"},
    { minRating: 913.72,  imageUrl: "static/media/rank_Bronze_III.b44c3a06f14234dec6f67e9b28088a6f.svg",    rank: "Bronze 3"},
    { minRating: 765.43,  imageUrl: "static/media/rank_Bronze_II.9d7a7994dbf087e3aea44f5b1c1409a7.svg",     rank: "Bronze 2"},
    { minRating: 0,       imageUrl: "static/media/rank_Bronze_I.90480ec5a08ee8d6048021f889933455.svg",      rank: "Bronze 1"}
  ]

  readonly property string gameType: {
    if(!parser.gameInfo) return ""

    var mId = parser.gameInfo?.matchId ?? ""
    if(mId.startsWith("mode.unranked")) return "Unranked"
    if(mId.startsWith("mode.ranked")) return "Ranked"
    if(mId.startsWith("mode.direct")) return "Direct"
    return ""
  }

  function getSlippiProfile(player) {
    var code = player.slippiCode

    if(player.playerType !== PlayerInformation.Human || !player.slippiCode) {
      // not a netplay player
      return
    }

    var bodyObj = {
      operationName: "UserProfilePageQuery",
      query: "
fragment profileFields on NetplayProfile {
  id
  ratingOrdinal
  ratingUpdateCount
  wins
  losses
  dailyGlobalPlacement
  dailyRegionalPlacement
  continent
  characters {
    character
    gameCount
    __typename
  }
  __typename
}

fragment userProfilePage on User {
  fbUid
  displayName
  connectCode {
    code
    __typename
  }
  status
  activeSubscription {
    level
    hasGiftSub
    __typename
  }
  rankedNetplayProfile {
    ...profileFields
    __typename
  }
  rankedNetplayProfileHistory {
    ...profileFields
    season {
      id
      startedAt
      endedAt
      name
      status
      __typename
    }
    __typename
  }
  __typename
}

query UserProfilePageQuery($cc: String, $uid: String) {
  getUser(fbUid: $uid, connectCode: $cc) {
    ...userProfilePage
    __typename
  }
}
",
      variables: {cc: code, uid: code}
    }

    var body = JSON.stringify(bodyObj)

    var r = new XMLHttpRequest();
    r.onreadystatechange = function() {
        if (r.readyState === 4) {
          if(r.status === 200) {
            var response = JSON.parse(r.responseText)
            var data = response.data

            if(data.getUser) {
              var user = data.getUser
              netplayProfiles[code] = user.rankedNetplayProfile
              netplayProfilesChanged()
            }
            else {
              console.warn("Response had no user data:", JSON.stringify(response, null, "  "))
            }
          }
          else {
            console.warn("Response status was not 200:", r.status,
                         "response:", r.responseText)
          }
        }
    };
    r.open("POST", "https://internal.slippi.gg/graphql");
    r.setRequestHeader("Content-Type", "application/json")
    r.send(body);
  }

  function getRank(rating) {
    return rankThresholds.find(t => rating > t.minRating) || { rank: "", imageUrl: "" }
  }

  function onGameStarted() {
    if(currentMatchId !== parser.gameInfo.matchId) {
      currentMatchId = parser.gameInfo.matchId
      playerScores = [0, 0, 0, 0]
      console.log("New game session:", currentMatchId)
    }
  }

  function onGameEnded(gameEndMethod, lrasPlayer, playerPlacements) {
    console.log("Game ended", parser.gameInfo.matchId, gameEndMethod, lrasPlayer, playerPlacements)

    playerPlacements.forEach((p, i) => {
      var ap = parser.gameInfo["player" + (i + 1)]

      if((lrasPlayer < 0 && p === 0) ||            // placements are 0-indexed, thus placement 0 is the winner
         (lrasPlayer >= 0 && lrasPlayer !== i && ap.playerType !== PlayerInformation.Empty)) {  // count LRAS as loss for that player
        playerScores[i]++
      }
    })

    playerScoresChanged()
  }
}
