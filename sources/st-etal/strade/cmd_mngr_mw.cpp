#include <iostream>
#include <fstream>

#include "defines.h"
#include "cmd_mngr_mw.h"

Middleware::Middleware(QObject* parent, QString _dataMsgInstName, QString _dataChInstName) :
    QObject(parent), DataMessages(_dataMsgInstName), DataChannel(_dataChInstName)
{
    utility = new Utilities();

    ensTableLst = new QList<EnsembleTableTy> ();
    serviceLst = new QList<ServiceListTy> ();
    dabBandIIIFreqList = new QList<quint32> ();

    // Initialize event manager
    eventManager = new EventManager<eventsList, eventDataInterface> ();

    mwFreqTablesSetup();
    mwInitQualityInfo();

    currEnsembleIndex = MAX_NB_DAB_FREQUENCIES;
    removeServiceFlag = false;
}

Middleware::~Middleware()
{
    delete utility;
}

void Middleware::mwInitQualityInfo()
{
    qualityInfo.qualFstRf = 0;
    qualityInfo.qualFstBb = 0;
    qualityInfo.qualFicBer = 0;
    qualityInfo.fmQualityInfo.qualDetune = 0;
    qualityInfo.fmQualityInfo.qualMultiPath = 0;
    qualityInfo.fmQualityInfo.qualMpxNoise = 0;
    qualityInfo.fmQualityInfo.qualSNR = 0;
    qualityInfo.fmQualityInfo.qualAdj = 0;
    qualityInfo.fmQualityInfo.qualCoChannel = 0;
    qualityInfo.fmQualityInfo.qualDeviation = 0;
    qualityInfo.fmQualityInfo.qualStereo = 0;

    qualityInfo.dabQualityInfo.audioBer = 0;
    qualityInfo.dabQualityInfo.audioCRCError = 0;
    qualityInfo.dabQualityInfo.audioCRCTotal = 0;
    qualityInfo.dabQualityInfo.ficBer = 0;
    qualityInfo.dabQualityInfo.mscBer = 0;
    qualityInfo.dabQualityInfo.qualDabTxMode = 0;
    qualityInfo.dabQualityInfo.qualServiceBitRate = 0;
    qualityInfo.dabQualityInfo.serviceComponent = 0;
    qualityInfo.dabQualityInfo.serviceSubCh = 0;
}

void Middleware::mdw_receivedDataFromDcopTcpLayer_slot(QByteArray data)
{
    quint16 data_length = 0;
    QByteArray data_tmp;
    quint16 curByte = 0;
    bool dataToParse = true;

    while (true == dataToParse)
    {
        // Check for Synchronization byte
        if ((QChar)data[curByte + COMMAND_SYNC_FIELD] == TARGET_MESSAGE_SYNC_BYTE)
        {
            // Get data and payload length
            data_length = ((quint8)data[curByte + COMMAND_DATA_LENGTH_FIELD] << 8) |
                (quint8)data[curByte + COMMAND_DATA_LENGTH_FIELD + 1];

            // Store the relevant data for this command
            data_tmp = data.mid((curByte + COMMAND_PAYLOAD_FIELD), data_length);

            if (data_tmp.size() != 0)
            {
                StatusCodeTy statusCode;
                MdwDataPacketTy decodedData = SaveMiddlewareReceivedData(data_tmp, &statusCode);

                // Save data & check responses status code
                SaveMiddlewareResponseData(decodedData);

                curByte += (data_length + COMMAND_PAYLOAD_FIELD);

                // If all bytes are parsed exit the while
                if (curByte >= data.size())
                {
                    dataToParse = false;
                }
            }
            else
            {
                dataToParse = false;
            }
        }
        else
        {
            // Wrong identifier byte found
            qDebug() << "ERROR: 0x1D is not in the right position from protocol layer message";
        }
    }
}

void Middleware::mwCommandExecute(QString command, bool waitResponse)
{
    StatusCodeTy errCode = CMD_OK;
    QByteArray parsed_command;

    // Parse middleware command and check syntax errors
    errCode = MdwCmdParser(command, &parsed_command);

    // Transmit middleware command
    errCode = MdwCmdTransmitter(parsed_command, waitResponse);

    // Send result code
    mdwSendCmdExecStatus(errCode);
}

void Middleware::mdwSendCmdExecStatus(StatusCodeTy cmdStatus)
{
    DataMessages::GetMdwNotificationObj()->notifStatusCode = cmdStatus;
    DataMessages::GetMdwNotificationObj()->notifStatusCodeName = DataMessages::GetMdwStatusCodeName(cmdStatus);
}

StatusCodeTy Middleware::MdwCmdParser(QString& command, QByteArray* prepared_cmd)
{
    QByteArray parsed_command = 0;
    qint32 msgId = 0, strLenght = 0;
    qint16 payloadLength = 0, bytesNb = 0;

    // Check if command line is empty then skip it
    if (command.isEmpty() || command.startsWith("\\"))
    {
        *prepared_cmd = 0;

        return CMD_SINTAX_ERROR;
    }

    // Parse the command string
    command = command.toLower();
    command = command.simplified(); // remove ASCII characters '\t', '\n', '\v', '\f', '\r', and ' '.
    command = command.remove(QChar(' '), Qt::CaseInsensitive); // remove spaces

    // Syntax trap
    if (command.contains(QRegExp("[^1234567890abcdef]")))
    {
        *prepared_cmd = 0;

        return CMD_SINTAX_ERROR;
    }

    command = DataMessages::InsertSpaces(command, 2);  // insert spaces every two characters

    QStringList cmdList = command.split(" ", QString::SkipEmptyParts);

    // Minimum required parameters number is not reached
    if (cmdList.size() < 3)
    {
        *prepared_cmd = 0;

        return CMD_SINTAX_ERROR;
    }

    // Retrieve the number of BYTES to write
    payloadLength = bytesNb = cmdList.count();

    parsed_command[COMMAND_PAYLOAD_FIELD + 2]      = (bytesNb >>  8) & 0xFF;
    parsed_command[COMMAND_PAYLOAD_FIELD + 3]      =  bytesNb & 0xFF;

    parsed_command.resize(COMMAND_COMMON_HEADER_LENGTH + payloadLength);

    parsed_command[COMMAND_SYNC_FIELD]             = TARGET_MESSAGE_SYNC_BYTE;
    parsed_command[COMMAND_PROTOCOL_LUN_FIELD]     = 0x30;
    parsed_command[COMMAND_MESSAGE_ID_FIELD]       = 0x00; // (message_id >> 24) & 0xFF;
    parsed_command[COMMAND_MESSAGE_ID_FIELD + 1]   = 0x00; // (message_id >> 16) & 0xFF;
    parsed_command[COMMAND_MESSAGE_ID_FIELD + 2]   = 0x00; // (message_id >>  8) & 0xFF;
    parsed_command[COMMAND_MESSAGE_ID_FIELD + 3]   = 0x00; // message_id & 0xFF;
    parsed_command[COMMAND_DATA_LENGTH_FIELD]      = (payloadLength >>  8) & 0xFF;
    parsed_command[COMMAND_DATA_LENGTH_FIELD + 1]  =  payloadLength & 0xFF;
    parsed_command[COMMAND_PAYLOAD_FIELD + 0]      = (msgId >>  8) & 0xFF;
    parsed_command[COMMAND_PAYLOAD_FIELD + 1]      =  msgId & 0xFF;

    // Fill the command buffer with command data in BIG ENDIAN format
    int value = 0, j = 0;
    bool ok = true;

    for (int i = 0; i < cmdList.count(); i++)
    {
        strLenght = cmdList[i].length();
        value = cmdList[i].toUInt(&ok, 16);
        strLenght = (strLenght / 2) + (strLenght % 2);

        for (int k = (strLenght - 1); k >= 0; k--)
        {
            parsed_command[COMMAND_PAYLOAD_FIELD + j] = (value >> (8 * k)) & 0xFF;
            j++;
        }
    }

    *prepared_cmd = parsed_command;

    return CMD_OK;
}

