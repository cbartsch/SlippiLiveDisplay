#ifndef DOLPHINCONNECTION_H
#define DOLPHINCONNECTION_H

#include <QObject>
#include <QThread>

#include <enet/enet.h>

class DolphinConnection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected MEMBER m_connected NOTIFY connectedChanged)
public:
    explicit DolphinConnection(QObject *parent = nullptr);
    ~DolphinConnection();


signals:
    void messageReceived(const QVariantMap &message);
    void connectedChanged();

private slots:
    void setConnected(bool newConnected);

private:
    friend class DolphinConnectionPrivate;
    class DolphinConnectionPrivate *d;

    QThread m_connectionThread;
    quint16 m_port = 51441;
    QString m_hostAddress = "localhost";
    bool m_connected;
};

#endif // DOLPHINCONNECTION_H
