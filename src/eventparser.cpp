#include "eventparser.h"

#include <QVariant>
#include <QTextCodec>
#include <QtEndian>

EventParser::EventParser(QObject *parent) : QObject{parent},
    m_dataStream(&m_dataBuffer, QIODevice::OpenModeFlag::ReadOnly),
    m_writeStream(&m_dataBuffer, QIODevice::OpenModeFlag::WriteOnly)
{
    m_dataStream.setByteOrder(QDataStream::ByteOrder::BigEndian);
    m_dataStream.setFloatingPointPrecision(QDataStream::FloatingPointPrecision::SinglePrecision);

    // add default values. current version of mainline beta only sends sizes for the first game of the session.
    m_payloadSizes[0x10] = 516;
    m_payloadSizes[0x36] = 760;
    m_payloadSizes[0x37] = 64;
    m_payloadSizes[0x38] = 84;
    m_payloadSizes[0x39] = 6;
    m_payloadSizes[0x3a] = 12;
    m_payloadSizes[0x3b] = 44;
    m_payloadSizes[0x3c] = 8;
    m_payloadSizes[0x3d] = 54480;

    m_payloadSizes[0x45] = 36; // some unknown command it sends if connecting to a running game

    resetGameState();
}

void EventParser::parseSlippiMessage(const QVariantMap &event)
{
    QString type = event["type"].toString();

    if(type == "connect_reply") {
        m_nick = event["nick"].toString();
        m_version = event["version"].toString();
        m_connected = true;
        emit connectedChanged();
    }
    else if(type == "start_game") {
        m_currentCursor = event["cursor"].toInt();
        m_nextCursor = event["next_cursor"].toInt();
    }
    else if(type == "game_event") {
        int cursor = event["cursor"].toInt();
        int nextCursor = event["next_cursor"].toInt();
        QString payloadBase64 = event["payload"].toString();
        QByteArray payload = QByteArray::fromBase64(payloadBase64.toLocal8Bit());

        parseGameEvent(cursor, nextCursor, payload);

        m_currentCursor = cursor;
        m_nextCursor = nextCursor;
    }
    else if(type == "end_game") {
        resetGameState();
    }
}

void EventParser::parseGameEvent(int cursor, int nextCursor, const QByteArray &payload)
{
    // Game events specification: https://github.com/project-slippi/slippi-wiki/blob/master/SPEC.md

    if(cursor != m_nextCursor) {
        qWarning() << "Game event cursors do not match: expected" << m_nextCursor << ", got" << cursor;
        return;
    }

    m_writeStream.writeRawData(payload.constData(), payload.size());
    m_availableBytes += payload.size();

    //qDebug() << "Feed" << payload.size() << "bytes, available now:" << m_availableBytes;

    if(!m_hasPayloadSizes) {
        if(payload[0] == EVENT_PAYLOADS) {
            parsePayloadSizes();
        }
        else {
            //qWarning() << "Did not get payload sizes command byte:" << QString::number(payload[0], 16);
        }
    }

    bool moreEvents = true;
    while(moreEvents) {
        if(m_currentCommandByte == 0 && m_availableBytes > 0) {
            quint8 nextCommandByte;
            m_dataStream >> nextCommandByte;
            m_currentCommandByte = nextCommandByte;
            m_availableBytes--;

            //qDebug() << "Next command byte:" << QString::number(m_currentCommandByte, 16);
        }
        else {
//            qDebug() << "Not reading command byte, currently:" << QString::number(m_currentCommandByte, 16) << m_availableBytes
//                     << ", would be:" << QString::number(payload[0], 16);
        }

        moreEvents = parseCommand();
    }
}

void EventParser::parsePayloadSizes()
{
    qint8 payloadCommandByte;
    quint8 payloadSizeLength;
    m_dataStream >> payloadCommandByte >> payloadSizeLength;

    //qDebug() << "Parse payload sizes, command size:" << payloadSizeLength << ", command byte:" << QString::number(payloadCommandByte, 16);

    for(int i = 1; i < payloadSizeLength; i+=3) {
        quint8 commandByte;
        quint16 payloadSize;
        m_dataStream >> commandByte >> payloadSize;

        //qDebug() << "Size for command" << QString::number(commandByte, 16) << "=" << payloadSize;

        if(commandByte < 0 || commandByte > EVENT_HIGHEST) {
            qWarning() << "Unknown commandByte" << QString::number(commandByte, 16) << "in payload size event.";
            continue;
        }

        m_payloadSizes[commandByte] = payloadSize;
    }

    m_availableBytes -= payloadSizeLength + 1; // payload size + command byte
    m_hasPayloadSizes = true;
}