StatusCodeTy Middleware::MdwCmdTransmitter(QByteArray parsed_command, bool waitResponse)
{
    QTimer* notificationTimer;
    QTimer* responseTimer;

    if (parsed_command.size() == 0)
    {
        return NO_DATA_TO_WRITE;
    }

    // Prepare timer related flags
    isMwNotifStatusReceived = false;
    isMwNotifTimeout = false;
    isMwRespReceived = false;
    isMwRespTimeout = false;

    // Emit signal (with this call the command is executed)
    emit mdw_sendCmdToDispatcher_signal(parsed_command);

    notificationTimer = new QTimer(0);

    notificationTimer->setInterval(500);
    notificationTimer->setSingleShot(true);

    QObject::connect(notificationTimer, &QTimer::timeout, this, &Middleware::mwNotifTimer_timeout_slot); // New syntax

    notificationTimer->start();

    // Wait the notification
    while (false == isMwNotifTimeout && false == isMwNotifStatusReceived)
    {
        QApplication::processEvents();
    }

    notificationTimer->stop();

    if (true == QObject::disconnect(notificationTimer))
    {
        delete notificationTimer;
    }

    if (true == waitResponse)
    {
        responseTimer = new QTimer(0);

        QObject::connect(responseTimer, &QTimer::timeout, this, &Middleware::mwRespTimer_timeout_slot); // New syntax

        responseTimer->setInterval(3500);
        responseTimer->setSingleShot(true);
        responseTimer->start();

        // Wait the response
        while (false == isMwRespTimeout && false == isMwRespReceived)
        {
            QApplication::processEvents();
        }

        responseTimer->stop();

        if (true == QObject::disconnect(responseTimer))
        {
            delete responseTimer;
        }
    }

    return CMD_OK;
}

