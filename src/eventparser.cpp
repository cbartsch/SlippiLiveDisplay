#include "eventparser.h"

#include <QVariant>
#include <QTextCodec>
#include <QtEndian>
#include <QVector2D>

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
    else {
        qWarning() << "EventParser: Unknown message type" << type;
    }
}

void EventParser::disconnnect()
{
    if(!m_connected) {
        qWarning() << "EventParser: not connected, cannot disconnect.";
        return;
    }

    resetGameState();

    m_connected = false;
    emit connectedChanged();
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

    if(payload[0] == EVENT_PAYLOADS) {
        parsePayloadSizes();
    }
    else {
        //qWarning() << "Did not get payload sizes command byte:" << QString::number(payload[0], 16);
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
        //qDebug() << "Parse command" << QString::number(m_currentCommandByte, 16) << "with" << m_commandData.size() << "bytes payload.";
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
        parsePreFrame();
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

    m_gameInfo.reset(new GameInformation(this));
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

bool EventParser::parsePreFrame()
{
    PreFrameData d(m_commandData);
    PlayerInformation &player = *m_gameInfo->players[d.playerIndex];
    player.preFrame = d;

    player.analyzeFrame();

    return true;
}

bool EventParser::parsePostFrame()
{
    PostFrameData d(m_commandData);
    PlayerInformation &player = *m_gameInfo->players[d.playerIndex];
    player.postFrame = d;

    player.analyzeFrame();

    return true;
}

bool PlayerInformation::analyzeFrame()
{
    if(preFrame.isEmpty || postFrame.isEmpty) {
        return false;
    }

    bool analogTriggerHeld = preFrame.processedButtons.anyTrigger || preFrame.processedButtons.physicalButtons.z;
    bool isLCancel = analogTriggerHeld;
    if(isLCancel && !this->isLCancel) {
        framesSinceLCancel = 0;
    }

    this->isLCancel = isLCancel;

    if(isLCancel || framesSinceLCancel > 0) {
        framesSinceLCancel++;
        emit lCancelFramesChanged();
    }

    // CliffWait - get 30 intangibility frames
    if(postFrame.actionStateId == 253 && intangibilityFrames == 0) {
        intangibilityFrames = 31;
    }

    if(intangibilityFrames > 0) {
        intangibilityFrames--;
        emit intangibilityFramesChanged();
    }

    bool falling = postFrame.airborne && postFrame.ySpeedSelf < 0;

    if(falling != isFalling) {
        framesSinceFall = 0;
        isFalling = falling;
    }

    // note: the fastFalling flag is true on the frame after inputting fast fall
    // thus increment the frames afterwards so frame 1 does not output frame 2
    if(postFrame.isFastFalling != isFastFalling) {
        isFastFalling = postFrame.isFastFalling;
        emit isFastFallingChanged();
    }

    if(falling) {
        framesSinceFall++;
        emit framesSinceFallChanged();
    }

    // LandingFallSpecial - landing lag after free fall or airdodge
    if(postFrame.actionStateId == 43 && postFrame.actionStateFrameCounter == 0) {
        // first frame of LandingFallSpecial
        QVector2D speedVector(postFrame.xSpeedSelfGround, postFrame.ySpeedSelf);
        qreal wdTiming = qLn(speedVector.length() / 3.1) / qLn(0.9);
        qreal fractionalPart = qAbs(wdTiming - qRound(wdTiming));

        if(fractionalPart > 0.001) {
            // not a wavedash if the speed isn't directly influenced by the airdodge (3.1 * 0.9 ^ nFrames)
            // TODO implement a better detection for this
            setWavedash(0, 0);
            qDebug() << "Not a wavedash:" << wdTiming << fractionalPart;
        }
        else {
            float angle = M_PI + qAtan2(postFrame.ySpeedSelf, postFrame.xSpeedSelfGround);
            if(angle > M_PI_2) {
                angle = M_PI - angle;
            }

            setWavedash(qRound(wdTiming), angle * 180 / M_PI);
        }
    }
    else {
        setWavedash(0, 0);
    }

    // Luigi aerial down B
    if(charId == 7 && postFrame.actionStateId == 0x166) {
        bool isBPress = preFrame.physicalButtons.b && !preFramePrev.physicalButtons.b;
        float ySpeedDiff = postFramePrev.ySpeedSelf - postFrame.ySpeedSelf;

        // B pressed + vertical speed increased -> press during mash window
        if(isBPress && ySpeedDiff) {
            setCycloneBPresses(cycloneBPresses + 1);
        }
    }
    else {
        setCycloneBPresses(0);
    }

    setComboCount(postFrame.comboCount);
    setLCancelState(PlayerInformation::LCancelState(postFrame.lCancelStatus));

    preFramePrev = preFrame;
    postFramePrev = postFrame;

    preFrame = {};
    postFrame = {};

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
    m_gameInfo.reset(nullptr);

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

void PlayerInformation::setLCancelState(const LCancelState &newLCancelState)
{
    if (lCancelState == newLCancelState)
        return;

    lCancelState = newLCancelState;
    emit lCancelStateChanged();
}

void PlayerInformation::setWavedash(int frame, qreal angle)
{
    if(frame == wavedashFrame && angle == wavedashAngle)
        return;

    wavedashFrame = frame;
    wavedashAngle = angle;
    emit wavedashChanged();
}

void PlayerInformation::setCycloneBPresses(int bPresses)
{
    if(cycloneBPresses == bPresses)
        return;

    cycloneBPresses = bPresses;
    emit cycloneBPressesChanged();
}
