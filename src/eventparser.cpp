#include "eventparser.h"

#include <QVariant>
#include <QTextCodec>

EventParser::EventParser(QObject *parent) : QObject{parent},
    m_dataStream(&m_dataBuffer, QIODevice::OpenModeFlag::ReadOnly),
    m_writeStream(&m_dataBuffer, QIODevice::OpenModeFlag::WriteOnly)
{
    m_dataStream.setByteOrder(QDataStream::ByteOrder::BigEndian);
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
        m_gameRunning = true;
        emit gameRunningChanged();
    }
    else if(type == "game_event") {
        int cursor = event["cursor"].toInt();
        int nextCursor = event["next_cursor"].toInt();
        QString payloadBase64 = event["payload"].toString();
        QByteArray payload = QByteArray::fromBase64(payloadBase64.toLocal8Bit());

        parseGameEvent(cursor, nextCursor, payload);
    }
    else if(type == "end_game") {
        resetGameState();
    }
}

void EventParser::parseGameEvent(int cursor, int nextCursor, const QByteArray &payload)
{
    if(cursor != m_nextCursor) {
        qWarning() << "Game event cursors do not match: expected" << m_nextCursor << ", got" << cursor;
        return;
    }

    m_currentCursor = cursor;
    m_nextCursor = nextCursor;

    m_writeStream.writeRawData(payload.constData(), payload.size());
    m_availableBytes += payload.size();

    //qDebug() << "Feed" << payload.size() << "bytes, available now:" << m_availableBytes;

    if(!m_hasPayloadSizes) {
        if(payload[0] == EVENT_PAYLOADS) {
            parsePayloadSizes();
        }
        else {
            qWarning() << "Did not get payload sizes command byte:" << QString::number(payload[0], 16);
        }
    }

    bool moreEvents = true;
    while(moreEvents) {
        if(m_currentCommandByte == 0 && m_availableBytes > 0) {
            quint8 nextCommandByte;
            m_dataStream >> nextCommandByte;
            m_currentCommandByte = nextCommandByte;
            m_availableBytes--;
           // qDebug() << "Next command byte:" << QString::number(m_currentCommandByte, 16);
        }
        else {
           // qDebug() << "Not reading command byte, currently:" << m_currentCommandByte << m_availableBytes;
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

    uint commandSize = m_payloadSizes[m_currentCommandByte];

    if(m_availableBytes < commandSize) {
        // command not yet received fully
        qDebug() << "Command" << QString::number(m_currentCommandByte, 16) << "needs" << commandSize << "bytes, but only received" << m_availableBytes << "yet.";
        return false;
    }

    char *bytes = new char[commandSize];
    m_dataStream.readRawData(bytes, commandSize);
    m_availableBytes -= commandSize;

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
        qDebug() << "Game end command.";
        break;
    default:
        qWarning() << "Command" << QString::number(m_currentCommandByte, 16) << "not implemented.";
        m_dataStream.skipRawData(commandSize);
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

    m_gameInfo.version = QString("%1.%2.%3 (%4)").arg(version[0]).arg(version[1]).arg(version[2]).arg(version[3]);

    char gameInfoBlock[312];
    stream.readRawData(gameInfoBlock, 312);

    QDataStream gameInfoStream(QByteArray(gameInfoBlock, 312));
    gameInfoStream.skipRawData(0x60);

    for(int i = 0; i < NUM_PLAYERS; i++) {
        gameInfoStream >> m_gameInfo.players[i].charId;
        gameInfoStream >> m_gameInfo.players[i].playerType;

        gameInfoStream.skipRawData(0x22);
    }

    stream >> m_gameInfo.seed;

    for(int i = 0; i < NUM_PLAYERS; i++) {
        stream >> m_gameInfo.players[i].dashbackFix;
    }

    for(int i = 0; i < NUM_PLAYERS; i++) {
        stream >> m_gameInfo.players[i].shieldDropFix;
    }

    QTextCodec *jisCodec = QTextCodec::codecForName("Shift-JIS");

    if(jisCodec == nullptr) {
        qWarning() << "Could not find Shift-JIS codec.";
        jisCodec = QTextCodec::codecForLocale();
    }

    for(int i = 0; i < NUM_PLAYERS; i++) {
        char rawTag[16];
        stream.readRawData(rawTag, 16);
        m_gameInfo.players[i].nameTag = jisCodec->toUnicode(rawTag);
    }

    stream >> m_gameInfo.isPal >> m_gameInfo.isFrozenPS >> m_gameInfo.minorScene >> m_gameInfo.majorScene;

    for(int i = 0; i < NUM_PLAYERS; i++) {
        char rawName[31];
        stream.readRawData(rawName, 31);
        m_gameInfo.players[i].slippiName = jisCodec->toUnicode(rawName);
    }

    for(int i = 0; i < NUM_PLAYERS; i++) {
        char rawCode[10];
        stream.readRawData(rawCode, 10);
        QString codeStr = jisCodec->toUnicode(rawCode);

        // replace Shift-JIS (Unicode 0xff03) hash with regular hash
        m_gameInfo.players[i].slippiCode = codeStr.replace(QChar(0xff03), '#');
    }

    for(int i = 0; i < NUM_PLAYERS; i++) {
        char rawUid[29];
        stream.readRawData(rawUid, 29);
        m_gameInfo.players[i].slippiUid = QString::fromUtf8(rawUid);
    }

    stream >> m_gameInfo.languageOption;

    char rawMatchId[51];
    stream.readRawData(rawMatchId, 51);
    m_gameInfo.matchId = QString::fromUtf8(rawMatchId);

    stream >> m_gameInfo.gameNumber >> m_gameInfo.tiebreakerNumber;

    emit gameInfoChanged();

    return true;
}

void EventParser::resetGameState()
{
    // reset state for next game:
    m_hasPayloadSizes = false;
    m_dataBuffer.clear();
    m_dataStream.device()->reset();
    m_writeStream.device()->reset();
    m_availableBytes = 0;
    m_currentCommandByte = 0;
    m_gameInfo = {};

    m_gameRunning = false;
    emit gameInfoChanged();
    emit gameRunningChanged();
}

GameInformation EventParser::gameInfo() const
{
    return m_gameInfo;
}
