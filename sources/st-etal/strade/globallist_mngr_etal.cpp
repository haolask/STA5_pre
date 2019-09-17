#include "globallist_mngr_etal.h"
#include <QThread>
#include <QTimer>
#include <QEventLoop>

// prototypes
void etalRDSCallbackFg(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext);
void etalRDSCallbackBg(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext);
#if defined (ETAL_GLOBAL_LIST_INIT_ETAL)
void etalUserNotificationHandler(void* context, ETAL_EVENTS dwEvent, void* pstatus);
#endif // #if defined(ETAL_GLOBAL_LIST_INIT_ETAL)

// public methods
ETAL_globalList_mngr::ETAL_globalList_mngr(QObject* parent) : QObject(parent)
{
    // boot here
    // etal init && booting
#if defined (ETAL_GLOBAL_LIST_INIT_ETAL)
    qint32 ret, i, j;
    EtalHardwareAttr init_params;
    EtalHwCapabilities* p2cap;
    QString msg;
#endif // #if defined(ETAL_GLOBAL_LIST_INIT_ETAL)

#if defined (ETAL_GLOBAL_LIST_INIT_ETAL)
    memset(&init_params, 0x0, sizeof (EtalHardwareAttr));

    init_params.m_cbNotify = etalUserNotificationHandler;
    init_params.m_context  = this;

    if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_initialize() error, ret = ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

    if ((ret = etal_get_capabilities(&p2cap)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_get_capabilities() error, ret = ") + QString::number(ret);
    }

    for (i = 0; i < 2; i++)
    {
        qDebug() << "tuner " << i << ", type " << p2cap->m_Tuner[i].m_TunerDevice.m_deviceType;

        for (j = 0; j < p2cap->m_Tuner[i].m_TunerDevice.m_channels; j++)
        {
            qDebug() << "fe " << j << ", standards DAB " << *p2cap->m_Tuner[i].m_standards;
        }
    }
#endif //  #if defined(ETAL_GLOBAL_LIST_INIT_ETAL)

#if defined (ETAL_GLOBAL_LIST_DEBUG)
    m_etal_handler_receiver_fg = ETAL_INVALID_HANDLE;
    m_etal_handler_rds_fg      = ETAL_INVALID_HANDLE;
#endif // #if defined( ETAL_GLOBAL_LIST_DEBUG)

    m_etal_handler_receiver_bg = ETAL_INVALID_HANDLE;
    m_etal_handler_rds_bg      = ETAL_INVALID_HANDLE;
}

ETAL_globalList_mngr::~ETAL_globalList_mngr()
{
#if defined (ETAL_GLOBAL_LIST_INIT_ETAL)
    QString msg;
    qint32 ret;

    if ((ret = etal_deinitialize()) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_deinitialize() error, ret ") + QString::number(ret);
        qDebug() << msg;
    }

    emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);
#endif // #if defined(ETAL_GLOBAL_LIST_INIT_ETAL)
}

void ETAL_globalList_mngr::on(bool fm, bool dab, quint32 fm_fe, quint32 dab_fe)
{
    QString msg;

    msg = QString("\nBACKGROUND scan ON, fm  ") + ((fm) ? "enabled" : "disabled")  + QString(" dab ") + ((dab) ? "enabled" : "disabled");
    emit message("\n" + msg, "\n\n", ETAL_GLOBAL_LIST_MES_COLOR);

    m_fm_enable = fm;
    m_dab_enable = dab;

    m_fm_fe = fm_fe;
    m_dab_fe = dab_fe;

#if defined (ETAL_GLOBAL_LIST_DEBUG)
    m_fm_fe =  ETAL_FE_HANDLE_2;
#if 1
    // FG tuner
    //    setMode(FM, true);
    //    setFreq(91900, true);
    //    setRDS(true, true);

    // FG tuner
    setMode(FM);
    setFreq(91900);
    setRDS(true);
#else
    setMode(DAB, true);
    if (true == setFreq(227360, true))
    {
        QThread::sleep(1);
        updateServiceList(true);
        selectAudioService(getCurrentEnsemble(true), 0x232f);

        QThread::sleep(10);
    }
#endif
#endif // #if defined( ETAL_GLOBAL_LIST_DEBUG)
}

void ETAL_globalList_mngr::off()
{
    emit message("\nBACKGROUND scan OFF", "\n", ETAL_GLOBAL_LIST_MES_COLOR);
}

void ETAL_globalList_mngr::term()
{
    m_lock.lock();
    m_mode_done = true;
    m_lock.unlock();
    emit terminate();
}