MdwDataPacketTy Middleware::SaveMiddlewareReceivedData(QByteArray data, StatusCodeTy* statusCode)
{
    MdwDataPacketTy decodedDataObj;

    HeaderZeroTy headZero;

    headZero.byte = data[0];

    quint8 status = data[1];
    status = status >> 2;

    // DATA ON DATACHANNEL
    if ((quint8)headZero.byte == (quint8)0xD6)
    {
        quint8 msb, lsb, lsb0, lsb1;

        quint8 h0 = data.at(0); // h0 must be 0xD6
        quint8 h1 = data.at(1); // h1 must be 0x64 (if MAIN_DAB and MessageType = DECODED )
        quint8 h2 = data.at(2); // PacketAdd LSB
        quint8 h3 = data.at(3); // PacketAdd MSB + SubChId
        quint8 h4 = data.at(4); // Type (specifies the type of payload for DECODED mode (e.g. EPG, BWS�))
        quint8 h5 = data.at(5); // MSB of Size field and Continuity index
        quint8 h6 = data.at(6); // Size
        quint8 h7 = data.at(7); // Size

        Q_UNUSED(h0)

        // Get current application Id
        quint8 CurrAppId = (h1 & 0x10) >> 4; // 0 = MAIN_DAB, 1 = SECONDARY_DAB

        DataChannel::DataChProtocolHeader->CurrAppId = CurrAppId;

        // Get and save Message type
        quint8 tmp = (h1 & 0xE0) >> 5;

        decodedDataObj.msgType = (MessageTypeTy)tmp;

        if (decodedDataObj.msgType == MDW_ASCII_DATA_ON_DATACHANNEL)                 // 0x84 ASCII data on datachannel
        {
            qDebug() << "ASCII Data On Datachannel: received " << data.size() << " bytes";
        }
        else if (decodedDataObj.msgType  == MDW_FIC_BINARY_DATA_ON_DATACHANNEL)      // 0x04 FIC bynary data on datachannel
        { }
        else if (decodedDataObj.msgType  == MDW_RAW_BINARY_DATA_ON_DATACHANNEL)      // 0x44 RAW bynary data on datachannel
        {
            qDebug() << "RAW BINARY Data On Datachannel: received " << data.size() << " bytes";
        }
        else if (decodedDataObj.msgType  == MDW_DECODED_BINARY_DATA_ON_DATACHANNEL)  // 0x64 DECODED bynary data on datachannel
        {
            // SubChId
            decodedDataObj.mot.BinDecodedAppHeader.SubChId = h3 & 0x3F;

            // PacketAdd
            msb  = h3 >> 6;
            lsb = h2;
            decodedDataObj.mot.BinDecodedAppHeader.PacketAdd = (msb << 8) | lsb;

            // ContinuityIndex
            quint8 ContinuityIndex = h5 & 0x0F;
            decodedDataObj.mot.BinDecodedAppHeader.ContinuityIndex = ContinuityIndex;

            // Size
            msb = 0;
            msb = h5 >> 4;
            lsb0 = h7;
            lsb1 = h6;
            quint32 Size = lsb0 | (lsb1 << 8) | (msb << 16);
            decodedDataObj.mot.BinDecodedAppHeader.Size = Size;

            // Type == PayloadFormat
            DecodedPayloadFormatTy DecodedPayloadFormat = (DecodedPayloadFormatTy)h4;
            decodedDataObj.mot.BinDecodedAppHeader.PayloadFormat = DecodedPayloadFormat;

            // Check PayloadType and decode data
            switch (DecodedPayloadFormat)
            {
                case EPG_RAW: // = 0x01, MOT Body objects containing EPG
                    break;

                case SLS: // = 0x02, MOT Body objects containing SLS extracted from a data stream
                case SLS_OVER_XPAD: // = 0x03, MOT Body objects containing SLS extracted from the X-PAD of an audio stream.
                {                    //         The PacketAddress field of the Binary Application Header is set to 0 in this case.
                    QByteArray dataChannelPayload;
                    quint32 dataEntryPos;
                    quint32 contentNameLen;
                    quint32 mimeTypeLen;
                    quint32 appParamLen;
                    quint32 globalAppParamLen;
                    quint32 globalUADataLen;

                    // To get the MOT body length we could use the first 2 bytes
                    // (this could be used to jump over it but for debug I parse all fields),
                    // instead we parse all sub-fields of teh RAW header

                    // Get the ContentNameLen
                    dataEntryPos = 14;
                    contentNameLen = data[dataEntryPos];
                    dataEntryPos += (contentNameLen + 1);

                    // Get the MimeTypeLen and jump over:
                    // - MimeTypeLen itself
                    // - MimeTypeCharSet
                    // - MimeType (label)
                    mimeTypeLen = data[dataEntryPos];
                    dataEntryPos += (mimeTypeLen + 1 + 1);

                    // Get the AppParamLen and jump over:
                    // - AppParamLen itself
                    // - AppParam (label)
                    appParamLen = data[dataEntryPos];
                    dataEntryPos += (appParamLen + 1);

                    // Get the GlobalAppParamLen and jump over:
                    // - GlobalAppParamLen itself
                    // - GlobalAppParam (label)
                    globalAppParamLen = data[dataEntryPos];
                    dataEntryPos += (globalAppParamLen + 1);

                    // Get the GlobalUADataLen and jump over:
                    // - GlobalUADataLen itself
                    // - GlobalUAData (label)
                    globalUADataLen = data[dataEntryPos];
                    dataEntryPos += (globalUADataLen + 1);

                    // MotBody
                    dataChannelPayload.insert(0, data.data(), data.size());
                    dataChannelPayload.remove(0, dataEntryPos);
                    decodedDataObj.mot.MotBody = dataChannelPayload;

                    qDebug() << "MOT received, size: " << decodedDataObj.mot.MotBody.size();

                    SendSls(decodedDataObj.mot.MotBody);

                    emit inShellMwData_signal("\n[i] Response info:SLS = New Image", " ", RADIO_DATA_COMMAND);
                }
                break;

                case TPEG_RAW: // = 0x05, DataGroup object containing TPEG extracted from a TDC data stream
                case TPEG_SNI: // = 0x06, A binary object containing the TPEG Service and Network Information (SNI).
                case SERVICE_LINKING_INFO: // = 0x07, A binary object describing the service linking Information.
                case EPG_BIN: // = 0x10, A complete set of EPG objects.
                case EPG_SRV: // = 0x12, EPG Service Information decoded from XML, filtered and re-encoded as a plain table of strings.
                case EPG_PRG: // = 0x13, EPG Programme Information decoded from XML, filtered and re-encoded as a plain table of strings
                    break;

                default:
                    // No code
                    break;
            }

            qDebug() << " Data Channel: DECODED Bynary Data Received... [" << data.size() << " ] bytes";
        }
        else if (decodedDataObj.msgType == MDW_AUDIO_BINARY_DATA_ON_DATACHANNEL) // 0xC4 AUDIO bynary data on datachannel
        {
            qDebug() << " Data Channel: AUDIO Bynary Data Received... [" << data.size() << " ] bytes";
        }

        // return status code
        *statusCode = (StatusCodeTy)CMD_OK;
    }
    else
    {
        // Check what we received, possible cases are:
        // - Immediate notification
        // - Response
        // - Auto-notification
        if ((headZero.bits.Reply == 0) && (headZero.bits.Host == 0)) // NOTIFICATION
        {
            // Save message type
            decodedDataObj.msgType = MDW_NOTIFICATION;

            // Save Notification data
            decodedDataObj.mdwNotif.notifData = data;
            decodedDataObj.mdwNotif.notifDataStr = DataMessages::FormatMdwCommandString(DataMessages::ByteArrayToStr(data));

            // Save payload length and header length
            if (headZero.bits.Data == 0)
            {
                decodedDataObj.mdwNotif.notifPayloadLenght = 0x00;
                decodedDataObj.mdwNotif.notifHeaderLenght = 0x03;
            }
            else if ((headZero.bits.Data == 1) && (headZero.bits.Len == 0)) // data = 1 && len = 0
            {
                decodedDataObj.mdwNotif.notifPayloadLenght = data[3];
                decodedDataObj.mdwNotif.notifHeaderLenght = 0x04;
            }
            else if ((headZero.bits.Data == 1) && (headZero.bits.Len == 1)) // data = 1 && len = 1
            {
                // TODO: implement
            }

            // Extract command code from notification
            quint8 headOne = (quint8)data.at(1);
            quint8 headTwo = (quint8)data.at(2);
            quint16 cmdCode = 0x000;
            quint8 cmdMsb = headOne & 0x03;
            quint8 cmdLsb = headTwo;
            cmdCode |= (cmdMsb << 8);
            cmdCode |= (cmdLsb);

            // Save notification command code
            decodedDataObj.mdwNotif.notifCmdCode = (MdwCommandCodeTy)cmdCode;

            // Save notification command code name
            decodedDataObj.mdwNotif.notifCmdCodeName = DataMessages::GetMdwCommandName((MdwCommandCodeTy)cmdCode);

            // Save notification status code
            decodedDataObj.mdwNotif.notifStatusCode = (StatusCodeTy)status;

            // Save notification status code name
            decodedDataObj.mdwNotif.notifStatusCodeName = DataMessages::GetMdwStatusCodeName((StatusCodeTy)status);

            // Return status code
            *statusCode = (StatusCodeTy)status;
        }
        else if ((headZero.bits.Reply == 1) && (headZero.bits.Host == 0)) // RESPONSE & AUTORESPONSE
        {
            // Save message type
            if (headZero.bits.Auto)
            {
                decodedDataObj.msgType = MDW_AUTORESPONSE;
                // Save response data
                decodedDataObj.mdwAutoResponse.respData = data;
                decodedDataObj.mdwAutoResponse.respDataStr = DataMessages::FormatMdwCommandString(DataMessages::ByteArrayToStr(data));

                // Save payload length and header length
                if (headZero.bits.Data == 0) // data = 0
                {
                    decodedDataObj.mdwAutoResponse.respPayloadLenght = 0x00;
                    decodedDataObj.mdwAutoResponse.respHeaderLenght  = 0x03;
                }
                else if ((headZero.bits.Data == 1) && (headZero.bits.Len == 0)) // data = 1 && len = 0
                {
                    decodedDataObj.mdwAutoResponse.respPayloadLenght = data[3];
                    decodedDataObj.mdwAutoResponse.respHeaderLenght  = 0x04;
                }
                else if ((headZero.bits.Data == 1) && (headZero.bits.Len == 1)) // data = 1 && len = 1
                {
                    decodedDataObj.mdwAutoResponse.respPayloadLenght = data.size() - 0x05;
                    decodedDataObj.mdwAutoResponse.respHeaderLenght  = 0x05;
                }

                // Extract command code from response
                quint8 headOne = (quint8)data.at(1);
                quint8 headTwo = (quint8)data.at(2);
                quint16 cmdCode = 0x000;
                quint8 cmdMsb = headOne & 0x03;
                quint8 cmdLsb = headTwo;
                cmdCode |= (cmdMsb << 8);
                cmdCode |= (cmdLsb);

                // Save response command code
                decodedDataObj.mdwAutoResponse.respCmdCode = (MdwCommandCodeTy)cmdCode;

                // Save response command code name
                decodedDataObj.mdwAutoResponse.respCmdCodeName =  DataMessages::GetMdwCommandName((MdwCommandCodeTy)cmdCode);

                // Save status code
                decodedDataObj.mdwAutoResponse.respStatusCode = (StatusCodeTy)status;

                // Save status code name
                decodedDataObj.mdwAutoResponse.respStatusCodeName = DataMessages::GetMdwStatusCodeName((StatusCodeTy)status);
            }
            else
            {
                decodedDataObj.msgType = MDW_RESPONSE;

                // Save response data
                decodedDataObj.mdwResponse.respData = data;
                decodedDataObj.mdwResponse.respDataStr =  DataMessages::FormatMdwCommandString(DataMessages::ByteArrayToStr(data));

                // Save payload length and header length
                if (headZero.bits.Data == 0) // data = 0
                {
                    decodedDataObj.mdwResponse.respPayloadLenght = 0x00;
                    decodedDataObj.mdwResponse.respHeaderLenght  = 0x03;
                }
                else if ((headZero.bits.Data == 1) && (headZero.bits.Len == 0)) // data = 1 && len = 0
                {
                    decodedDataObj.mdwResponse.respPayloadLenght = data[3];
                    decodedDataObj.mdwResponse.respHeaderLenght  = 0x04;
                }
                else if ((headZero.bits.Data == 1) && (headZero.bits.Len == 1)) // data = 1 && len = 1
                {
                    decodedDataObj.mdwResponse.respPayloadLenght = data.size() - 0x05;
                    decodedDataObj.mdwResponse.respHeaderLenght  = 0x05;
                }

                // Extract command code from response
                quint8 headOne = (quint8)data.at(1);
                quint8 headTwo = (quint8)data.at(2);
                quint16 cmdCode = 0x000;
                quint8 cmdMsb = headOne & 0x03;
                quint8 cmdLsb = headTwo;
                cmdCode |= (cmdMsb << 8);
                cmdCode |= (cmdLsb);

                // Save response command code
                decodedDataObj.mdwResponse.respCmdCode = (MdwCommandCodeTy)cmdCode;

                // Save response command code name
                decodedDataObj.mdwResponse.respCmdCodeName =  DataMessages::GetMdwCommandName((MdwCommandCodeTy)cmdCode);

                // Save status code
                decodedDataObj.mdwResponse.respStatusCode = (StatusCodeTy)status;

                // Save status code name
                decodedDataObj.mdwResponse.respStatusCodeName = DataMessages::GetMdwStatusCodeName((StatusCodeTy)status);
            }

            *statusCode = (StatusCodeTy)status;
        }
        else if ((headZero.byte == (quint8)0xF0) || (headZero.byte == (quint8)0xF2) || (headZero.byte == (quint8)0xFA)) // AUTONOTIFICATION
        {
            // Save message type
            decodedDataObj.msgType = MDW_AUTONOTIFICATION;

            // Save auto-notification data
            decodedDataObj.mdwAutoNotif.autoNotifData = data;
            decodedDataObj.mdwAutoNotif.autoNotifDataStr = DataMessages::FormatMdwCommandString(DataMessages::ByteArrayToStr(data));

            // Save payload length and header length
            if (headZero.bits.Data == 0)
            {
                decodedDataObj.mdwAutoNotif.autoNotifPayloadLenght = 0x00;
                decodedDataObj.mdwAutoNotif.autoNotifHeaderLenght = 0x03;
            }
            else if ((headZero.bits.Data == 1) && (headZero.bits.Len == 0)) // data = 1 && len = 0
            {
                decodedDataObj.mdwAutoNotif.autoNotifPayloadLenght = data[3];
                decodedDataObj.mdwAutoNotif.autoNotifHeaderLenght = 0x04;
            }
            else if ((headZero.bits.Data == 1) && (headZero.bits.Len == 1)) // data = 1 && len = 1
            {
                // TODO: implement
            }

            // Save status code
            decodedDataObj.mdwAutoNotif.autoNotifStatusCode = (StatusCodeTy)status;

            // Save status code name
            decodedDataObj.mdwAutoNotif.autoNotifStatusCodeName = DataMessages::GetMdwStatusCodeName((StatusCodeTy)status);

            // Return status code
            *statusCode = (StatusCodeTy)status;
        }
    }

    return decodedDataObj;
}

