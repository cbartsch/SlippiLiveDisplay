#include "dolphinconnection.h"

#include <QNetworkDatagram>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

class DolphinConnectionPrivate : public QObject {
    Q_OBJECT

public:
    DolphinConnectionPrivate(DolphinConnection *item);
    ~DolphinConnectionPrivate();

public slots:
    void connect(const QString &hostname, quint16 port);
    void receive();

private:
    DolphinConnection *m_item;
    ENetHost *m_client;
    bool m_running = false;
};

#include "dolphinconnection.moc"

DolphinConnection::DolphinConnection(QObject *parent)
    : QObject{parent}, d(new DolphinConnectionPrivate(this))
{
    d->moveToThread(&m_connectionThread);
    m_connectionThread.start();

    QMetaObject::invokeMethod(d, "connect", Q_ARG(QString, m_hostAddress), Q_ARG(quint16, m_port));
}

DolphinConnection::~DolphinConnection()
{
    m_connectionThread.quit();
    m_connectionThread.wait();

    delete d;
}

DolphinConnectionPrivate::DolphinConnectionPrivate(DolphinConnection *item) : m_item(item) {
    m_client = enet_host_create(nullptr, 1, 3, 0, 0);

    if (m_client == nullptr)
    {
        qWarning() << "An error occurred while trying to create an ENet client host.";
        return;
    }
}

DolphinConnectionPrivate::~DolphinConnectionPrivate() {
    if(m_client) {
        enet_host_destroy(m_client);
        m_client = nullptr;
    }
}

void DolphinConnectionPrivate::connect(const QString &hostname, quint16 port) {
    ENetAddress address;
    ENetEvent event;
    ENetPeer *peer;

    enet_address_set_host(&address, hostname.toLocal8Bit().data());
    address.port = port;
    peer = enet_host_connect(m_client, &address, 3, 0);

    if (peer == nullptr)
    {
        qWarning() << "No available peers for initiating an ENet connection.";
        return;
    }
    if (enet_host_service(m_client, &event, 5000) > 0)
    {
        switch(event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            qDebug() << "Connected to:" << event.peer->channelCount;
            QMetaObject::invokeMethod(m_item, "setConnected", Q_ARG(bool, true));
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            qDebug() << "Could not connect to Dolphin.";
            break;
        default:
            qDebug() << "Unknown event:" << event.type;
        }
    }
    else {
        qDebug() << "Not connected to Dolphin after 5s. Try again.";

        enet_peer_disconnect(peer, 0);

        QMetaObject::invokeMethod(this, "connect", Qt::QueuedConnection,
                                  Q_ARG(QString, m_item->m_hostAddress),
                                  Q_ARG(quint16, m_item->m_port));
        return;
    }

    QString data = "{\"type\": \"connect_request\", \"cursor\": 0}\0";

    qDebug().noquote() << "Send:" << data.toLocal8Bit();

    ENetPacket * packet = enet_packet_create(data.toLocal8Bit().constData(),
                                             data.toLocal8Bit().size(),
                                             ENET_PACKET_FLAG_RELIABLE);

    if(enet_peer_send(peer, 0, packet) != 0) {
        qWarning() << "Could not set connect request";
        return;
    }

    m_running = true;
    QMetaObject::invokeMethod(this, "receive", Qt::QueuedConnection);
}

void DolphinConnectionPrivate::receive() {
    if(m_running) {
        QMetaObject::invokeMethod(this, "receive", Qt::QueuedConnection);
    }
    else {
        return;
    }

    ENetEvent event;

    // Begin receiving packets
    auto ret = enet_host_service(m_client, &event, 500);

    if(ret <= 0) {
        // timeout or other error
        return;
    }

    switch(event.type) {
    case ENET_EVENT_TYPE_DISCONNECT:
        qDebug() << "Disconnected from Dolphin.";

        enet_peer_disconnect(event.peer, 0);

        QMetaObject::invokeMethod(m_item, "setConnected", Q_ARG(bool, false));
        QMetaObject::invokeMethod(this, "connect", Q_ARG(QString, m_item->m_hostAddress), Q_ARG(quint16, m_item->m_port));
        break;
    case ENET_EVENT_TYPE_RECEIVE: {
        QByteArray data((const char*)event.packet->data, event.packet->dataLength);
        QJsonParseError jsonError;
        auto json = QJsonDocument::fromJson(data, &jsonError).object();
        emit m_item->messageReceived(json.toVariantMap());

        enet_packet_destroy (event.packet);

        break;
    }
    default:
        qDebug() << "Unknown event:" << event.type;
        break;
    }
}

void DolphinConnection::setConnected(bool newConnected)
{
    if (m_connected == newConnected)
        return;
    m_connected = newConnected;
    emit connectedChanged();
}