bool EventParser::parseCommand()
{
    if(m_currentCommandByte == 0) {
        // no command byte received yet
        //qDebug() << "No command byte received yet.";
        return false;
    }

    uint commandSize = m_currentCommandByte > EVENT_HIGHEST ? 0 : m_payloadSizes[m_currentCommandByte];

    if(m_availableBytes < commandSize) {
        // command not yet received fully
        qDebug() << "Command" << QString::number(m_currentCommandByte, 16) << "needs" << commandSize << "bytes, but only received" << m_availableBytes << "yet.";
        return false;
    }

    if(m_currentCommandByte > EVENT_HIGHEST) {
        qDebug() << "Skip unknown command:" << QString::number(m_currentCommandByte, 16) << "with payload size" << m_availableBytes;
        m_currentCommandByte = 0;
        m_dataStream.skipRawData(m_availableBytes);
        m_availableBytes = 0;
        return false;
    }

    char *bytes = new char[commandSize];
    m_dataStream.readRawData(bytes, commandSize);
    m_availableBytes -= commandSize;

    if(m_currentCommandByte == EVENT_SPLIT_MSG) {
        quint8 internalCommand = bytes[commandSize - 4];
        quint16 actualSize = *((quint16*)&bytes[commandSize - 3]);
        bool lastMessage = bytes[commandSize - 1];

        commandSize -= 4;

        //qDebug() << "Reading a split message for command" << QString::number(internalCommand, 16) << ", actual size:" << actualSize << "is last:" << lastMessage;
        Q_UNUSED(actualSize) // is always 512, or less for the last message

        if(lastMessage) {
            m_currentCommandByte = internalCommand;
        }
    }

    m_commandData.append(QByteArray::fromRawData(bytes, commandSize));

    if(m_currentCommandByte != EVENT_SPLIT_MSG) {
    //    qDebug() << "Parse command" << QString::number(m_currentCommandByte, 16) << "with" << m_commandData.size() << "bytes payload.";
    }

    // can now fully read the command
    switch(m_currentCommandByte) {
    case EVENT_SPLIT_MSG:
        // do nothing
        break;
    case EVENT_GAME_START:
        parseGameStart();
        break;
    case EVENT_PRE_FRAME:
        break;
    case EVENT_POST_FRAME:
        parsePostFrame();
        break;
    case EVENT_FRAME_START:
        break;
    case EVENT_ITEM_UPDATE:
        break;
    case EVENT_FRAME_BOOKEND:
        break;
    case EVENT_GECKO_LIST:
        break;
    case EVENT_GAME_END:
        parseGameEnd();
        break;
    default:
        qWarning() << "Command" << QString::number(m_currentCommandByte, 16) << "not implemented.";
        break;
    }

    if(m_currentCommandByte != EVENT_SPLIT_MSG) {
        // done reading current command
        m_commandData.clear();
    }
    m_currentCommandByte = 0;
    delete[] bytes;

    return true;
}

bool EventParser::parseGameStart()
{
    QDataStream stream(m_commandData);

    quint8 version[4];
    stream.readRawData((char*)&version, 4);

    GameInformation &gi = *m_gameInfo;

    gi.version = QString("%1.%2.%3 (%4)").arg(version[0]).arg(version[1]).arg(version[2]).arg(version[3]);

    char gameInfoBlock[312];
    stream.readRawData(gameInfoBlock, 312);

    QDataStream gameInfoStream(QByteArray(gameInfoBlock, 312));
    gameInfoStream.skipRawData(0x60);

    for(auto &player: gi.players) {
        gameInfoStream >> player->charId;
        gameInfoStream >> player->playerType;

        gameInfoStream.skipRawData(0x22);
    }

    stream >> gi.seed;

    for(auto &player: gi.players) {
        stream >> player->dashbackFix;
    }

    for(auto &player: gi.players) {
        stream >> player->shieldDropFix;
    }

    QTextCodec *jisCodec = QTextCodec::codecForName("Shift-JIS");

    if(jisCodec == nullptr) {
        qWarning() << "Could not find Shift-JIS codec.";
        jisCodec = QTextCodec::codecForLocale();
    }

    for(auto &player: gi.players) {
        char rawTag[16];
        stream.readRawData(rawTag, 16);
        player->nameTag = jisCodec->toUnicode(rawTag);
    }

    stream >> gi.isPal >> gi.isFrozenPS >> gi.minorScene >> gi.majorScene;

    for(auto &player: gi.players) {
        char rawName[31];
        stream.readRawData(rawName, 31);
        player->slippiName = jisCodec->toUnicode(rawName);
    }

    for(auto &player: gi.players) {
        char rawCode[10];
        stream.readRawData(rawCode, 10);
        QString codeStr = jisCodec->toUnicode(rawCode);

        // replace full-width (Unicode 0xff03) hash with regular (half-width) hash
        player->slippiCode = codeStr.replace(QChar(0xff03), '#');
    }

    for(auto &player: gi.players) {
        char rawUid[29];
        stream.readRawData(rawUid, 29);
        player->slippiUid = QString::fromUtf8(rawUid);
    }

    stream >> gi.languageOption;

    char rawMatchId[51];
    stream.readRawData(rawMatchId, 51);
    gi.matchId = QString::fromUtf8(rawMatchId);

    stream >> gi.gameNumber >> gi.tiebreakerNumber;

    m_gameRunning = true;
    emit gameInfoChanged();
    emit gameRunningChanged();
    emit gameStarted();

    return true;
}