void Middleware::mwNotifTimer_timeout_slot(void)
{
    isMwNotifTimeout = true;
}

void Middleware::mwRespTimer_timeout_slot()
{
    isMwRespTimeout = true;
}

RadioSequencesStatusTy Middleware::GetMdwResponse()
{
    StatusCodeTy respStatusCode = DataMessages::GetMdwResponseObj()->respStatusCode;

    // Check if we got a response and then if it is correct
    if (false == isMwRespReceived)
    {
        return RADIO_SEQUENCE_INTERNAL_TIMEOUT_ERROR;
    }
    else
    {
        if (respStatusCode == OPERATION_TIMEOUT)
        {
            return RADIO_NO_SIGNAL_ERROR;
        }
        else if (respStatusCode != CMD_OK)
        {
            return RADIO_GENERIC_SEQUENCE_ERROR;
        }
    }

    return RADIO_SEQUENCE_OK;
}

RadioSequencesStatusTy Middleware::GetMdwNotification()
{
    StatusCodeTy notifStatusCode = DataMessages::GetMdwNotificationObj()->notifStatusCode;

    // Check if we got a correct notification
    if (notifStatusCode != CMD_OK)
    {
        if (notifStatusCode == CONNECTION_ERRORS)
        {
            // Error
            return RADIO_NO_CONNECTION_AVAILABLE_ERROR;
        }
        else if (notifStatusCode == DUPLICATED_AUTO_COMMAND)
        {
            // We keep going
            return RADIO_SEQUENCE_OK;
        }

        // Error
        return RADIO_GENERIC_SEQUENCE_ERROR;
    }

    return RADIO_SEQUENCE_OK;
}

RadioSequencesStatusTy Middleware::CommandSend(MdwCommandCodeTy commandCode, QString commandString, bool waitResponse)
{
    RadioSequencesStatusTy seqStatus;

    mwCommandExecute(commandString, waitResponse);

    emit inShellMwData_signal("\n[i] Send Command:" +
                              DataMessages::GetMdwCommandName(commandCode), "\n[c] " + commandString, RADIO_DATA_COMMAND);

    emit inShellMwData_signal("\n[n] " +
                              DataMessages::GetMdwNotificationObj()->notifDataStr, " ", RADIO_DATA_NOTIFY);

    // Get notification status
    seqStatus = GetMdwNotification();

    if (seqStatus != RADIO_SEQUENCE_OK)
    {
        emit inShellMwError_signal(seqStatus,
                                   DataMessages::GetMdwNotificationObj()->notifDataStr,
                                   DataMessages::GetMdwNotificationObj()->notifStatusCodeName);

        return seqStatus;
    }

    // Get response
    if (true == waitResponse)
    {
        emit inShellMwData_signal("\n[r] " +
                                  DataMessages::GetMdwResponseObj()->respDataStr, " ", RADIO_DATA_RESPONSE);

        // Check status
        seqStatus = GetMdwResponse();

        if (seqStatus != RADIO_SEQUENCE_OK)
        {
            emit inShellMwData_signal("\nERROR in Response Status: " +
                                      DataMessages::GetMdwResponseObj()->respStatusCodeName, " ", RADIO_DATA_RESPONSE);

            return seqStatus;
        }
    }

    return RADIO_SEQUENCE_OK;
}

RadioSequencesStatusTy Middleware::CommandExecute(MdwCommandCodeTy commandCode, bool waitResponse)
{
    QStringList commandList;
    RadioSequencesStatusTy ret;

    Q_UNUSED(waitResponse); // We override the request using the internal command format to understand if there's a reponse

    commandList.clear();
    commandList = CommandPrepare(commandCode); // After this call the command properties are saved (i.e. need response)

    if (commandList.isEmpty())
    {
        return RADIO_GENERIC_SEQUENCE_ERROR;
    }

    for (int cnt = 0; cnt < commandList.size(); cnt++)
    {
        // ret = CommandSend(commandCode, commandList.at(cnt), waitResponse);
        ret = CommandSend(commandCode, commandList.at(cnt), DataMessages::GetMdwCommandObj()->cmdNeedResponse);

        if (RADIO_SEQUENCE_OK != ret)
        {
            return ret;
        }
    }

    return RADIO_SEQUENCE_OK;
}

