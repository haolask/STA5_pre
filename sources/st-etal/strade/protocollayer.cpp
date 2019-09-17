///
//! \file          protocollayer.cpp
//! \brief         Implementation file for interfacing with the external program interfaced usign TCP/IP sockets
//! \author        Alberto Saviotti
//!
//! Project        STRaDe
//! Sw component   HMI
//!
//! Copyright (c)  STMicroelectronics
//!
//! History
//! Date           | Modification               | Author
//! 20180127       | Initial version            | Alberto Saviotti
///

#include <QThread>

#include "protocollayer.h"

//! \brief <i><b> Command string to ask to the 'Protocol Layer' to close itself when no more sockets are connected </b></i>
const QByteArray parseData = QByteArrayLiteral("\x2D\xFE\x01\x61\xA8\x00\x00\x00");

ProtocolLayer::ProtocolLayer(QObject *parent) : QObject(parent)
{
    // Create the Protocol Layer process obj
    protocolLayerProcess = new QProcess(this);

    fileManager = new FilesManager();

    // Create the taskKiller process obj
    taskKillerProcess = new QProcess();

    // Initialize control port to NULL value
    tcpControlPort = NULL;
    isConnected = false;
}

ProtocolLayer::~ProtocolLayer()
{
    // Disconnect from protocol layer and free mem
    if (true == isConnected)
    {
        // Emit signal to close the protocol layer
        emit sendCmdToTcp_signal(parseData);

        // Wait a little bit (100 ms)
        QThread::msleep(100);

        // Disconnect socket
        tcpControlPort->disconnect();
    }

    // Delete members
    delete tcpControlPort;

    delete fileManager;
    delete taskKillerProcess;
}

void ProtocolLayer::TcpRx_slot (QByteArray data)
{
    Q_UNUSED(data);
}

bool ProtocolLayer::SetupControlPort ()
{
    // Check if the control port is already been instantiated
    if (NULL != tcpControlPort)
    {
        return false;
    }

    // Connect to protocol layer external program TCP/IP port
    tcpControlPort = new TcpTransportLayer(this, parameters.ipPortAddress, static_cast <qint16> (parameters.controlPort));

    // Open the dcop tcp socket
    if (false == tcpControlPort->OpenTcpLayerSocket())
    {
        qDebug() << "ERROR: CTRL socket doesn`t open!";

        delete tcpControlPort;
        tcpControlPort = NULL;

        return false;
    }

    // Try to connect the tcpCmostTransportLayer to the protocolLayerProcess
    if (false == tcpControlPort->connect())
    {
        qDebug() << "ERROR: CTRL socket doesn`t connect!";

        delete tcpControlPort;
        tcpControlPort = NULL;

        return false;
    }

    // Do connections
    QObject::connect(tcpControlPort,
                     static_cast <void (TcpTransportLayer::*)(QByteArray)> (&TcpTransportLayer::tcpLayer_sendDataToDispatcher_signal),
                     this,
                     static_cast <void (ProtocolLayer::*)(QByteArray)> (&ProtocolLayer::TcpRx_slot));

    QObject::connect(this,
                     static_cast <void (ProtocolLayer::*)(QByteArray)> (&ProtocolLayer::sendCmdToTcp_signal),
                     tcpControlPort,
                     static_cast <void (TcpTransportLayer::*)(QByteArray)> (&TcpTransportLayer::receivedCmdForDevice_slot));

    isConnected = true;

    return true;
}

bool ProtocolLayer::CheckIfProtocolLayerIsRunning ()
{
    return SetupControlPort();
}

bool ProtocolLayer::ProtocolLayerStart ()
{
    bool res = false;

    // Get parameters (mainly ports to open)
    parameters = GetParameters();

    // Since we just start Protocol Layer in case it is auto-start we do nothing
    // Create the taskKiller command string
    QString taskKillerProcessString = QString(TOOLS_FOLDER_NAME) + QString(TASK_KILLER_FILE_NAME) + "  /IM " + PROTOCOL_LAYER_FILE_NAME;

    // Kill all instances of MDR_Protocol.exe file
    taskKillerProcess->start(taskKillerProcessString);
    taskKillerProcess->waitForFinished();

    // Check if protocol layer is already running trying to connect to control port
    if (false == CheckIfProtocolLayerIsRunning())
    {
        // Close ProtocolLayerProcess (if it is open)
        if (false == CloseProtocolLayerProcess())
        {
            qDebug() << "---The ProtocolLayerProcess doesn`t close!";
        }

        // Make ProtocolLayerProcess CommandLine
        QString protocol_layer_command_line = MakeProtocolLayerCommandLine();

        // Start ProtocolLayerProcess (no detached)
        if (false == StartProtocolLayerProcess(protocol_layer_command_line, false))
        {
            qDebug() << "---The ProtocolLayerProcess doesn`t start!" << endl;
        }
        else
        {
            // Setup control port in order to be able to send commands to protocol layer itself
            SetupControlPort();

            // Return success
            res = true;
        }
    }

    return res;
}

