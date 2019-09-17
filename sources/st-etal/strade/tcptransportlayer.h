#ifndef TCPTRANSPORTLAYER_H
#define TCPTRANSPORTLAYER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QtNetwork>
#include <QByteArray>

#include "common.h"

class TcpTransportLayer : public QObject
{
    Q_OBJECT

    public:
        TcpTransportLayer(QObject *parent, QString address, qint16 port);
        ~TcpTransportLayer();

        QString getError();
        void Flush();
        bool OpenTcpLayerSocket();
        bool connect();
        bool disconnect();
        bool reconnect();

    private:
        bool openTcpLayerSocket() const;
        void closeTcpLayerSocket() const;
        bool waitForData();
        bool open();
        bool isOpen();
        bool isValid();
        bool isConnected();
        bool isSocketOpen();

        qint64 writeTcpRawData(const QByteArray &data, qint16 size);
        QByteArray readTcpRawData(int &size);
        void readIncomingData();
        qint64 GetWrittenBytes();

        QString tcpLayerAddress;
        qint16 tcpLayerPort;
        quint32 blockSize;
        QTcpSocket *tcpSocket;
        qint64 writtenBytes;

        QByteArray rxBuffer;
        bool isRxPacketBegin;
        int receivedSize;

    public slots:
        void receivedCmdForDevice_slot(QByteArray parsed_cmd);
        void tcpLayerDataReceived_slot();
        void tcpLayerDisconnected_slot();
        void tcpLayerConnected_slot();

        QAbstractSocket::SocketError tcpLayerGetError_slot(QAbstractSocket::SocketError error_number);

    signals:
        void tcpLayer_sendDataToDispatcher_signal(QByteArray data);

        void tcpLayerDataSent_signal();
        void tcpLayerTransmitError_signal();
};

#endif // TCPTRANSPORTLAYER_H
