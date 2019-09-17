#ifndef CMD_MNGR_MW_H_
#define CMD_MNGR_MW_H_

#include <QMap>

#include "common.h"
#include "utilities.h"

#include "event_manager.h"
#include "tcptransportlayer.h"

#include "cmd_mngr_base.h"

#define SYNC_NO_SIGNAL              ((unsigned char)0x00)
#define SYNC_DAB_SIGNAL             ((unsigned char)0x01)
#define SYNC_FULLY_SYNC             ((unsigned char)0x03)
#define SYNC_DECODE_MCI             ((unsigned char)0x07)

class Middleware : public QObject, public DataMessages, public DataChannel, public CmdMngrBase
{
    Q_OBJECT

    public:
        Middleware(QObject* parent, QString _dataMsgInstName = "MW_msg", QString _dataChInstName = "MW_ch");

        ~Middleware();

    public:
        QList<quint32>* GetDab3BandFromFreq() { return dabBandIIIFreqList; }

        QList<EnsembleTableTy>* GetEnsTableLst_Obj() { return ensTableLst; }

        QList<ServiceListTy>* GetServiceLst_Obj() { return serviceLst; }

        int GetCurrEnsembleIndex() { return currEnsembleIndex; }

        EnsembleTableTy GetCurrEnsembleTable();
        ServiceListTy GetCurrServiceList();

        void SetServiceRemoveMode(bool removeFlag) { removeServiceFlag = removeFlag; }

        void SetQualAcquireStatus(bool startFlag) { startReadQualFlag = startFlag; }

        bool GetSyncStatus() { return qualityInfo.sync; }

        quint32 getDab3FreqFromIndex(int freqIdx);

        RadioSequencesStatusTy CommandExecute(MdwCommandCodeTy commandCode, bool waitResponse = false);

        void SetCurrServiceId(unsigned int serviceId) { currServiceId = serviceId; }

        void SetMuteFlag(bool flag) { muteCommandFlag = flag; }

        void SetActiveFrequency(quint32 freq)
        {
            activeFreq = freq;

            if (0 != freq)
            {
                SetCurrEnsembleIndex(getDab3IndexFromFrequency(freq));
            }
        }

        void SelectPadOnDlsState(bool cmd) { padStart = cmd; }

        void SelectSlsOnXpadState(bool cmd) { slsStart = cmd; }

        void SetEventAutonotificationRunning(bool value) { isEventAutonotificationRunning = value; }

        dabQualityInfoTy getQualityInfo() { return qualityInfo.dabQualityInfo; }

        void SendLastServiceList();

        void seekHandler(void* status) { Q_UNUSED(status); }

    private:
        QList<EnsembleTableTy>* ensTableLst;
        QList<ServiceListTy>* serviceLst;
        QList<quint32>* dabBandIIIFreqList;

        Utilities* utility;

        EventManager<eventsList, eventDataInterface>* eventManager;

        int WrittenByte;

        bool isMwNotifTimeout;
        bool isMwRespTimeout;

        bool isMwNotifStatusReceived;
        bool isMwRespReceived;

        bool cmdNeedResponse;
        bool cmdNeedAutoResponse;

        bool isEventAutonotificationRunning;
        bool muteCommandFlag;
        quint32 activeFreq;
        int currEnsembleIndex;
        unsigned int currServiceId;
        bool newPadDlsData;
        bool isThereNewSlsOnXpadImage;

        bool padStart;
        bool slsStart;

        bool removeServiceFlag;
        bool startReadQualFlag;
        bool newDabQualData;

        qualityInfoTy qualityInfo;

        // Service map for a specific ensemble
        QMap<unsigned int, ServiceTy> servicesMap;

        void SendPing();
        void SendTuneSyncStatus(qualityInfoTy* tuneSyncLevel);
        void SendEnsembleName(const EnsembleTableTy* ensTable);
        void SendServiceList(const ServiceListTy* serviceList);
        void SendServiceSelect(QString servStr);
        void SendDls(QString dlsStr);
        void SendSls(QByteArray slsArray);

        void SetCurrEnsembleIndex(int m_currEnsIndex) { currEnsembleIndex = m_currEnsIndex; }

        QString getFreqHexStrFromFrequency(quint32 curFreq);
        int getDab3IndexFromFrequency(quint32 curFreq);
        QString ByteArrayToString(QByteArray);
        quint32 getDab3FreqFromStrHexFreq(QString strHexFr);

        MdwDataPacketTy SaveMiddlewareReceivedData(QByteArray, StatusCodeTy *);
        StatusCodeTy MdwCmdParser(QString& command, QByteArray *);
        StatusCodeTy MdwCmdTransmitter(QByteArray, bool waitResponse = false);

        void mwCommandExecute(QString command, bool waitResponse = false);
        QStringList CommandPrepare(MdwCommandCodeTy m_commandCode);
        RadioSequencesStatusTy CommandSend(MdwCommandCodeTy commandCode, QString commandString, bool waitResponse = false);

        RadioSequencesStatusTy GetMdwResponse();
        RadioSequencesStatusTy GetMdwNotification();

        void DecodeImmediateAnswer(QByteArray data, MdwCommandCodeTy m_commandCode);
        void DecodeAutoNotify(QByteArray data);
        void DecodeResponses(QByteArray data, MdwCommandCodeTy m_commandCode);

        void SaveMiddlewareResponseData(MdwDataPacketTy receivedData);

        quint32 GetFreqDabFromIndex(int ensIndex);

        QString GetCurrEnsembleFreqNumber(int channelIndex);

        void mwFreqTablesSetup();
        QString getFreqHexStr(quint32 currFreq);
        int EnsFreqNumberToEnsIndex(QString ensFreqNumber);
        int EnsFreqHexToEnsIndex(QString ensFreqHex);

        void mdwSendCmdExecStatus(StatusCodeTy cmdStatus);

        void UpdateServiceList(ServiceTy& service);
        void SaveDabQualityData(QByteArray data);
        void mwInitQualityInfo();

    public slots:
        // Internal timers
        void mwNotifTimer_timeout_slot();
        void mwRespTimer_timeout_slot();
        void mdw_receivedDataFromDcopTcpLayer_slot(QByteArray);

    signals:
        // To TCP/IP
        void mdw_sendCmdToDispatcher_signal(QByteArray);

        // To worker
        void inShellMwData_signal(QString, QString, int);
        void inShellMwError_signal(RadioSequencesStatusTy, QString, QString);
};

#endif // CMD_MNGR_MW_H_