void ETAL_globalList_mngr::freqFound(quint32 freq)
{
    m_freq = freq;
    m_found_freq = true;
    emit seekFreqFoundSignal();
}

void ETAL_globalList_mngr::fullcycleFound(quint32 freq)
{
    m_freq = freq;
    m_mode_done = true;

    // emit message( Q_FUNC_INFO, "full cycle signal\n", ETAL_GLOBAL_LIST_MES_COLOR);
    emit seekFullCycleSignal();
}

// private

ServiceListTy ETAL_globalList_mngr::handlerFM()
{
    ServiceListTy list;
    QString msg;
    bool first = true;

    if (!m_fm_enable)
    {
        return list;
    }

    setMode(FM);

    msg = "\tScanning FM ...\n";
    emit message(msg, "\n", ETAL_GLOBAL_LIST_MES_COLOR);

    QTime t;
    t.start();

    m_found_freq = false;

    for (;;)
    {
        QTimer timer;
        QEventLoop loop;

        timer.setSingleShot(true);

        //connect(this,  ETAL_globalList_mngr::seekFreqFoundSignal, &loop, QEventLoop::quit);
        //connect(this,  ETAL_globalList_mngr::seekFullCycleSignal, &loop, QEventLoop::quit);
        //connect(this, ETAL_globalList_mngr::terminate, &loop, QEventLoop::quit);
        //connect(&timer,  QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(this,
                         static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::seekFreqFoundSignal),
                         &loop,
                         static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        QObject::connect(this,
                         static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::seekFullCycleSignal),
                         &loop,
                         static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        QObject::connect(this,
                         static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::terminate),
                         &loop,
                         static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        QObject::connect(&timer,
                         &QTimer::timeout,
                         &loop,
                         static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        timer.start(40000);

        if (true == first)
        {
            start();
            first = false;
        }
        else
        {
            // !TODO workaround here to exit if auto seek is killed by fg app, bug in etal
            if (false == cont())
            {
                qDebug() << "End of scan detected (forced)";
                break;
            }
        }

        loop.exec();

        //disconnect(this,  ETAL_globalList_mngr::seekFreqFoundSignal, &loop, QEventLoop::quit);
        //disconnect(this,  ETAL_globalList_mngr::seekFullCycleSignal, &loop, QEventLoop::quit);
        QObject::disconnect(this,
                            static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::seekFreqFoundSignal),
                            &loop,
                            static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));
        QObject::disconnect(this,
                            static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::seekFullCycleSignal),
                            &loop,
                            static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        // here we have good freq or end cycle
        if ((true == m_mode_done) && (false == m_found_freq))
        {
            qDebug() << "End of scan detected";
            stop();
            break;
        }

        m_found_freq = false;

        // Use time for RDS data timeout
        //connect(this,  ETAL_globalList_mngr::rdsAcqDoneSignal, &loop, QEventLoop::quit);
        //connect(&timer,  QTimer::timeout, &loop, QEventLoop::quit);
        //connect(this, ETAL_globalList_mngr::terminate, &loop, QEventLoop::quit);
        QObject::connect(this,
                         static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::rdsAcqDoneSignal),
                         &loop,
                         static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        QObject::connect(&timer,
                         &QTimer::timeout,
                         &loop,
                         static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        QObject::connect(this,
                         static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::terminate),
                         &loop,
                         static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        // 5 seconds timeout
        timer.start(5000);

        setRDS(true);
        loop.exec();
        setRDS(false);

        //disconnect(this,  ETAL_globalList_mngr::rdsAcqDoneSignal, &loop, QEventLoop::quit);
        QObject::disconnect(this,
                            static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::rdsAcqDoneSignal),
                            &loop,
                            static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        if (true == m_mode_done)
        {
            qDebug() << "End of scan detected in rds acq";
            stop();
            break;
        }

        m_rds_data.freq = m_freq;

#if defined (GLOABLE_LIST_VERB)
        msg = "\tfreq " + QString::number(m_rds_data.freq) + \
            "\tPI " + m_rds_data.pi + \
            "\tPS " + m_rds_data.ps;

        emit message(msg, "\n", ETAL_GLOBAL_LIST_MES_COLOR);
#endif // #if defined(GLOABLE_LIST_VERB)

        // add to list
        ServiceTy service;

        service.frequency    = m_rds_data.freq;
        service.serviceUniqueId  = m_rds_data.pi.toInt(0, 16);
        service.ServiceID = m_rds_data.pi;
        service.ServiceLabel = m_rds_data.ps;
        service.ServiceBitrate  = 0;
        service.SubChID  = 0;

        list.serviceList.append(service);
    }

    msg = "\n\tTime elapsed: " + QString::number(t.elapsed()) + " ms";
    emit message(msg, "\n", ETAL_GLOBAL_LIST_MES_COLOR);

    msg = "\n\tFound services: " + QString::number(list.serviceList.count());
    emit message(msg, "\n", ETAL_GLOBAL_LIST_MES_COLOR);

    setMode(IDLE);
    return list;
}