parametersProtLayerTy ProtocolLayer::GetPLParameters ()
{
    return parameters;
}

parametersProtLayerTy ProtocolLayer::GetParameters ()
{
    parametersProtLayerTy parameters;
    QStringList strList, lst;

    lst = fileManager->LoadFile(PARAMETERS_FILE_NAME);

    fileManager->TokenizeFile("=", lst);

    // For now we have 1 parameter, just take the value
    int i = 0;

    while (i < fileManager->GetChunkListObj().size())
    {
        ChunkTy chunk;
        chunk = fileManager->GetChunkListObj().at(i);
        strList.append(chunk.key + "=" + chunk.value);

        if ("TCP_DCOP_PROTOCOL_LAYER_PORT" == chunk.key)
        {
            parameters.dcopPort = static_cast <quint32> (chunk.value.toInt());
        }
        else if ("TCP_CMOST_PROTOCOL_LAYER_PORT" == chunk.key)
        {
            parameters.cmostPort = static_cast <quint32> (chunk.value.toInt());
        }
        else if ("TCP_CTRL_PORT" == chunk.key)
        {
            parameters.controlPort = static_cast <quint32> (chunk.value.toInt());
        }
        else if ("TCP_PROTOCOL_LAYER_ADDRESS" == chunk.key)
        {
            parameters.ipPortAddress = chunk.value;
        }

        i++;
    }

    return parameters;
}

QString ProtocolLayer::MakeProtocolLayerCommandLine ()
{
    QStringList strList, lst;
    QString exeFileName = QString(TOOLS_FOLDER_NAME) + QString(PROTOCOL_LAYER_FILE_NAME);

    lst = fileManager->LoadFile(PROTOCOL_CONFIG_FILE_NAME);

    fileManager->TokenizeFile("=", lst);

    int i = 0;

    while (i < fileManager->GetChunkListObj().size())
    {
        ChunkTy chunk;
        chunk = fileManager->GetChunkListObj().at(i);
        strList.append(chunk.key + "=" + chunk.value);

        i++;
    }

    QString paramStr = strList.join(" ");
    QString command  = exeFileName + " " + paramStr;

    return command.trimmed();
}

bool ProtocolLayer::CloseProtocolLayerProcess ()
{
    int count = 0;

    if (protocolLayerProcess->pid() == 0)
    {
        qDebug() << "Protocol Layer Closed Successfully 1";

        return true;
    }
    else
    {
        while (count < 3)
        {
            protocolLayerProcess->close();

            if (protocolLayerProcess->pid() == 0)
            {
                qDebug() << "Protocol Layer Closed Successfully 2";

                return true;
            }
        }

        qDebug() << "Protocol Layer Closed Unsuccessfully 3";

        return false;
    }
}

bool ProtocolLayer::StartProtocolLayerProcess (QString protocol_layer_command_line, bool startDetached)
{
    bool res = false;

    // Starting the Protocol Layer process
    if (true == startDetached)
    {
        // Start the Protocol Layer detached, we do nto know anything about it, it is a run and forget
        protocolLayerProcess->startDetached(protocol_layer_command_line);

        res = true;
    }
    else
    {
        // Start Protocol Layer in normal mode: we can check if it is really running
        protocolLayerProcess->start(protocol_layer_command_line);

        protocolLayerProcess->waitForStarted(DEFAULT_PROCESS_START_TIMEOUT);

        // Check if the protocolLayerProcess has been started correctly
        if ((protocolLayerProcess->state() == QProcess::Running) || (protocolLayerProcess->pid() != 0))
        {
            qDebug() << "---Protocol Layer Status:  Running";

            res = true;
        }
        else
        {
            qDebug() << "---Protocol Layer Status: NotRunning!";
        }
    }

    return res;
}