QStringList Middleware::CommandPrepare(MdwCommandCodeTy m_commandCode)
{
    QStringList m_commandList;
    EnsembleTableTy ensTable;
    ServiceListTy serList;
    bool commandAppended = false;
    QString cmdStr;

    switch (m_commandCode)
    {
        case MIDW_GET_ALIVE_CHK:
#if 1
            // Prepare FAST command
            cmdStr = QString("94 00 00");

            // Put in command list
            m_commandList.append(cmdStr);

            commandAppended = true;
#endif

            break;

        case MIDW_GET_FIRMW_VER:
            // Prepare FAST command
            cmdStr = QString("94 00 02");

            // Put in command list
            m_commandList.append(cmdStr);

            commandAppended = true;
            break;

        case MIDW_SETUP_EVENT_NOTIFICATIONS:
        {
            if (true == isEventAutonotificationRunning)
            {
                cmdStr = QString("96 80 10 02 00 4E");
            }
            else
            {
                cmdStr = QString("96 80 10 02 00 00");
            }

            // Put in command list
            m_commandList.append(cmdStr);

            commandAppended = true;
        }
        break;

        case MIDW_GET_ENSEMBLE_LIST:
        {
            // For now we request the data for the MAIN DAB application, second byte is then set to 0x04
            cmdStr = QString("90 04 40");

            // Put in command list
            m_commandList.append(cmdStr);

            commandAppended = true;
        }
        break;

        case MIDW_GET_ENSEMBLE_DATA:
        {
            EnsembleTableTy ensTable = ensTableLst->at(currEnsembleIndex);

            if (false == ensTable.EnsECCID.isEmpty())
            {
                // For now we request the data for the MAIN DAB application, second byte is then set to 0x04
                QString CommandHeaders = "92 04 41 03";

                // Format command string
                cmdStr = DataMessages::FormatMdwCommandString(CommandHeaders + ensTable.EnsECCID);

                // Add command to command list
                m_commandList.append(cmdStr);

                commandAppended = true;
            }
        }
        break;

        case MIDW_GET_SERVICE_LIST:
        {
            // For now we request the data for the MAIN DAB application,
            // second byte is then set to 0xC4 to get all audio services (bit 6) and all data services (bit 7)
            ensTable = ensTableLst->at(currEnsembleIndex);
            QString CommandHeaders = "92 C4 43 03";

            // Format command string
            cmdStr = DataMessages::FormatMdwCommandString(CommandHeaders + ensTable.EnsECCID);

            // Add command to command list
            m_commandList.append(cmdStr);

            commandAppended = true;
        }
        break;

        case MIDW_GET_SPECIFIC_SERVICE_DATA:
        {
            // For now we request the data for the MAIN DAB application,
            // second byte is then set to 0x04 we do not want to get secondary services (bit 6) and extended info (bit 7)
            QString CommandHeaders = "92 04 44 07";

            if (ensTableLst->size() < currEnsembleIndex)
            {
                qDebug() << " Error in QList ensTableLst size";
            }
            else
            {
                ensTable = ensTableLst->at(currEnsembleIndex);

                //while (iter != servicesMap.end())
                foreach(auto it, servicesMap)
                {
                    // Format command string
                    cmdStr = DataMessages::FormatMdwCommandString(CommandHeaders + ensTable.EnsECCID + it.ServiceID);

                    // Add command to command list
                    m_commandList.append(cmdStr);

                    commandAppended = true;
                }
            }
        }
        break;

        case MIDW_GET_PAD_DLS:
        {
            if (true == padStart)
            {
                cmdStr = QString("B0 00 51");
            }
            else
            {
                cmdStr = QString("B1 00 51");
            }

            // Add command to command list
            m_commandList.append(cmdStr);

            commandAppended = true;
        }
        break;

        case MIDW_TUNE_FREQUENCY:
        {
            // Get ensemble
            ensTable = GetEnsTableLst_Obj()->at(currEnsembleIndex);
            serList = GetServiceLst_Obj()->at(currEnsembleIndex);

            // Clear old data ---> do not clear 'ensFrequency'
            ensTable.EnsChLabel.clear();
            ensTable.EnsECCID.clear();
            ensTable.EnsTuneSync = 0;
            ensTable.EnsTune_Ber_Fic = 0;

            // Clear service list
            serList.serviceList.clear();

            // Clear the service map for the current ensemble
            servicesMap.clear();

            // Save cleared ensemble
            ensTableLst->replace(currEnsembleIndex, ensTable);

            // Prepare command string
            QString CommandFreqParameter = "40"; // Set to "40" to have immediate tune, "00" for normal mode
            QString CommandHeaders = "92 04 60 05";

            // Prepare  command
            QString freqHexStr;

            if (activeFreq == 0)
            {
                freqHexStr = " 00 00 00 00 ";
            }
            else
            {
                freqHexStr = getFreqHexStrFromFrequency(ensTable.ensFrequency);
            }

            cmdStr = DataMessages::FormatMdwCommandString(CommandHeaders + freqHexStr + CommandFreqParameter);

            // Add command to command list
            m_commandList.append(cmdStr);

            commandAppended = true;
        }
        break;

        case MIDW_SERVICE_SELECT:
        {
            QString CommandHeaders;

            // Service remove
            if (true == removeServiceFlag)
            {
                CommandHeaders = "92 04 61 08 81";
            }
            else
            {
                CommandHeaders = "92 04 61 08 85";
            }

            ensTable = ensTableLst->at(currEnsembleIndex);

            foreach(auto it, servicesMap)
            {
                if (it.serviceUniqueId == currServiceId)
                {
                    // Format command string
                    cmdStr = DataMessages::FormatMdwCommandString(CommandHeaders + ensTable.EnsECCID + it.ServiceID);

                    // Add command to command list
                    m_commandList.append(cmdStr);

                    commandAppended = true;

                    break;
                }
            }
        }
        break;

        case MIDW_SEEK:
        {
            // No code (this is not used)
        }
        break;

        case MIDW_GET_DAB_INFO:
            if (false == startReadQualFlag)
            {
                // STOP Acquire Quality
                cmdStr = QString("B3 04 03");
            }
            else
            {
                // START Acquire Quality
                cmdStr = QString("B2 04 03 02 00 90");
            }

            // Add command to command list
            m_commandList.append(cmdStr);

            commandAppended = true;
            break;

        case MIDW_MUTE:
        {
            if (true == muteCommandFlag) // if mute status is Mute
            {
                cmdStr = QString("94 44 82");  // MUTE
            }
            else // mute stop
            {
                cmdStr = QString("94 84 82");   // UNMUTE
            }

            // Add command to command list
            m_commandList.append(cmdStr);

            commandAppended = true;
        }
        break;

        case MIDW_GET_MOT:
        {
            cmdStr = QString("96 05 00 04 00 00 00 04");

            // Add command to command list
            m_commandList.append(cmdStr);

            commandAppended = true;
        }
        break;

        case MIDW_STOP_MOT:
        {
            cmdStr = QString("96 05 01 04 00 00 00 04");

            // Add command to command list
            m_commandList.append("96 05 01 04 00 00 00 04");

            commandAppended = true;
        }
        break;

        default:
            qDebug() << "ERROR: Unknown command code received";
            break;
    }

    if (true == commandAppended)
    {
        if (false == cmdStr.isEmpty())
        {
            MdwDataPacketTy dataCmd = DataMessages::ExtractCommandInfo(cmdStr);

            // Save command data
            DataMessages::GetMdwCommandObj()->cmdCode               = dataCmd.mdwCommand.cmdCode;
            DataMessages::GetMdwCommandObj()->cmdName               = dataCmd.mdwCommand.cmdName;
            DataMessages::GetMdwCommandObj()->cmdNeedAutoResponse   = dataCmd.mdwCommand.cmdNeedAutoResponse;
            DataMessages::GetMdwCommandObj()->cmdNeedResponse       = dataCmd.mdwCommand.cmdNeedResponse;
            DataMessages::GetMdwCommandObj()->cmdStatusCode         = CMD_STATUS_RUNNING;
            DataMessages::GetMdwCommandObj()->cmdStr                = dataCmd.mdwCommand.cmdStr;

            cmdNeedResponse                                         = dataCmd.mdwCommand.cmdNeedResponse;
            cmdNeedAutoResponse                                     = dataCmd.mdwCommand.cmdNeedAutoResponse;

            if ("MIDW_GET_DAB_INFO" == dataCmd.mdwCommand.cmdName)
            {
                qDebug() << "Got MIDW_GET_DAB_INFO answer";
            }
        }
        else
        {
            qDebug() << "ERROR: command string is empty when it shouldn't";
        }
    }
    else
    {
        qDebug() << "ERROR: command not appended";
    }

    return m_commandList;
}

void Middleware::DecodeImmediateAnswer(QByteArray data, MdwCommandCodeTy m_commandCode)
{
    Q_UNUSED(data);

    switch (m_commandCode)
    {
        case MIDW_GET_ALIVE_CHK:
            SendPing();
            break;

        default:
            // No code
            break;
    }
}

void Middleware::DecodeAutoNotify(QByteArray data)
{
    HeaderZeroTy headZero;

    headZero.byte = data[0];

    if ((headZero.byte == (quint8)0xF2) && (data.size() == 16))
    {
        if ((currEnsembleIndex >= 0) &&
            (currEnsembleIndex < ensTableLst->size()))
        {
            // Save signal level
            EnsembleTableTy ensTable = ensTableLst->at(currEnsembleIndex);
            ensTable.EnsTuneSync = data[15];
            ensTable.EnsTune_Ber_Fic = data[8];
            ensTable.EnsTxMode = data[7];

            // Save ensemble into list
            ensTableLst->replace(currEnsembleIndex, ensTable);

            qualityInfo.qualFicBer = ensTableLst->at(currEnsembleIndex).EnsTune_Ber_Fic;

            if (ensTable.EnsTuneSync == SYNC_DECODE_MCI)
            {
                qualityInfo.sync = true;
                qualityInfo.dabQualityInfo.qualDabTxMode = ensTableLst->at(currEnsembleIndex).EnsTxMode;

                if (false == servicesMap.contains(currServiceId))
                {
                    ServiceTy service = servicesMap.value(currServiceId);

                    qualityInfo.dabQualityInfo.serviceSubCh = service.SubChID;
                    qualityInfo.dabQualityInfo.qualServiceBitRate = (quint8)service.ServiceBitrate;
                }
                else
                {
                    qualityInfo.dabQualityInfo.serviceSubCh = INVALID_DATA_BYTE;
                    qualityInfo.dabQualityInfo.qualServiceBitRate = INVALID_DATA_BYTE;
                }
            }
            else
            {
                qualityInfo.sync = false;
            }

            qualityInfo.syncLevel = ensTable.EnsTuneSync;

            SendTuneSyncStatus(&qualityInfo);
        }
        else
        {
            qDebug() << "ERROR: WrongEnsemble Index = " << currEnsembleIndex;
        }
    }
}