// workaround has to in place since etal seek callback is not called on not valid frequencies
// #define ETAL_GLOBAL_LIST_DAB_SEEK_WORKAROUND
// #define ETAL_GLOBAL_LIST_DAB_SEEK_WORKAROUND_ITER 2
// #define ETAL_GLOBAL_LIST_DAB_SEEK_WORKAROUND_TOUT 300
#define ETAL_GLOBAL_LIST_DAB_SEEK_WORKAROUND_ITER 1
#define ETAL_GLOBAL_LIST_DAB_SEEK_WORKAROUND_TOUT 1000

ServiceListTy ETAL_globalList_mngr::handlerDAB()
{
    ServiceListTy list;
    QString msg;

    if (!m_dab_enable)
    {
        return list;
    }

    setMode(DAB);

    msg = "\n\tScanning DAB ...\n";
    emit message(msg, "\n", ETAL_GLOBAL_LIST_MES_COLOR);

    QTime t;
    t.start();

#if defined (ETAL_GLOBAL_LIST_DAB_SEEK_WORKAROUND)
    QList<quint32> dabFreqList;

    dabFreqList << 174928 << 176640 << 178352 << 180064 << 181936 \
                << 183648 << 185360 << 187072 << 188928 << 190640 \
                << 192352 << 194064 << 195936 << 197648 << 199360 \
                << 201072 << 202928 << 204640 << 206352 << 208064 \
                << 209936 << 210096 << 211648 << 213360 << 215072 \
                << 216928 << 217088 << 218640 << 220352 << 222064 \
                << 223936 << 224096 << 225648 << 227360 << 229072 \
                << 230784 << 232496 << 234208 << 235776 << 237488 \
                << 239200;

    foreach(quint32 freq, dabFreqList)
    {
        bool is_good =  false;
        quint32 cntr = ETAL_GLOBAL_LIST_DAB_SEEK_WORKAROUND_ITER;

        QThread::sleep(1);

        do
        {
            is_good = setFreq(freq);

            if (is_good)
            {
                break;
            }

            // QThread::usleep(ETAL_GLOBAL_LIST_DAB_SEEK_WORKAROUND_TOUT*1000);

            // decrement
            cntr--;
        }
        while (cntr);

        qDebug() << "DAB freq " << freq << " good " << is_good;

        if (is_good)
        {
            QThread::sleep(1);
            EnsembleTableTy ensTable;
            ensTable.ensFrequency = freq;
            getEnsembleInfo(&ensTable);
            updateServiceList(&ensTable);

#if defined (GLOABLE_LIST_VERB)
            qDebug() << "Ens name " << ensTable.EnsChLabel;
            qDebug() << "Ens freq " << ensTable.ensFrequency;
            qDebug() << "ECCID    " << ensTable.EnsECCID;
            qDebug() << "Ens id   " << QString("%1").arg(ensTable.ensembleUniqueId, 0, 16);

            foreach(ServiceTy service, ensTable.ServicesTableList)
            {
                qDebug() << "\t" << service.ServiceLabel << ", PID" << service.ServiceID << ", bitrate " << service.ServiceBitrate;
            }
#endif // #if defined(GLOABLE_LIST_VERB)
            list.append(ensTable);
        }
    }
#else
    bool first = true;
    m_found_freq = false;
    list.serviceList.clear();

    for (;;)
    {
        QTimer timer;
        QEventLoop loop;

        timer.setSingleShot(true);

        //connect(this,  ETAL_globalList_mngr::seekFreqFoundSignal, &loop, QEventLoop::quit);
        //connect(this,  ETAL_globalList_mngr::seekFullCycleSignal, &loop, QEventLoop::quit);
        //connect(this,  ETAL_globalList_mngr::terminate, &loop, QEventLoop::quit);
        //connect(&timer,  QTimer::timeout, &loop, QEventLoop::quit);
        QObject::connect(this,
                         static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::seekFreqFoundSignal),
                         &loop,
                         static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        QObject::connect(this,
                         static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::seekFullCycleSignal),
                         &loop,
                         static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        QObject::connect(this,
                         static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::terminate),
                         &loop,
                         static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        QObject::connect(&timer,
                         &QTimer::timeout,
                         &loop,
                         static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        timer.start(40000);

        if (true == first)
        {
            start();
            first = false;
        }
        else
        {
            cont();
        }

        loop.exec();

        //disconnect(this,  ETAL_globalList_mngr::seekFreqFoundSignal, &loop, QEventLoop::quit);
        //disconnect(this,  ETAL_globalList_mngr::seekFullCycleSignal, &loop, QEventLoop::quit);
        QObject::disconnect(this,
                            static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::seekFreqFoundSignal),
                            &loop,
                            static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));
        QObject::disconnect(this,
                            static_cast<void (ETAL_globalList_mngr::*)()> (&ETAL_globalList_mngr::seekFullCycleSignal),
                            &loop,
                            static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

        // here we have good freq or end cycle
        if ((true == m_mode_done) && (false == m_found_freq))
        {
            stop();
            break;
        }

        EnsembleTableTy ensTable;
        int cntr = 20;

        do
        {
            QThread::sleep(0.1);
            getEnsembleInfo(&ensTable);
            qDebug() << "ens " << ensTable.ensembleUniqueId << " label " << ensTable.EnsChLabel << " is empty " << ensTable.EnsChLabel.isEmpty();
        }
        while ((cntr--) && (ensTable.EnsChLabel.isEmpty() && (ensTable.ensembleUniqueId == 0xffffff)));

        // here we have good freq or end cycle
        if ((true == m_mode_done) && (false == m_found_freq))
        {
            stop();
            break;
        }

        m_found_freq = false;
        ensTable.ensFrequency = m_freq;

        cntr = 10;
        quint32 ser_numb;
        do
        {
            QThread::sleep(0.1);
            ser_numb = updateServiceList(&list);
        }
        while ((cntr--) && ((ser_numb != quint32(list.serviceList.count()) || (!list.serviceList.count()))));

#if defined (GLOABLE_LIST_VERB)
        QString msg = "\tEns name " + ensTable.EnsChLabel + "\n" + \
            "\tEns freq " + QString::number(ensTable.ensFrequency) + "\n" + \
            "\tECCID    " + QString("%1").arg(ensTable.EnsECCID, 0, 16) + "\n" + \
            "\tEns id   " + QString("%1").arg(ensTable.ensembleUniqueId, 0, 16) + "\n";

        if (list.serviceList.isEmpty())
        {
            emit message(msg, "\tServices: 0 \n", ETAL_GLOBAL_LIST_MES_COLOR);
        }
        else
        {
            emit message(msg, "\tServices:\n", ETAL_GLOBAL_LIST_MES_COLOR);

            foreach(ServiceTy service, list.serviceList)
            {
                msg = "\t\t" + service.ServiceLabel + ", PID" + service.ServiceID + \
                    ", bitrate " + QString::number(service.ServiceBitrate) + \
                    ", charset " + QString::number(service.ServiceCharset);

                emit message(msg, "\n", ETAL_GLOBAL_LIST_MES_COLOR);

                list.serviceList.append(service);
            }
        }
#endif // #if defined(GLOABLE_LIST_VERB)
        emit message("", "\n", ETAL_GLOBAL_LIST_MES_COLOR);

        // here we have good freq or end cycle
        if (true == m_mode_done)
        {
            stop();
            break;
        }
    }
#endif // #if defined(ETAL_GLOBAL_LIST_DAB_SEEK_WORKAROUND)

    msg = "\n\tTime elapsed: " + QString::number(t.elapsed()) + " ms";
    emit message(msg, "\n", ETAL_GLOBAL_LIST_MES_COLOR);

    //    msg = "\n\tFound ensembles: " + QString::number(list.serviceList.count());
    //    emit message(msg, "\n", ETAL_GLOBAL_LIST_MES_COLOR);

    //    for (int i = 0; i < list.serviceList.count(); i++)
    //    {
    //        msg = "\ttensemble: " + QString::number(i) + ", services: " + QString::number(list. .count());
    //        emit message(msg, "\n", ETAL_GLOBAL_LIST_MES_COLOR);
    //    }

    setMode(IDLE);
    return list;
}

