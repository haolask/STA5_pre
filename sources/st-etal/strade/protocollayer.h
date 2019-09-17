///
//! \file          protocollayer.h
//! \brief         Interface with the protocol layer program
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

#ifndef PROTOCOLLAYER_H
#define PROTOCOLLAYER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>

#include "common.h"
#include "tcptransportlayer.h"

#include "filemanager.h"

//! \brief <i><b> This structure contains the parameters used to start the protocol layer program </b></i>
struct parametersProtLayerTy
{
    quint32 dcopPort;       //!< DCOP TCP/IP port address. This port will be used to send command to the baseband device
    quint32 cmostPort;      //!< CMOST TCP/IP port address. This port will be used to send command to the tuner device
    quint32 controlPort;    //!< Control port used in order to send commands for the protocol layer itself
    QString ipPortAddress;  //!< IP address to start the external program with. It is the IP address of the machine
};

//! \brief        <i><b> Connect and send commands to external program acting as a server </b></i>
//!
//! The ProtocolLayer class implements interfacing with the external "Protocol Layer" program that acts as a TCP/IP server.
//! This class uses a control port to send command to the external programin order to ask to close when the HMI exits.
class ProtocolLayer : public QObject
{
    Q_OBJECT

    public:
        //! \brief        <i><b> Create the instance </b></i>
        //!
        //! A protocol layer instance is created and memory is properly allocated.It is possible to create the instance in different modes.
        //! If a valid object is passed as a parameter then QObjects organize themselves in object trees. When you create a QObject with
        //! another object as parent, the object will automatically add itself to the parent's children() list.
        //! The parent takes ownership of the object; i.e., it will automatically delete its children in its destructor.
        //! Otherwise the instance is created without a parent and it needs to be deleted explicitely.
        //!
        //! \param[in]    parent Parent class
        //! \return       None
        //!
        //! \remark       The explicit keyword is used in order to avoid any implicit conversion of parameter passed at construction time.
        //! \callgraph
        //! \callergraph
        explicit ProtocolLayer(QObject *parent = nullptr);

        //! \brief        <i><b> Destroy the object </b></i>
        //!
        //! The protocol layer class instance is deleted.
        //!
        //! \return       None
        //!
        //! \remark       It shall be called prior to exit the program.
        //! \callgraph
        //! \callergraph
        ~ProtocolLayer();

        //! \brief        <i><b> Run the external program 'Protocol Layer' </b></i>
        //!
        //! The program is run with configuration read from file './settings/protocol_layer_config.cfg'.
        //!
        //! \return       True if the external program has started correctly and false in case of errors
        //!
        //! \remark       It shall be called before any connection is attempted with the server side of the TCP/IP
        //!               connection is tempted.
        //! \callgraph
        //! \callergraph
        bool ProtocolLayerStart();

        //! \brief        <i><b> Close external program 'Protocol Layer' </b></i>
        //!
        //! The program is closed sending the appropriate command to the control port.
        //!
        //! \return       True if the external program has closed correctly and false in case of errors
        //!
        //! \remark       After this call the 'Protocol Layer' program remains active until all clients close the
        //!               socket.
        //! \callgraph
        //! \callergraph
        bool CloseProtocolLayerProcess();

        //! \brief        <i><b> Connect to control port </b></i>
        //!
        //! Connect to the control port socket.
        //!
        //! \return       True if the external socket has opened correctly and false in case of errors
        //!
        //! \remark       After this call it is possible to send commands to the protocol layer.
        //! \callgraph
        //! \callergraph
        bool SetupControlPort();

        //! \brief        <i><b> Retrieve 'Protocol Layer' parameters </b></i>
        //!
        //! Retrieve the protocol layer parameters. They can be used to open different sockets like the one to send and
        //! retrieve data from the tuner and from the baseband.
        //!
        //! \return       Parameter structure
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        parametersProtLayerTy GetPLParameters();

    private:
        //! \brief        <i><b> Create the command line for the external 'Protocol Layer' command </b></i>
        //!
        //! Create the command line for the 'Protocol layer' program. Fields are taken from configuration file or
        //! defined by this class.
        //!
        //! \return       QString containing the string to be passed to the program
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        QString MakeProtocolLayerCommandLine();

        //! \brief        <i><b> Run the external program 'Protocol Layer' </b></i>
        //!
        //! The program is run with configuration read from file './settings/protocol_layer_config.cfg'.
        //!
        //! \param[in]    protocol_layer_command_line Command line to use with the 'Protocol Layer' external program
        //! \param[in]    startDetached True if the program shall be started detached, false otherwise
        //! \return       True if the external program has started correctly and false otherwise
        //!
        //! \remark       If the program is started detached it cannot be killed but only properly closed using the
        //!               control port and sending the proper command. It is recommended to start not detached in order
        //!               to have better control of the external process but to close it properly sending commands.
        //! \callgraph
        //! \callergraph
        bool StartProtocolLayerProcess(QString protocol_layer_command_line, bool startDetached);

        //! \brief        <i><b> Send the command to close the protocol layer </b></i>
        //!
        //! Send command to close the protocol layer. The protocol layer will not close immediately but only when
        //! no sockets are open (the closing will happen when the last socket is closed).
        //!
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void sendCommandCloseProtocolLayer();

        //! \brief        <i><b> Check if the protocol layer is running </b></i>
        //!
        //! Check if the protocol layer is running .
        //!
        //! \return       True if the protocol layer is running, false otherwise
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        bool CheckIfProtocolLayerIsRunning();

        //! \brief        <i><b> Retrieve 'Protocol Layer' parameters </b></i>
        //!
        //! Retrieve the protocol layer parameters.
        //!
        //! \return       Parameter structure
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        parametersProtLayerTy GetParameters();

        QProcess *protocolLayerProcess;     //!< Process instance to run the external protocol layer program
        QProcess *taskKillerProcess;        //!< Process to run external command to kill the protocol layer

        FilesManager *fileManager;          //!< File manager class pointer: used to access files

        TcpTransportLayer *tcpControlPort;  //!< Socket to send command to the protocol layer

        parametersProtLayerTy parameters;   //!< Parameter structure containing TCP/IP ports used to access devices

        bool isConnected;                   //!< Boolean flag indicating if the control port socket is connected

    public slots:
        //! \brief        <i><b> Slot connected to the data RX from the 'Protocol Layer' </b></i>
        //!
        //! This function acts as a slot in order to get the data pushed in the socket by the 'Protocol Layer'
        //! in the control socket.
        //!
        //! \param[in]    data Data retrived from the control socket
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void TcpRx_slot(QByteArray data);

    signals:
        //! \brief        <i><b> Signal used to transmit data to the 'Protocol Layer' using the control port </b></i>
        //!
        //! This function acts as a signal connected to TcpTransportLayer::receivedCmdForDevice_slot.
        //! It is used to send data to teh external program using the control port. A single command is used:
        //! close the program when no more sockets are open.
        //!
        //! \param[in]    QByteArray Bytes encoding the data to be transmitted to the control port
        //! \return       None
        //!
        //! \remark       None.
        //! \callgraph
        //! \callergraph
        void sendCmdToTcp_signal(QByteArray);
};

#endif // PROTOCOLLAYER_H

// End of file