void Middleware::DecodeResponses(QByteArray data, MdwCommandCodeTy m_commandCode)
{
    switch (m_commandCode)
    {
        case MIDW_GET_ENSEMBLE_LIST:
        {
            QByteArray payload = data.mid(DataMessages::GetMdwResponseObj()->respHeaderLenght, DataMessages::GetMdwResponseObj()->respPayloadLenght);

            // Each ensemble data is 7 bytes long, calculate the number of ensemble
            quint8 ensNumber = (payload.size() / 7);
            quint32 currentFreq;

            for (int i = 0; i < ensNumber; i++)
            {
                int offset = 7 * i;

                QString ensembleIdString = ByteArrayToString(payload.mid(offset, 3));
                QString FrequencyHex = ByteArrayToString(payload.mid(offset + 3, 4));

                // Format data
                ensembleIdString = DataMessages::FormatMdwCommandString(ensembleIdString);

                FrequencyHex = DataMessages::FormatMdwCommandString(FrequencyHex);
                currentFreq = getDab3FreqFromStrHexFreq(FrequencyHex);

                // Get the selected EnsembleIndex from the retrieved ensemble frequency
                int ensIndex = getDab3IndexFromFrequency(currentFreq);

                if (currEnsembleIndex == ensIndex)
                {
                    // Save ensemble data
                    EnsembleTableTy ensTable = ensTableLst->at(currEnsembleIndex);
                    ensTable.EnsECCID = ensembleIdString;

                    // Retrieve the ensemble ID as number
                    bool ok;
                    ensTable.ensembleUniqueId = ensTable.EnsECCID.remove(" ").toUInt(&ok, 16);

                    if (false == ok)
                    {
                        // Parsing failed, handle error here
                        qDebug() << "ERROR: ensemble ID conversion to number failed";

                        // Set the ensemble number to invalid ensemble
                        ensTable.ensembleUniqueId = INVALID_ENSEMBLE_ID;
                    }

                    // Save ensemble into list
                    ensTableLst->replace(currEnsembleIndex, ensTable);

                    break;
                }
            }
        }
        break;

        case MIDW_GET_ENSEMBLE_DATA:
        {
            EnsembleTableTy ensTable = ensTableLst->at(currEnsembleIndex);
            ensTable.EnsCharset = (quint8)data[7];
            ensTable.EnsChLabel =  toQStringUsingCharset(data.mid(8, 16), ensTable.EnsCharset, -1);     // QString(data.mid(8, 16)).toUtf8();

            ensTableLst->replace(currEnsembleIndex, ensTable);

            SendEnsembleName(&ensTableLst->at(currEnsembleIndex));
        }
        break;

        case MIDW_GET_SERVICE_LIST:
        {
            quint8 servNumber = (DataMessages::GetMdwResponseObj()->respPayloadLenght / 4);

            // Save new services data
            if (servNumber > 0)
            {
                for (int i = 0; i < servNumber; i++)
                {
                    ServiceTy service;

                    // Initialize all fields (except serviceUniqueId that is initialized inside UpdateServiceList)
                    service.ServiceID = ByteArrayToString(data.mid((i + 1) * 4, 4));
                    service.ServiceID = DataMessages::FormatMdwCommandString(service.ServiceID);

                    service.ServiceBitrate = INVALID_DATA_U16;
                    service.SubChID = INVALID_DATA_BYTE;
                    service.ServiceLabel.clear();
                    service.ServiceCharset = 0;

                    // Check if we already have this service (use unique ID)
                    UpdateServiceList(service);
                }
            }
        }
        break;

        case MIDW_GET_SPECIFIC_SERVICE_DATA:
        {
            // We need to check if the command was correct
            if (COMMAND_COMPLETED_CORRECTLY == DataMessages::GetMdwResponseObj()->respStatusCode)
            {
                ServiceTy service;
                const ServiceListTy* serviceListPtr = &serviceLst->at(currEnsembleIndex);
                //    const EnsembleTableTy* ensTablePtr = &ensTableLst->at(currEnsembleIndex);
                bool ok;

                // Get current service id from response
                service.ServiceID = ByteArrayToString(data.mid(7, 4));
                service.ServiceID = DataMessages::FormatMdwCommandString(service.ServiceID);

                service.ServiceBitrate      = data.mid(11, 2).toHex().toInt(&ok, 16);
                service.SubChID             = data[13];
                service.ServiceLabel        = toQStringUsingCharset(data.mid(21, 16), service.ServiceCharset, -1);  //QString(data.mid(21, 16)).toUtf8();

                // Get current PTY (dab Pty Static International Code)
                service.ServicePty          = data[47];

                // Check if we already have this service (use unique ID)
                UpdateServiceList(service);

                // Send the service list to upper layers
                SendServiceList(serviceListPtr);
            }
        }
        break;

        case MIDW_GET_PAD_DLS:
        {
            if (true == newPadDlsData)
            {
                QString paddls_str;
                QByteArray data_to_decode;
                quint8 data_1;

                if (data[0] & 0x08)
                {
                    data_to_decode = QByteArray(data.mid(7, -1));
                    data_1 = (quint8)data[5];
                }
                else
                {
                    data_to_decode = QByteArray(data.mid(6, -1));
                    data_1 = (quint8)data[4];
                }

                if (data_1 == 0x02)   // If PAD_DLS type (DAB)
                {
                    quint8 charset_type = (quint8)data[5];

                    paddls_str = toQStringUsingCharset(data_to_decode.constData(), charset_type, -1);
                }

                SendDls(paddls_str);

                emit inShellMwData_signal("\n[i] Response info:DLS = " + paddls_str, " ", RADIO_DATA_COMMAND);
            }
        }
        break;

        case MIDW_TUNE_FREQUENCY:
        {
            if (MAX_NB_DAB_FREQUENCIES == currEnsembleIndex)
            {
                break;
            }

            EnsembleTableTy ensTable = ensTableLst->at(currEnsembleIndex);

            // Save new data
            if (data.length() > 8)
            {
                ensTable.EnsTune_Ber_Fic = (data.at(5) >> 5) & 0x07;
                ensTable.EnsTuneSync = data.at(8) & 0x07;
                ensTable.EnsTxMode = data.at(4) & 0x07;
            }
            else
            {
                ensTable.EnsTune_Ber_Fic = INVALID_DATA_BYTE;
                ensTable.EnsTuneSync = INVALID_DATA_BYTE;
                ensTable.EnsTxMode = 0;
            }

            // Save ensemble
            ensTableLst->replace(currEnsembleIndex, ensTable);
        }
        break;

        case MIDW_SERVICE_SELECT:
        {
            SendServiceSelect("OK");
        }
        break;

        case MIDW_SEEK:
        {
            // Update current ensemble index from tuned frequency
            currEnsembleIndex = getDab3IndexFromFrequency(activeFreq);

            // Get ensemble
            EnsembleTableTy ensTable = ensTableLst->at(currEnsembleIndex);
            ServiceListTy currList = serviceLst->at(currEnsembleIndex);

            // Clear old data ---> do not clear 'ensFrequency'
            ensTable.EnsChLabel.clear();
            ensTable.EnsECCID.clear();
            ensTable.EnsTuneSync = 0;
            ensTable.EnsTune_Ber_Fic = 0;

            // Clear service list
            currList.serviceList.clear();

            // Clear the service map for the current ensemble
            servicesMap.clear();

            // Save new data
            if (data.length() > 9)
            {
                ensTable.EnsTuneSync = data.at(9);
            }
            else
            {
                ensTable.EnsTuneSync = INVALID_DATA_BYTE;
            }

            // Save ensemble
            ensTableLst->replace(currEnsembleIndex, ensTable);
        }
        break;

        case MIDW_GET_DAB_INFO:
            if (true == newDabQualData)
            {
                SaveDabQualityData(data);

                emit inShellMwData_signal("\n[r] Response Dab Quality = ", utility->processTextFormat(data, HEX_FORMAT), RADIO_DATA_RESPONSE);
            }

            break;

        default:
            qDebug() << "ERROR: Unknown response code received";
            break;
    }
}

