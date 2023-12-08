#ifndef EVENTPARSER_H
#define EVENTPARSER_H

#include <QObject>
#include <QBuffer>
#include <QMap>
#include <QVariant>
#include <QQmlListProperty>

#include "slippievents.h"

const int NUM_PLAYERS = 4;

struct PlayerInformation : public QObject {
    Q_OBJECT
    Q_PROPERTY(quint32 dashbackFix MEMBER dashbackFix CONSTANT)
    Q_PROPERTY(quint32 shieldDropFix MEMBER shieldDropFix CONSTANT)
    Q_PROPERTY(quint8 playerType MEMBER playerType CONSTANT)
    Q_PROPERTY(quint8 charId MEMBER charId CONSTANT)

    Q_PROPERTY(QString nameTag MEMBER nameTag CONSTANT)
    Q_PROPERTY(QString slippiCode MEMBER slippiCode CONSTANT)
    Q_PROPERTY(QString slippiName MEMBER slippiName CONSTANT)
    Q_PROPERTY(QString slippiUid MEMBER slippiUid CONSTANT)

    // generic stats
    Q_PROPERTY(quint32 comboCount MEMBER comboCount WRITE setComboCount NOTIFY comboCountChanged)
    Q_PROPERTY(LCancelState lCancelState MEMBER lCancelState WRITE setLCancelState NOTIFY lCancelStateChanged)
    Q_PROPERTY(int lCancelFrames MEMBER framesSinceLCancel NOTIFY lCancelFramesChanged)
    Q_PROPERTY(int intangibilityFrames MEMBER intangibilityFrames NOTIFY intangibilityFramesChanged)
    Q_PROPERTY(int wavedashFrame MEMBER wavedashFrame NOTIFY wavedashChanged)
    Q_PROPERTY(qreal wavedashAngle MEMBER wavedashAngle NOTIFY wavedashChanged)
    Q_PROPERTY(bool isFastFalling MEMBER isFastFalling NOTIFY isFastFallingChanged)
    Q_PROPERTY(int fastFallFrame MEMBER framesSinceFall NOTIFY framesSinceFallChanged)

    // char specific stats
    Q_PROPERTY(int cycloneBPresses MEMBER cycloneBPresses NOTIFY cycloneBPressesChanged)

signals:
    void comboCountChanged();
    void lCancelStateChanged();
    void lCancelFramesChanged();
    void intangibilityFramesChanged();
    void wavedashChanged();
    void isFastFallingChanged();
    void framesSinceFallChanged();
    void cycloneBPressesChanged();

public:
    PlayerInformation(QObject *parent = nullptr);

    enum ControllerFixType : quint32 { Off = 0, UCF = 1, Dween = 2 };
    Q_ENUM(ControllerFixType);

    enum PlayerType : quint8 { Human = 0, CPU = 1, Demo = 2, Empty = 3};
    Q_ENUM(PlayerType);

    enum LCancelState : quint8 { Unknown = 0, Successful = 1, Unsuccessful = 2};
    Q_ENUM(LCancelState);

    void setComboCount(quint32 newComboCount);
    void setLCancelState(const LCancelState &newLCancelState);
    void setWavedash(int frame, qreal angle);
    void setCycloneBPresses(int bPresses);

    bool analyzeFrame();

    // fields set from EventParser
    PreFrameData preFrame, preFramePrev;
    PostFrameData postFrame, postFramePrev;

    quint32 dashbackFix = Off, shieldDropFix = Off;
    quint8 charId = 0, playerType = Empty;
    QString nameTag, slippiCode, slippiName, slippiUid;

private:
    // fields set from analyzeFrame()
    bool isLCancel = false, isFalling = false, isFastFalling = false;
    int framesSinceLCancel = 0, framesSinceFall = 0;
    quint32 comboCount = 0;
    LCancelState lCancelState = Unknown;
    int wavedashFrame = 0;
    int intangibilityFrames = 0;
    qreal wavedashAngle = 0;
    int cycloneBPresses = 0;
};
Q_DECLARE_METATYPE(PlayerInformation);