bool ETAL_globalList_mngr::setFreq(quint32 freq, bool fg)
{
    qint32 ret;
    bool is;

#if defined (ETAL_GLOBAL_LIST_DEBUG)
    ETAL_HANDLE* etal_handler_receiver = (true == fg) ? &m_etal_handler_receiver_fg : &m_etal_handler_receiver_bg;
#else
    Q_UNUSED(fg);
    ETAL_HANDLE* etal_handler_receiver = &m_etal_handler_receiver_bg;
#endif

    ret = etal_tune_receiver(*etal_handler_receiver, freq);

    switch (ret)
    {
        case ETAL_RET_SUCCESS:
            is = true;
            break;

        case ETAL_RET_NO_DATA:
        default:
            is = false;
            break;
    }

    m_freq = freq;

    return is;
}

void ETAL_globalList_mngr::setMode(Mode mode, bool fg)
{
    EtalReceiverAttr attr;
    EtalProcessingFeatures processingFeatures;
    QString          msg;
    qint32           ret;

    quint32             fe        = (mode == DAB) ? m_dab_fe : m_fm_fe;
    EtalBcastStandard   bcast_std = (mode == DAB) ? ETAL_BCAST_STD_DAB : ETAL_BCAST_STD_FM;
    EtalFrequencyBand   band      = (mode == DAB) ? ETAL_BAND_DAB3 : ETAL_BAND_FMEU;
    EtalAudioSourceTy   src       = (mode == DAB) ? ETAL_AUDIO_SOURCE_DCOP_STA660 : ETAL_AUDIO_SOURCE_STAR_AMFM;
    quint32             freq      = (mode == DAB) ? ETAL_GLOBAL_LIST_DAB_START_FREQ : ETAL_GLOBAL_LIST_FM_START_FREQ;

#if defined (ETAL_GLOBAL_LIST_DEBUG)
    ETAL_HANDLE* etal_handler_receiver = (true == fg) ? &m_etal_handler_receiver_fg : &m_etal_handler_receiver_bg;
#else
    ETAL_HANDLE* etal_handler_receiver = &m_etal_handler_receiver_bg;
#endif

    // destroy receiver if any
    if (*etal_handler_receiver != ETAL_INVALID_HANDLE)
    {
        if ((ret = etal_destroy_receiver(etal_handler_receiver)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_receiver() error, ret ") + QString::number(ret);
        }
        else
        {
            qDebug() << "BG rec destroyed";
        }

        emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);
        qDebug() << msg;
    }

    if (mode == IDLE)
    {
        return;
    }

    qDebug() << Q_FUNC_INFO << " mode " << ((mode == DAB) ? "DAB" : "FM");

    // set mode internally
    m_mode = mode;
    m_mode_done = false;

    // clean attr
    memset(&attr, 0x00, sizeof (EtalReceiverAttr));

    attr.m_FrontEndsSize = 1;
    attr.m_FrontEnds[0]  = fe;
    attr.m_Standard      = bcast_std;

    if ((ret = etal_config_receiver(etal_handler_receiver, &attr)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_config_receiver() error, ret ") + QString::number(ret);
        qDebug() << msg;
    }

    emit receiverUpdate(*etal_handler_receiver);
    emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

    // processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_FM_VPA;
    processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;

    if ((ret = etal_change_band_receiver(*etal_handler_receiver, band, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_change_band_receiver() error, ret ") + QString::number(ret);
        qDebug() << msg;
    }

    emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

    if ((ret = etal_tune_receiver(*etal_handler_receiver, freq)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_tune_receiver() error, ret ") + QString::number(ret);
        qDebug() << msg;
    }

    emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

    if (true == fg)
    {
        if ((ret = etal_audio_select(*etal_handler_receiver, src)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_audio_select() error, ret ") + QString::number(ret);
            qDebug() << msg;
        }

        emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);
    }

    m_last_freq = 0;
}

void ETAL_globalList_mngr::start()
{
    QString          msg;
    qint32           ret;

    if ((ret = etal_autoseek_start(m_etal_handler_receiver_bg, cmdDirectionUp, 100, cmdAudioMuted, dontSeekInSPS, true)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_autoseek_start() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_autoseek_start() done, ret ") + QString::number(ret);
    }

    qDebug() << msg;

    emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);
}

bool ETAL_globalList_mngr::cont()
{
    QString          msg;
    qint32           ret;

    if ((ret = etal_autoseek_continue(m_etal_handler_receiver_bg)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_autoseek_continue() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_autoseek_continue() done, ret ") + QString::number(ret);
    }

    qDebug() << msg;

    emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

    return (ret == ETAL_RET_SUCCESS) ? true : false;
}

void ETAL_globalList_mngr::stop()
{
    QString          msg;
    qint32           ret;

    if ((ret = etal_autoseek_stop(m_etal_handler_receiver_bg, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_autoseek_stop() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_autoseek_stop() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);
}

void ETAL_globalList_mngr::setRDS(bool en, bool fg)
{
    EtalDataPathAttr dataPathAttr;
    QString msg;
    qint32 ret;

#if defined (ETAL_GLOBAL_LIST_DEBUG)
    ETAL_HANDLE* etal_handler_receiver = (true == fg) ? &m_etal_handler_receiver_fg : &m_etal_handler_receiver_bg;
    ETAL_HANDLE* etal_handler_rds = (true == fg) ? &m_etal_handler_rds_fg : &m_etal_handler_rds_bg;
#else
    ETAL_HANDLE* etal_handler_receiver = &m_etal_handler_receiver_bg;
    ETAL_HANDLE* etal_handler_rds = &m_etal_handler_rds_bg;
#endif

    if (en)
    {
        *etal_handler_rds = ETAL_INVALID_HANDLE;
        memset(&dataPathAttr, 0x00, sizeof (EtalDataPathAttr));
        dataPathAttr.m_receiverHandle = *etal_handler_receiver;
        dataPathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS;
        dataPathAttr.m_sink.m_context = this;
        dataPathAttr.m_sink.m_BufferSize = sizeof (EtalRDSData);

        if (true == fg)
        {
            dataPathAttr.m_sink.m_CbProcessBlock = etalRDSCallbackFg;
        }
        else
        {
            dataPathAttr.m_sink.m_CbProcessBlock = etalRDSCallbackBg;
        }

        if ((ret = etal_config_datapath(etal_handler_rds, &dataPathAttr)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_config_datapath() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_config_datapath() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

        if ((ret = etal_start_RDS(*etal_handler_receiver, 0, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_start_RDS() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_start_RDS() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

        if ((ret = etaltml_start_decoded_RDS(*etal_handler_receiver, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etaltml_start_decoded_RDS() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etaltml_start_decoded_RDS() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

        m_rds_data.freq = 0;
        m_rds_data.pi = QString(ETAL_GLOBAL_LIST_RDS_EMPTY_MSG);
        m_rds_data.ps = QString(ETAL_GLOBAL_LIST_RDS_EMPTY_MSG);
        // m_rds_data.ps = QString("not acquiring");
    }
    else
    {
        // first check data handler and destroy old if needed
        if (*etal_handler_rds != ETAL_INVALID_HANDLE)
        {
            if ((ret = etaltml_stop_decoded_RDS(*etal_handler_receiver, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
            {
                msg = Q_FUNC_INFO + QString(": etaltml_stop_decoded_RDS() error, ret ") + QString::number(ret);
            }
            else
            {
                msg = Q_FUNC_INFO + QString(": etaltml_stop_decoded_RDS() done, ret ") + QString::number(ret);
            }

            emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

            if ((ret = etal_stop_RDS(*etal_handler_receiver)) != ETAL_RET_SUCCESS)
            {
                msg = Q_FUNC_INFO + QString(": etal_stop_RDS() error, ret ") + QString::number(ret);
            }
            else
            {
                msg = Q_FUNC_INFO + QString(": etal_stop_RDS() done, ret ") + QString::number(ret);
            }

            emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

            if ((ret = etal_destroy_datapath(etal_handler_rds)) != ETAL_RET_SUCCESS)
            {
                msg = Q_FUNC_INFO + QString(": etal_destroy_datapath() error, ret ") + QString::number(ret);
            }
            else
            {
                msg = Q_FUNC_INFO + QString(": etal_destroy_datapath() done, ret ") + QString::number(ret);
            }

            emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);
        }
    }
}

void ETAL_globalList_mngr::handleRDS(EtalRDSData* prds)
{
    // check PS
    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PI) == ETAL_DECODED_RDS_VALID_PI)
    {
        m_rds_data.pi =  QString("%1").arg(prds->m_PI, 0, 16);
    }

    // check PS
    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
    {
        m_rds_data.ps = prds->m_PS;
        m_rds_data.ps.resize(8);
    }

    // if both ready emit signal
    if ((m_rds_data.pi != ETAL_GLOBAL_LIST_RDS_EMPTY_MSG) && (m_rds_data.ps != ETAL_GLOBAL_LIST_RDS_EMPTY_MSG))
    {
        m_rds_data.freq = m_freq;
        // qDebug() << "freq " << m_rds_data.freq << ", PI " << m_rds_data.pi << ", PS" << m_rds_data.ps;
        emit rdsAcqDoneSignal();
    }
}

quint32 ETAL_globalList_mngr::updateServiceList(ServiceListTy* list, bool fg)
{
    quint32         ens              = getCurrentEnsemble(fg);

    QList<quint32>  services         = getServiceList(ens, true, false, fg);
    qint32          ret;

    // clear list first
    list->serviceList.clear();

    Q_FOREACH (quint32 item, services)
    {
        EtalServiceInfo info;
        EtalServiceComponentList compList;
        QString label, msg;
        quint32 dummy;
        ServiceTy service;

        ret = etal_get_specific_service_data_DAB(ens, item, &info, &compList, &dummy);

        if (ret != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_get_specific_service_data_DAB() error, ret ") + QString::number(ret);
            continue; // if error go to the next service
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_get_specific_service_data_DAB() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

        switch (info.m_serviceLabelCharset)
        {
            case 0x0:
                label = QString::fromLatin1(&info.m_serviceLabel[0]);
                break;

            case 0x6:
                label = QString::fromLatin1(&info.m_serviceLabel[0]);
                break;

            case 0xf:
                label = QString::fromUtf8(&info.m_serviceLabel[0]);
                break;

            default:
                label = QString("not handled");
                break;
        }

        service.ServiceBitrate  = info.m_serviceBitrate;
        service.ServiceCharset  = info.m_serviceLabelCharset;
        service.serviceUniqueId = item;
        service.ServiceID       =  QString("%1").arg(item, 0, 16);
        service.SubChID         = services.indexOf(item);
        service.ServiceLabel    = label;

        // add service to table
        list->serviceList.append(service);
    }

    return services.count();
}

quint32 ETAL_globalList_mngr::getCurrentEnsemble(bool fg)
{
    quint32 ens;
    qint32  ret;
    QString msg;

#if defined (ETAL_GLOBAL_LIST_DEBUG)
    ETAL_HANDLE* etal_handler_receiver = (true == fg) ? &m_etal_handler_receiver_fg : &m_etal_handler_receiver_bg;
#else
    Q_UNUSED(fg);
    ETAL_HANDLE* etal_handler_receiver = &m_etal_handler_receiver_bg;
#endif  // ETAL_HANDLE *etal_handler_rds = (true == fg) ? &m_etal_handler_rds_fg : &m_etal_handler_rds_bg;

    ret = etal_get_current_ensemble(*etal_handler_receiver, &ens);

    if (ret != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_get_current_ensemble() error, ret ") + QString::number(ret);
        ens = 0;
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_get_current_ensemble() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

    return ens;
}

QList<quint32> ETAL_globalList_mngr::getServiceList(qint32 ens, bool audioServ, bool dataServ, bool fg)
{
    EtalServiceList servList;
    QString msg;
    qint32 ret;

    QList<quint32> list;
#if defined (ETAL_GLOBAL_LIST_DEBUG)
    ETAL_HANDLE* etal_handler_receiver = (true == fg) ? &m_etal_handler_receiver_fg : &m_etal_handler_receiver_bg;
#else
    Q_UNUSED(fg);
    ETAL_HANDLE* etal_handler_receiver = &m_etal_handler_receiver_bg;
#endif  // ETAL_HANDLE *etal_handler_rds = (true == fg) ? &m_etal_handler_rds_fg : &m_etal_handler_rds_bg;

    ret = etal_get_service_list(*etal_handler_receiver, ens, audioServ, dataServ, &servList);

    if (ret != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_get_service_list() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_get_service_list() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

    qDebug() << "Services: " << servList.m_serviceCount;

    if (ETAL_RET_NO_DATA != ret)
    {
        qDebug() << "Services: " << servList.m_serviceCount;
        for (quint32 i = 0; i < servList.m_serviceCount; i++)
        {
            list << servList.m_service[i];
            // qDebug() << "\t" << QString(" %1").arg(servList.m_service[i], 0, 16);
        }
    }
    else
    {
        qDebug() << "Services: 0 ";
    }

    return list;
}

#if defined (ETAL_GLOBAL_LIST_DEBUG)
void ETAL_globalList_mngr::selectAudioService(qint32 ens, quint16 id)
{
    qint32      ret;
    QString     msg;

    ret = etal_service_select_audio(m_etal_handler_rds_fg, ETAL_SERVSEL_MODE_SERVICE, ens, id, 0, 0);

    if (ret != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_service_select_audio() error, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);
}

#endif // #if defined (ETAL_GLOBAL_LIST_DEBUG)

void ETAL_globalList_mngr::seekHandler(void* state)
{
    EtalSeekStatus* status = (EtalSeekStatus *)state;

    m_lock.lock();

    qDebug() << "SEEK EVENT BG freq " << status->m_frequency << " good " << status->m_frequencyFound << " full cycle " << status->m_fullCycleReached << " status " << status->m_status;

    if (status->m_frequencyFound && (status->m_status == ETAL_SEEK_RESULT))
    {
        freqFound(status->m_frequency);
    }

    if ((status->m_status == ETAL_SEEK_RESULT) && (status->m_fullCycleReached))
    {
        fullcycleFound(status->m_frequency);
    }

    m_lock.unlock();
}

void ETAL_globalList_mngr::getEnsembleInfo(EnsembleTableTy* ensTable)
{
    QString msg;
    qint32 ret;
    quint32 ens = getCurrentEnsemble();
    quint8  charset;
    tChar  label[17];
    quint16 bitmap;

    ret = etal_get_ensemble_data(ens, &charset, &label[0],  &bitmap);

    if (ret != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_get_ensemble_data() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_get_ensemble_data() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), ETAL_GLOBAL_LIST_LOG_COLOR);

    // fill return structure
    ensTable->EnsChLabel = QString::fromLatin1(&label[0]);
    ensTable->EnsECCID   = "empty";
    ensTable->ensembleUniqueId = ens;
}

// friend function section
// used for ETAL callbacks
#if defined (ETAL_GLOBAL_LIST_INIT_ETAL)
void etalUserNotificationHandler(void* context, ETAL_EVENTS dwEvent, void* pstatus)
{
    ETAL_globalList_mngr* p2this = (ETAL_globalList_mngr *)context;

    if (dwEvent == ETAL_INFO_TUNE)
    {
        EtalTuneStatus* status = (EtalTuneStatus *)pstatus;
        qDebug() << Q_FUNC_INFO;
        qDebug() << "freq    "  << status->m_stopFrequency;
        qDebug() << "state   "  << status->m_sync;
        qDebug() << "service "  << status->m_serviceId;
    }
    else if (ETAL_INFO_SEEK == dwEvent)
    {
        EtalSeekStatus* status = (EtalSeekStatus *)pstatus;
        p2this->callbackHandler(status);
    }
    else if (ETAL_INFO_LEARN == dwEvent)
    {
        qDebug() << "learn call back";

        EtalLearnStatusTy* status = (EtalLearnStatusTy *)pstatus;

        if (ETAL_LEARN_FINISHED == status->m_status)
        {
            qDebug() << "Learn freq. numb: " << status->m_nbOfFrequency;

            //             for(quint32 i=0; i<status->m_nbOfFrequency; ++i)
            //             {
            //                qDebug() << "freq " <<  petal->m_freqList[i].m_frequency << ", fst " << petal->m_freqList[i].m_fieldStrength;
            //             }
        }
        else
        {
            qDebug() << "freq " <<  status->m_frequency << " status " << status->m_status;
        }
    }
    else
    {
        qDebug() << "Unhandled event " << dwEvent;
    }
}

#endif // #if defined(ETAL_GLOBAL_LIST_INIT_ETAL)

void etalRDSCallbackBg(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext)
{
    Q_UNUSED(status);
    Q_UNUSED(dwActualBufferSize);

    EtalRDSData* prds = (EtalRDSData *)pBuffer;
    ETAL_globalList_mngr* p2this = (ETAL_globalList_mngr *)pvContext;

    p2this->handleRDS(prds);
}

void etalRDSCallbackFg(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext)
{
    Q_UNUSED(status);
    Q_UNUSED(dwActualBufferSize);
    Q_UNUSED(pvContext);

    EtalRDSData* prds = (EtalRDSData *)pBuffer;
    // ETAL_globalList_mngr *p2this = (ETAL_globalList_mngr *) pvContext;

    // check PI
    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PI) == ETAL_DECODED_RDS_VALID_PI)
    {
        qDebug() << "FG RDS PI " << QString("%1").arg(prds->m_PI, 0, 16);
    }

    // check PS
    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
    {
        qDebug() << "FG RDS PS " << prds->m_PS;
    }

    // check RT
    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
    {
        qDebug() << "FG RDS RT " << prds->m_RT;
    }
}
