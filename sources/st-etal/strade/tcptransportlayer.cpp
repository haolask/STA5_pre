#include "tcptransportlayer.h"

TcpTransportLayer::TcpTransportLayer(QObject *parent, QString address, qint16 port):
        QObject(parent), tcpLayerAddress(address), tcpLayerPort(port), blockSize(0)
{
    // Initialize variables
    rxBuffer.clear();
    isRxPacketBegin = true;
    receivedSize = 0;

    // Create TCP socket object
    tcpSocket = new QTcpSocket();

    // Do connections
    QObject::connect(tcpSocket,
                     &QIODevice::readyRead,
                     this,
                     static_cast<void (TcpTransportLayer::*)()>(&TcpTransportLayer::tcpLayerDataReceived_slot));

    QObject::connect(tcpSocket,
                     static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
                     this,
                     static_cast<QAbstractSocket::SocketError (TcpTransportLayer::*)(QAbstractSocket::SocketError)>(&TcpTransportLayer::tcpLayerGetError_slot));

    QObject::connect(tcpSocket,
                     static_cast<void (QTcpSocket::*)()>(&QAbstractSocket::connected),
                     this,
                     static_cast<void (TcpTransportLayer::*)()>(&TcpTransportLayer::tcpLayerConnected_slot));

    QObject::connect(tcpSocket,
                     static_cast<void (QTcpSocket::*)()>(&QAbstractSocket::disconnected),
                     this,
                     static_cast<void (TcpTransportLayer::*)()>(&TcpTransportLayer::tcpLayerDisconnected_slot));
}

TcpTransportLayer::~TcpTransportLayer()
{
    tcpSocket->disconnectFromHost();

    if (tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        tcpSocket->waitForDisconnected(TCP_PROTOCOL_LAYER_CONNECTION_TIMEOUT);
    }

    if (true == tcpSocket->isOpen())
    {
        tcpSocket->close();
    }
}

void TcpTransportLayer::tcpLayerConnected_slot()
{
    qDebug() << "TCP/IP connected slot called";
}

void TcpTransportLayer::tcpLayerDisconnected_slot()
{
    qDebug() << "TCP/IP disconnected slot called";
}

bool  TcpTransportLayer::openTcpLayerSocket() const
{
    return tcpSocket->open(QIODevice::ReadWrite);
}

bool TcpTransportLayer::OpenTcpLayerSocket()
{
    if (tcpSocket->isOpen())
    {
        qDebug() << "TCP/IP: tcpSocket already open";

        return true;
    }
    else
    {
        if(tcpSocket->open(QIODevice::ReadWrite))
        {
            qDebug() << "TCP/IP: tcpSocket open successfully";

            return true;
        }
    }

    return false;
}

void TcpTransportLayer::closeTcpLayerSocket() const
{
    if (tcpSocket->isOpen())
    {
        qDebug() << "closeTcpLayerSocket";

        tcpSocket->close();
    }
}

void TcpTransportLayer::tcpLayerDataReceived_slot()
{
    readIncomingData();
}

void TcpTransportLayer::readIncomingData()
{
    int size;

    if (true == isRxPacketBegin)
    {
        rxBuffer.clear();

        rxBuffer = readTcpRawData(size);

        receivedSize = (int)((((unsigned int)rxBuffer[6] << 8) & (unsigned int)0xFF00) |
                             (((unsigned int)rxBuffer[7] << 0) & (unsigned int)0x00FF));

        receivedSize += 8; // Add the size of the communication protocol header

        if (receivedSize <= size) // Size of TCP/IP protocol
        {
            emit tcpLayer_sendDataToDispatcher_signal(rxBuffer);
        }
        else
        {
            receivedSize -= size;
            isRxPacketBegin = false;
        }
    }
    else
    {
        rxBuffer.append(readTcpRawData(size));

        receivedSize -= size;

        if (0 == receivedSize)
        {
            emit tcpLayer_sendDataToDispatcher_signal(rxBuffer);

            isRxPacketBegin = true;
        }
    }
}

bool TcpTransportLayer::waitForData()
{
    qDebug() << "TcpTransportLayer:: waitForData void";

    return false;
}