void Middleware::UpdateServiceList(ServiceTy& service)
{
    //EnsembleTableTy* currentEnsemblePtr = const_cast<EnsembleTableTy *> (&ensTableLst->at(currEnsembleIndex));
    ServiceListTy* currList = const_cast<ServiceListTy *> (&serviceLst->at(currEnsembleIndex));

    // Retrieve the ensemble ID as number
    bool ok;

    service.serviceUniqueId = service.ServiceID.remove(" ").toUInt(&ok, 16);

    if (true == ok)
    {
        // Remove element to update it
        servicesMap.remove(service.serviceUniqueId);

        // Update the service map
        servicesMap.insert(service.serviceUniqueId, service);

        // Update global table
        currList->serviceList.clear();

        foreach(auto service, servicesMap)
        {
            currList->serviceList.append(service);
        }
    }
    else
    {
        // Parsing failed, handle error here
        qDebug() << "ERROR: service ID number conversion to number failed";

        // Set the ensemble number to invalid ensemble
        service.serviceUniqueId = INVALID_SERVICE_ID;
    }
}

void Middleware::SaveMiddlewareResponseData(MdwDataPacketTy receivedData)
{
    MessageTypeTy msgType = receivedData.msgType;

    switch (msgType)
    {
        case MDW_COMMAND:
        {
            DataMessages::GetMdwCommandObj()->cmdCode             = receivedData.mdwCommand.cmdCode;
            DataMessages::GetMdwCommandObj()->cmdName             = receivedData.mdwCommand.cmdName;
            DataMessages::GetMdwCommandObj()->cmdStr              = receivedData.mdwCommand.cmdStr;
            DataMessages::GetMdwCommandObj()->cmdStatusCode       = receivedData.mdwCommand.cmdStatusCode;
            DataMessages::GetMdwCommandObj()->cmdStatusCodeName   = receivedData.mdwCommand.cmdStatusCodeName;
            DataMessages::GetMdwCommandObj()->cmdNeedResponse     = receivedData.mdwCommand.cmdNeedResponse;
        }
        break;

        case MDW_NOTIFICATION:
        {
            // Save notification data
            DataMessages::GetMdwNotificationObj()->notifCmdCode          = receivedData.mdwNotif.notifCmdCode;
            DataMessages::GetMdwNotificationObj()->notifCmdCodeName      = receivedData.mdwNotif.notifCmdCodeName;
            DataMessages::GetMdwNotificationObj()->notifStatusCode       = receivedData.mdwNotif.notifStatusCode;
            DataMessages::GetMdwNotificationObj()->notifStatusCodeName   = receivedData.mdwNotif.notifStatusCodeName;
            DataMessages::GetMdwNotificationObj()->notifData             = receivedData.mdwNotif.notifData;
            DataMessages::GetMdwNotificationObj()->notifDataStr          = receivedData.mdwNotif.notifDataStr;
            DataMessages::GetMdwNotificationObj()->notifHeaderLenght     = receivedData.mdwNotif.notifHeaderLenght;
            DataMessages::GetMdwNotificationObj()->notifPayloadLenght    = receivedData.mdwNotif.notifPayloadLenght;
            DataMessages::GetMdwNotificationObj()->notifStatusCodeAction = receivedData.mdwNotif.notifStatusCodeAction;

            DecodeImmediateAnswer(receivedData.mdwNotif.notifData, receivedData.mdwNotif.notifCmdCode);

            isMwNotifStatusReceived = true;
        }
        break;

        case MDW_AUTONOTIFICATION:
        {
            // Get auto-notification data
            MdwAutoNotificationTy node;

            node.autoNotifData              = receivedData.mdwAutoNotif.autoNotifData;
            node.autoNotifDataStr           = receivedData.mdwAutoNotif.autoNotifDataStr;
            node.autoNotifHeaderLenght      = receivedData.mdwAutoNotif.autoNotifHeaderLenght;
            node.autoNotifPayloadLenght     = receivedData.mdwAutoNotif.autoNotifPayloadLenght;
            node.autoNotifStatusCode        = receivedData.mdwAutoNotif.autoNotifStatusCode;
            node.autoNotifStatusCodeName    = receivedData.mdwAutoNotif.autoNotifStatusCodeName;
            node.autoNotifStatusCodeAction  = receivedData.mdwAutoNotif.autoNotifStatusCodeAction;

            DecodeAutoNotify(receivedData.mdwAutoNotif.autoNotifData);
        }
        break;

        case MDW_AUTORESPONSE:
        {
            DataMessages::GetMdwAutoResponseObj()->respCmdCode             = receivedData.mdwAutoResponse.respCmdCode;
            DataMessages::GetMdwAutoResponseObj()->respCmdCodeName         = receivedData.mdwAutoResponse.respCmdCodeName;
            DataMessages::GetMdwAutoResponseObj()->respData                = receivedData.mdwAutoResponse.respData;
            DataMessages::GetMdwAutoResponseObj()->respDataStr             = receivedData.mdwAutoResponse.respDataStr;
            DataMessages::GetMdwAutoResponseObj()->respHeaderLenght        = receivedData.mdwAutoResponse.respHeaderLenght;
            DataMessages::GetMdwAutoResponseObj()->respPayloadLenght       = receivedData.mdwAutoResponse.respPayloadLenght;
            DataMessages::GetMdwAutoResponseObj()->respStatusCode          = receivedData.mdwAutoResponse.respStatusCode;
            DataMessages::GetMdwAutoResponseObj()->respStatusCodeName      = receivedData.mdwAutoResponse.respStatusCodeName;
            DataMessages::GetMdwAutoResponseObj()->respStatusCodeAction    = receivedData.mdwAutoResponse.respStatusCodeAction;

            if (receivedData.mdwAutoResponse.respCmdCode == MIDW_GET_PAD_DLS)
            {
                newPadDlsData = true;
            }
            else
            {
                newPadDlsData = false;

                if (receivedData.mdwAutoResponse.respCmdCode == MIDW_GET_DAB_INFO)
                {
                    newDabQualData = true;
                }
                else
                {
                    newDabQualData = false;
                    isMwRespReceived = true;

                    return;
                }
            }

            DecodeResponses(receivedData.mdwAutoResponse.respData, receivedData.mdwAutoResponse.respCmdCode);

            isMwRespReceived = true;
        }
        break;

        case MDW_RESPONSE:
        {
            DataMessages::GetMdwResponseObj()->respCmdCode             = receivedData.mdwResponse.respCmdCode;
            DataMessages::GetMdwResponseObj()->respCmdCodeName         = receivedData.mdwResponse.respCmdCodeName;
            DataMessages::GetMdwResponseObj()->respData                = receivedData.mdwResponse.respData;
            DataMessages::GetMdwResponseObj()->respDataStr             = receivedData.mdwResponse.respDataStr;
            DataMessages::GetMdwResponseObj()->respHeaderLenght        = receivedData.mdwResponse.respHeaderLenght;
            DataMessages::GetMdwResponseObj()->respPayloadLenght       = receivedData.mdwResponse.respPayloadLenght;
            DataMessages::GetMdwResponseObj()->respStatusCode          = receivedData.mdwResponse.respStatusCode;
            DataMessages::GetMdwResponseObj()->respStatusCodeName      = receivedData.mdwResponse.respStatusCodeName;
            DataMessages::GetMdwResponseObj()->respStatusCodeAction    = receivedData.mdwResponse.respStatusCodeAction;

            DecodeResponses(receivedData.mdwResponse.respData, receivedData.mdwResponse.respCmdCode);

            isMwRespReceived = true;
        }
        break;

        case MDW_ASCII_DATA_ON_DATACHANNEL:
        case MDW_FIC_BINARY_DATA_ON_DATACHANNEL:
        case MDW_RAW_BINARY_DATA_ON_DATACHANNEL:
        case MDW_DECODED_BINARY_DATA_ON_DATACHANNEL:
        case MDW_AUDIO_BINARY_DATA_ON_DATACHANNEL:
        default:
            // No code
            break;
    }
}

