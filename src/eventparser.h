#ifndef EVENTPARSER_H
#define EVENTPARSER_H

#include <QObject>
#include <QBuffer>
#include <QMap>
#include <QVariant>

struct GameInformation {
    Q_GADGET
    Q_PROPERTY(QString version MEMBER m_version)

private:
    QString m_version;

    friend class EventParser;
};

class EventParser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected MEMBER m_connected NOTIFY connectedChanged)
    Q_PROPERTY(QString slippiNick MEMBER m_nick NOTIFY connectedChanged)
    Q_PROPERTY(QString slippiVersion MEMBER m_version NOTIFY connectedChanged)

    Q_PROPERTY(bool gameRunning MEMBER m_gameRunning NOTIFY gameRunningChanged)

    Q_PROPERTY(GameInformation gameInfo READ gameInfo NOTIFY gameInfoChanged CONSTANT)

    const int NUM_PLAYERS = 4;

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

    void parseGameEvent(int cursor, int nextCursor, const QByteArray &payload);
    void parsePayloadSizes();
    bool parseCommand();
    bool parseGameStart();

    GameInformation gameInfo() const;

signals:
    void connectedChanged();
    void gameRunningChanged();
    void gameInfoChanged();

private:
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