struct GameInformation : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString version MEMBER version CONSTANT)
    Q_PROPERTY(quint32 randomSeed MEMBER seed CONSTANT)

    Q_PROPERTY(PlayerInformation *player1 READ player1 CONSTANT)
    Q_PROPERTY(PlayerInformation *player2 READ player2 CONSTANT)
    Q_PROPERTY(PlayerInformation *player3 READ player3 CONSTANT)
    Q_PROPERTY(PlayerInformation *player4 READ player4 CONSTANT)

    Q_PROPERTY(quint8 isPal MEMBER isPal CONSTANT)
    Q_PROPERTY(quint8 isFrozenPS MEMBER isFrozenPS CONSTANT)
    Q_PROPERTY(quint8 minorScene MEMBER minorScene CONSTANT)
    Q_PROPERTY(quint8 majorScene MEMBER majorScene CONSTANT)

    Q_PROPERTY(quint8 languageOption MEMBER languageOption CONSTANT)

    Q_PROPERTY(QString matchId MEMBER matchId CONSTANT)
    Q_PROPERTY(quint32 gameNumber MEMBER gameNumber CONSTANT)
    Q_PROPERTY(quint32 tiebreakerNumber MEMBER tiebreakerNumber CONSTANT)

public:
    GameInformation(QObject *parent = nullptr);

    QString version;
    quint32 seed = 0;
    quint8 isPal = 0, isFrozenPS = 0, minorScene = 0, majorScene = 0;
    quint8 languageOption = 0;

    QString matchId;
    quint32 gameNumber = 0, tiebreakerNumber = 0;

    PlayerInformation *player1() { return players[0].data(); }
    PlayerInformation *player2() { return players[1].data(); }
    PlayerInformation *player3() { return players[2].data(); }
    PlayerInformation *player4() { return players[3].data(); }

    QScopedPointer<PlayerInformation> players[NUM_PLAYERS];
};

class EventParser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected MEMBER m_connected NOTIFY connectedChanged)
    Q_PROPERTY(QString slippiNick MEMBER m_nick NOTIFY connectedChanged)
    Q_PROPERTY(QString slippiVersion MEMBER m_version NOTIFY connectedChanged)

    Q_PROPERTY(bool gameRunning MEMBER m_gameRunning NOTIFY gameRunningChanged)

    Q_PROPERTY(GameInformation *gameInfo READ gameInfo NOTIFY gameInfoChanged)

    // events from: https://github.com/project-slippi/slippi-wiki/blob/master/SPEC.md#events
    enum SlippiEvents {
        EVENT_SPLIT_MSG     = 0x10,
        EVENT_PAYLOADS      = 0x35,
        EVENT_GAME_START    = 0x36,
        EVENT_PRE_FRAME     = 0x37,
        EVENT_POST_FRAME    = 0x38,
        EVENT_GAME_END      = 0x39,
        EVENT_FRAME_START   = 0x3A,
        EVENT_ITEM_UPDATE   = 0x3B,
        EVENT_FRAME_BOOKEND = 0x3c,
        EVENT_GECKO_LIST    = 0x3D,
        EVENT_UNKNOWN       = 0x45,
        EVENT_HIGHEST       = EVENT_UNKNOWN
    };
public:
    explicit EventParser(QObject *parent = nullptr);

    Q_INVOKABLE void parseSlippiMessage(const QVariantMap &event);
    Q_INVOKABLE void disconnnect();

    GameInformation *gameInfo() const;

    enum GameEndMethod {
        Unresolved = 0, Resolved = 3,
        Time = 1, Game = 2, NoContext = 7
    };
    Q_ENUM(GameEndMethod);

signals:
    void connectedChanged();
    void gameInfoChanged();

    void gameRunningChanged();
    void gameStarted();
    void gameEnded(EventParser::GameEndMethod endMethod, int lrasPlayer, QList<int> playerPlacements);

private:
    void parseGameEvent(int cursor, int nextCursor, const QByteArray &payload);

    void parsePayloadSizes();
    bool parseCommand();
    bool parseGameStart();

    bool parsePreFrame();
    bool parsePostFrame();

    bool parseGameEnd();
    void resetGameState();

    QString m_nick;
    QString m_version;

    bool m_connected;
    bool m_gameRunning;

    int m_currentCursor = -1, m_nextCursor = -1;

    bool m_hasPayloadSizes = false;
    quint16 m_payloadSizes[EVENT_HIGHEST + 1] = { 0 };

    quint8 m_currentCommandByte = 0;

    QByteArray m_dataBuffer;
    QDataStream m_dataStream, m_writeStream;
    uint m_availableBytes = 0;

    QByteArray m_commandData;

    QScopedPointer<GameInformation> m_gameInfo;
};

#endif // EVENTPARSER_H