QString Middleware::ByteArrayToString(QByteArray data)
{
    return utility->processTextFormat(data, HEX_FORMAT);
}

EnsembleTableTy Middleware::GetCurrEnsembleTable()
{
    if (currEnsembleIndex < ensTableLst->size() && currEnsembleIndex >= 0)
    {
        return ensTableLst->at(currEnsembleIndex);
    }

    return ensTableLst->at(0);
}

ServiceListTy Middleware::GetCurrServiceList()
{
    if (currEnsembleIndex < serviceLst->size() && currEnsembleIndex >= 0)
    {
        return serviceLst->at(currEnsembleIndex);
    }

    return serviceLst->at(0);
}

void Middleware::mwFreqTablesSetup()
{
    // DAB BAND-III (40 channels + 1 (0 Hz))
    QList<quint32> Dab3FrequencyList;

    Dab3FrequencyList << 174928 << 176640 << 178352 << 180064 << 181936
                      << 183648 << 185360 << 187072 << 188928 << 190640
                      << 192352 << 194064 << 195936 << 197648 << 199360
                      << 201072 << 202928 << 204640 << 206352 << 208064
                      << 209936 << 210096 << 211648 << 213360 << 215072
                      << 216928 << 217088 << 218640 << 220352 << 222064
                      << 223936 << 224096 << 225648 << 227360 << 229072
                      << 230784 << 232496 << 234208 << 235776 << 237488
                      << 239200 << 0;

    for (int i = 0; i < (MAX_NB_DAB_FREQUENCIES + 1); i++)
    {
        // Initialize the Band_III_Freq list
        GetDab3BandFromFreq()->insert(i, Dab3FrequencyList[i]);

        // Initialize EnsTable list
        EnsembleTableTy ensTable;
        ServiceListTy serviceList;

        ensTable.ensFrequency = Dab3FrequencyList[i];
        ensTable.EnsChLabel.clear();
        ensTable.EnsECCID.clear();
        ensTable.ensembleUniqueId = INVALID_ENSEMBLE_ID;
        ensTable.EnsTuneSync = 0;
        ensTable.EnsTune_Ber_Fic = 0;
        serviceList.serviceList.clear();

        // Insert the newly created ensemble table in the ensemble table list that stores all ensemble data
        ensTableLst->insert(i, ensTable);
        serviceLst->insert(i, serviceList);
    }
}

QString Middleware::getFreqHexStrFromFrequency(quint32 curFreq)
{
    QString frStr;

    frStr = QString::number(curFreq, 16);

    while (frStr.size() < 8)
    {
        frStr = "0" + frStr;
    }

    for (int cnt = 2; cnt <= frStr.size(); cnt += 2)
    {
        frStr.insert(cnt, " ");
    }

    frStr = " " + frStr;

    return frStr;
}

int Middleware::getDab3IndexFromFrequency(quint32 curFreq)
{
    QList<quint32>* freqList = GetDab3BandFromFreq();
    int ensIndex = 0;

    for (int idx = 0; idx < freqList->size(); idx++)
    {
        if (freqList->at(idx) == curFreq)
        {
            ensIndex = idx;

            break;
        }
    }

    return ensIndex;
}

quint32 Middleware::getDab3FreqFromIndex(int freqIdx)
{
    QList<quint32>* freqList = GetDab3BandFromFreq();
    quint32 ensFr = 0;

    for (int idx = 0; idx < freqList->size(); idx++)
    {
        if (freqIdx == idx)
        {
            ensFr = freqList->at(idx);

            break;
        }
    }

    return ensFr;
}

quint32 Middleware::getDab3FreqFromStrHexFreq(QString strHexFr)
{
    bool ok;

    QString FreqHexV = strHexFr.mid(0, 2) +
        strHexFr.mid(3, 2) +
        strHexFr.mid(6, 2) +
        strHexFr.mid(9, 2);

    quint32 ensFr = FreqHexV.toUInt(&ok, 16);

    return ensFr;
}

void Middleware::SendPing()
{
    eventDataInterface eventData;

    eventData.dataPtr = (unsigned char *)NULL;
    eventData.size = 0;
    eventData.eventType = EVENTS_RX_PING;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Middleware::SendTuneSyncStatus(qualityInfoTy* qualInfo)
{
    qDebug() << Q_FUNC_INFO;

    eventDataInterface eventData;

    eventData.dataPtr = (unsigned char *)qualInfo;
    eventData.size = sizeof (qualityInfoTy);
    eventData.eventType = EVENTS_RX_SYNC_LEVEL;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Middleware::SendLastServiceList()
{
    const ServiceListTy* serviceList = &serviceLst->at(currEnsembleIndex);
    eventDataInterface eventData;

    eventData.dataPtr = (unsigned char *)serviceList;
    eventData.size = sizeof (ServiceListTy);
    eventData.eventType = EVENTS_UPDATE_SERVICE_LIST;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Middleware::SendServiceList(const ServiceListTy* serviceList)
{
    eventDataInterface eventData;

    eventData.dataPtr = (unsigned char *)serviceList;
    eventData.size = sizeof (ServiceListTy);
    eventData.eventType = EVENTS_RX_SERVICE_LIST;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Middleware::SendEnsembleName(const EnsembleTableTy* ensTable)
{
    eventDataInterface eventData;

    eventData.dataPtr = (unsigned char *)ensTable;
    eventData.size = sizeof (EnsembleTableTy);
    eventData.eventType = EVENTS_RX_ENSEMBLE_NAME;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Middleware::SendServiceSelect(QString servStr)
{
    eventDataInterface eventData;
    QByteArray a;

    a.append(servStr);
    eventData.dataPtr = (unsigned char *)a.data();
    eventData.size = a.size();
    eventData.eventType = EVENTS_RX_SERVICESEL;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Middleware::SendDls(QString dlsStr)
{
    eventDataInterface eventData;
    QByteArray a;

    a.append(dlsStr);
    eventData.dataPtr = (unsigned char *)a.data();
    eventData.size = a.size();
    eventData.eventType = EVENTS_RX_DLS;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Middleware::SendSls(QByteArray slsArray)
{
    eventDataInterface eventData;

    eventData.dataPtr = (unsigned char *)slsArray.data();
    eventData.size = slsArray.size();
    eventData.eventType = EVENTS_RX_SLS;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Middleware::SaveDabQualityData(QByteArray data)
{
    quint8 syncValue;

    qualityInfo.dabQualityInfo.ficBer = ((quint8)data[17] << 24) | ((quint8)data[18] << 16) |
        ((quint8)data[19] << 8)  | ((quint8)data[20]);

    qualityInfo.dabQualityInfo.mscBer = ((quint8)data[29] << 24) | ((quint8)data[30] << 16) |
        ((quint8)data[31] << 8)  | ((quint8)data[32]);

    qualityInfo.dabQualityInfo.audioBer = ((quint8)data[21] << 24) | ((quint8)data[22] << 16) |
        ((quint8)data[23] << 8)  | ((quint8)data[24]);

    qualityInfo.dabQualityInfo.baseband_level = (quint8)data[46];       // Use oly 8LSB of BaseBand Level

    if (data.size() ==  55)
    {
        qualityInfo.dabQualityInfo.audioCRCError = (quint8)data[43];
        qualityInfo.dabQualityInfo.audioCRCTotal = 1;
    }
    else
    {
        qualityInfo.dabQualityInfo.audioCRCError = (quint8)data[47];
        qualityInfo.dabQualityInfo.audioCRCTotal = (quint8)data[48];
    }

    // Calculate sync value
    syncValue = ((quint8)data[41] >> 4);

    if (SYNC_DECODE_MCI == syncValue)
    {
        qualityInfo.sync = true;
    }
    else
    {
        qualityInfo.sync = false;
    }
}

// End of file