bool EventParser::parsePostFrame()
{

// from: https://github.com/project-slippi/slippi-wiki/blob/master/SPEC.md#post-frame-update
    struct PostFrameData {
        PostFrameData(const QByteArray &data) {

            QDataStream stream(data);
            stream.setByteOrder(QDataStream::ByteOrder::BigEndian);
            stream.setFloatingPointPrecision(QDataStream::FloatingPointPrecision::SinglePrecision);

            stream >> frameNumber >> playerIndex >> isFollower >> charId >> actionStateId
                >> posX >> posY >> facingDirection >> percent >> shieldSize
                >> lastHitAttackId >> comboCount >> lastHitBy >> stocks >> actionStateFrameCounter
                >> stateBitFlag1 >> stateBitFlag2 >> stateBitFlag3 >> stateBitFlag4 >> stateBitFlag5
                >> actionStateData >> airborne >> lastGroundId
                >> jumpsRemaining >> lCancelStatus >> hurtboxCollisionState
                >> xSpeedSelfAir >> ySpeedSelf >> xSpeedAttack >> ySpeedAttack >> xSpeedSelfGround
                >> hitlagFrameRemaining >> animationIndex;

        }

        qint32 frameNumber;
        quint8 playerIndex;
        bool isFollower;
        quint8 charId;
        quint16 actionStateId;
        float posX, posY, facingDirection, percent, shieldSize;
        quint8 lastHitAttackId, comboCount, lastHitBy, stocks;
        float actionStateFrameCounter;

        union { struct { quint8 : 4; bool isReflectActive : 1;  quint8 : 3;                                                                                 }; quint8 stateBitFlag1; };
        union { struct { quint8 : 2; bool hasIntangibility : 1; bool isFastFalling : 1;    quint8 : 1; bool isInHitlag: 1;          quint8 : 2;             }; quint8 stateBitFlag2; };
        union { struct { quint8 : 7; bool isShieldActive : 1;                                                                                               }; quint8 stateBitFlag3; };
        union { struct { quint8 : 2; bool isInHitStun : 1;      bool isTouchingShield : 1; quint8 : 2; bool isPowershieldActive : 1; quint8 : 2;            }; quint8 stateBitFlag4; };
        union { struct { quint8 : 3; bool isFollowerBit : 1;    bool isSleeping: 1;        quint8 : 1; bool isDead: 1;               bool isOffscreen : 1;  }; quint8 stateBitFlag5; };

        float actionStateData; // eg. hitstun remaining
        bool airborne;
        quint16 lastGroundId;
        quint8 jumpsRemaining, lCancelStatus, hurtboxCollisionState;
        float xSpeedSelfAir, ySpeedSelf, xSpeedAttack, ySpeedAttack, xSpeedSelfGround, hitlagFrameRemaining;
        quint32 animationIndex;
    } d(m_commandData);

    PlayerInformation &player = *m_gameInfo->players[d.playerIndex];

    player.setComboCount(d.comboCount);

    return true;
}

bool EventParser::parseGameEnd()
{
    QDataStream stream(m_commandData);

    quint8 gameEndMethod;
    qint8 lrasPlayerIndex;
    QList<int> playerPlacements;
    stream >> gameEndMethod >> lrasPlayerIndex;

    for(int i = 0; i < NUM_PLAYERS; i++) {
        qint8 placement;
        stream >> placement;
        playerPlacements << placement;
    }

    emit gameEnded(GameEndMethod(gameEndMethod), lrasPlayerIndex, playerPlacements);

    return true;
}

void EventParser::resetGameState()
{
    // reset state for next game:
    //m_hasPayloadSizes = false;
    m_dataBuffer.clear();
    m_dataStream.device()->reset();
    m_writeStream.device()->reset();
    m_availableBytes = 0;
    m_currentCommandByte = 0;
    m_gameInfo.reset(new GameInformation(this));

    m_gameRunning = false;
    emit gameInfoChanged();
    emit gameRunningChanged();
}

GameInformation *EventParser::gameInfo() const
{
    return m_gameInfo.data();
}

GameInformation::GameInformation(QObject *parent) : QObject(parent) {
    for(int i = 0; i < NUM_PLAYERS; i++) {
        players[i].reset(new PlayerInformation(this));
    }
}

PlayerInformation::PlayerInformation(QObject *parent) : QObject(parent) {

}

void PlayerInformation::setComboCount(quint32 newComboCount)
{
    if (comboCount == newComboCount)
        return;
    comboCount = newComboCount;
    emit comboCountChanged();
}