QAbstractSocket::SocketError TcpTransportLayer::tcpLayerGetError_slot(QAbstractSocket::SocketError error_number)
{
    QString error;

    switch (error_number)
    {
        case QAbstractSocket::ConnectionRefusedError:
            error = "The connection was refused by the peer (or timed out)";
        break;

        case QAbstractSocket::RemoteHostClosedError:
            error = "The remote host closed the connection";
        break;

        case QAbstractSocket::HostNotFoundError:
            error = "The host address was not found";
        break;

        case QAbstractSocket::SocketAccessError:
            error = "The socket operation failed because the application lacked the required privileges";
        break;

        case QAbstractSocket::SocketResourceError:
            error = "The local system ran out of resources (e.g., too many sockets)";
        break;

        case QAbstractSocket::SocketTimeoutError:
            error = "The socket operation timed out";
        break;

        case QAbstractSocket::DatagramTooLargeError:
            error = "The datagram was larger than the operating system's limit";
        break;

        case QAbstractSocket::NetworkError:
            error = "An error occurred with the network";
        break;

        case QAbstractSocket::AddressInUseError:
            error = "The address specified is already in use and was set to be exclusive";
        break;

        case QAbstractSocket::SocketAddressNotAvailableError:
            error = "The address specified does not belong to the host";
        break;

        case QAbstractSocket::UnsupportedSocketOperationError:
            error = "The requested socket operation is not supported by the local operating system";
        break;

        case QAbstractSocket::ProxyAuthenticationRequiredError:
            error = "The socket is using a proxy, and the proxy requires authentication";
        break;

        case QAbstractSocket::SslHandshakeFailedError:
            error = "The SSL/TLS handshake failed, so the connection was closed";
        break;

        case QAbstractSocket::UnfinishedSocketOperationError:
            error = "The last operation attempted has not finished yet";
        break;

        case QAbstractSocket::ProxyConnectionRefusedError:
            error = "Could not contact the proxy server because the connection to that server was denied";
        break;

        case QAbstractSocket::ProxyConnectionClosedError:
            error = "The connection to the proxy server was closed unexpectedly";
        break;

        case QAbstractSocket::ProxyConnectionTimeoutError:
            error = "The connection to the proxy server timed out or the proxy server stopped responding in the authentication phase";
        break;

        case QAbstractSocket::ProxyNotFoundError:
            error = "The proxy address set was not found";
        break;

        case QAbstractSocket::ProxyProtocolError:
            error = "The connection negotiation with the proxy server because the response from the proxy server could not be understood";
        break;

        case QAbstractSocket::UnknownSocketError:
            error = "Unknown socket error";
        break;

        case QAbstractSocket::OperationError:
            error = "Operation error";
        break;

        case QAbstractSocket::SslInternalError:
            error = "SSL internal error";
        break;

        case QAbstractSocket::SslInvalidUserDataError:
            error = "SSL invalid user data error";
        break;

        case QAbstractSocket::TemporaryError:
            error = "Temporary error";
        break;

        default:
            // No code
        break;
    }

    qDebug() << "ERROR: tcpSocket error number:" << error_number;

    qDebug() << "ERROR: tcpSocket error description:" << error;

    return error_number;
}

QString TcpTransportLayer::getError(void)
{
    qDebug() << "TcpTransportLayer:: NOT_USED_METHOD";

    return "NOT_USED_METHOD";
}

qint64 TcpTransportLayer:: GetWrittenBytes()
{
    qDebug() << "TcpTransportLayer:: NOT_USED_METHOD";

    return 0;
}

qint64 TcpTransportLayer::writeTcpRawData(const QByteArray &data, qint16 size)
{
    qint64 written_bytes = 0;

    if (tcpSocket->ConnectedState == QAbstractSocket::ConnectedState)
    {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);

        out.writeRawData(data, size);
        written_bytes = tcpSocket->write(block, size);
    }

    return written_bytes;
}

void TcpTransportLayer::Flush()
{
    tcpSocket->flush();
}

QByteArray TcpTransportLayer::readTcpRawData(int &size)
{
    QByteArray data;
    QDataStream in(tcpSocket);
    qint64 bytesAvailable;

    in.setVersion(QDataStream::Qt_4_0);

    bytesAvailable = tcpSocket->bytesAvailable();

    data.resize(bytesAvailable);
    in.readRawData(data.data(), bytesAvailable);
    size = data.size();

    return data;
}

bool TcpTransportLayer::isOpen()
{
    qDebug() << "TcpTransportLayer:: NOT_USED_METHODE";

    return false;
}

bool TcpTransportLayer::isSocketOpen()
{
    return tcpSocket->isOpen();
}

bool TcpTransportLayer::isValid()
{
    qDebug() << "TcpTransportLayer:: NOT_USED_METHOD";

    return false;
}

bool TcpTransportLayer::isConnected()
{
    if (tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        return true;
    }

    return false;
}

bool TcpTransportLayer::open()
{
    qDebug() << "TcpTransportLayer:: NOT_USED_METHOD";

    return false;
}

bool TcpTransportLayer::reconnect()
{
    qDebug() << "TcpTransportLayer:: NOT_USED_METHOD";

    return false;
}

bool TcpTransportLayer::disconnect()
{
    tcpSocket->disconnectFromHost();

    if (tcpSocket->state() != QAbstractSocket::UnconnectedState)
        tcpSocket->waitForDisconnected(TCP_PROTOCOL_LAYER_CONNECTION_TIMEOUT);

    if (tcpSocket->state() == QAbstractSocket::UnconnectedState)
    {
        qDebug() << "TCP SOCKET disconnected Successfully";

        return true;
    }

    return false;
}

bool TcpTransportLayer::connect()
{
    bool res = true;
    tcpSocket->connectToHost("127.0.0.1", tcpLayerPort, QIODevice::ReadWrite);
    tcpSocket->waitForConnected(TCP_PROTOCOL_LAYER_CONNECTION_TIMEOUT);

    if (tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        qDebug() << "TCP SOCKET connected at Address: " << tcpLayerAddress << " Port: " << tcpLayerPort;
    }
    else
    {
        res = false;
    }

    return res;
}

void TcpTransportLayer::receivedCmdForDevice_slot(QByteArray parsed_cmd)
{
    if (true == isConnected())
    {
        writeTcpRawData(parsed_cmd, parsed_cmd.size());
    }
}
