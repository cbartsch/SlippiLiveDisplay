#ifndef EVENTPARSER_H
#define EVENTPARSER_H

#include <QObject>
#include <QBuffer>
#include <QMap>
#include <QVariant>
#include <QQmlListProperty>

const int NUM_PLAYERS = 4;

struct PlayerInformation {
    Q_GADGET
    Q_PROPERTY(quint32 dashbackFix MEMBER dashbackFix)
    Q_PROPERTY(quint32 shieldDropFix MEMBER shieldDropFix)
    Q_PROPERTY(quint8 playerType MEMBER playerType)
    Q_PROPERTY(quint8 charId MEMBER charId)

    Q_PROPERTY(QString nameTag MEMBER nameTag)
    Q_PROPERTY(QString slippiCode MEMBER slippiCode)
    Q_PROPERTY(QString slippiName MEMBER slippiName)
    Q_PROPERTY(QString slippiUid MEMBER slippiUid)

public:
    enum ControllerFixType : quint32 { Off = 0, UCF = 1, Dween = 2 };
    Q_ENUM(ControllerFixType);

    enum PlayerType : quint8 { Human = 0, CPU = 1, Demo = 2, Empty = 3};
    Q_ENUM(PlayerType);

    quint32 dashbackFix = Off, shieldDropFix = Off;
    quint8 charId = 0, playerType = Empty;
    QString nameTag, slippiCode, slippiName, slippiUid;
};
Q_DECLARE_METATYPE(PlayerInformation);


struct GameInformation {
    Q_GADGET
    Q_PROPERTY(QString version MEMBER version)
    Q_PROPERTY(quint32 randomSeed MEMBER seed)

    Q_PROPERTY(PlayerInformation player1 READ player1)
    Q_PROPERTY(PlayerInformation player2 READ player2)
    Q_PROPERTY(PlayerInformation player3 READ player3)
    Q_PROPERTY(PlayerInformation player4 READ player4)

    Q_PROPERTY(quint8 isPal MEMBER isPal)
    Q_PROPERTY(quint8 isFrozenPS MEMBER isFrozenPS)
    Q_PROPERTY(quint8 minorScene MEMBER minorScene)
    Q_PROPERTY(quint8 majorScene MEMBER majorScene)

    Q_PROPERTY(quint8 languageOption MEMBER languageOption)
    Q_PROPERTY(QString matchId MEMBER matchId)

public:
    QString version;
    quint32 seed = 0;
    quint8 isPal = 0, isFrozenPS = 0, minorScene = 0, majorScene = 0;
    quint8 languageOption = 0;

    QString matchId;
    quint32 gameNumber = 0, tiebreakerNumber = 0;

    PlayerInformation &player1() { return players[0]; }
    PlayerInformation &player2() { return players[1]; }
    PlayerInformation &player3() { return players[2]; }
    PlayerInformation &player4() { return players[3]; }

    PlayerInformation players[NUM_PLAYERS];
};

class EventParser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected MEMBER m_connected NOTIFY connectedChanged)
    Q_PROPERTY(QString slippiNick MEMBER m_nick NOTIFY connectedChanged)
    Q_PROPERTY(QString slippiVersion MEMBER m_version NOTIFY connectedChanged)

    Q_PROPERTY(bool gameRunning MEMBER m_gameRunning NOTIFY gameRunningChanged)

    Q_PROPERTY(GameInformation gameInfo READ gameInfo NOTIFY gameInfoChanged CONSTANT)

    enum SlippiEvents {
        EVENT_PAYLOADS      = 0x35,
        EVENT_GAME_START    = 0x36,
        EVENT_PRE_FRAME     = 0x37,
        EVENT_POST_FRAME    = 0x38,
        EVENT_GAME_END      = 0x39,
        EVENT_FRAME_START   = 0x3A,
        EVENT_ITEM_UPDATE   = 0x3B,
        EVENT_FRAME_BOOKEND = 0x3c,
        EVENT_GECKO_LIST    = 0x3D,
        EVENT_SPLIT_MSG     = 0x10,
        EVENT_HIGHEST       = EVENT_GECKO_LIST
    };

public:
    explicit EventParser(QObject *parent = nullptr);

    Q_INVOKABLE void parseSlippiMessage(const QVariantMap &event);

    GameInformation gameInfo() const;

signals:
    void connectedChanged();
    void gameRunningChanged();
    void gameInfoChanged();

private:
    void parseGameEvent(int cursor, int nextCursor, const QByteArray &payload);
    void parsePayloadSizes();
    bool parseCommand();
    bool parseGameStart();
    void resetGameState();

    QString m_nick;
    QString m_version;

    bool m_connected;
    bool m_gameRunning;

    int m_currentCursor = -1, m_nextCursor = -1;

    bool m_hasPayloadSizes = false;
    quint16 m_payloadSizes[EVENT_HIGHEST + 1] = {0};

    quint8 m_currentCommandByte = 0;

    QByteArray m_dataBuffer;
    QDataStream m_dataStream, m_writeStream;
    uint m_availableBytes = 0;

    QByteArray m_commandData;

    GameInformation m_gameInfo;
};

#endif // EVENTPARSER_H
