#include "cmd_mngr_etal.h"

#if (defined CONFIG_USE_ETAL)
#include "radio_storage_types.h"
#include <QDebug>
#include <QDateTime>
#include <QThread>

void etalRDSCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext);
void etalRadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext);
void etalSLSCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext);
void etalUserNotificationHandler(void* context, ETAL_EVENTS dwEvent, void* pstatus);
void etalQualityMonitor(EtalBcastQualityContainer* pQuality, void* vpContext);

quint32 Etal::m_cntr = 0;

Etal::Etal(QObject* parent) : QObject(parent)
{
    // Initialize event manager
    eventManager = new EventManager<eventsList, eventDataInterface> ();

    m_boot = false;
    m_seek = false;
    m_ens_available = false;
    m_update_cntr = 0;
    m_cntr++;

    m_etal_handler          = ETAL_INVALID_HANDLE;
    m_etal_handler_qual_mon = ETAL_INVALID_HANDLE;
    m_etal_handler_data     = ETAL_INVALID_HANDLE;
}

void Etal::boot()
{
    // etal init && booting
    qint32 ret;
    EtalHardwareAttr init_params;
    QString msg;

    m_lock.lock();

    if (!m_boot)
    {
        memset(&init_params, 0x0, sizeof (EtalHardwareAttr));

        init_params.m_cbNotify = etalUserNotificationHandler;
        init_params.m_context  = this;

        if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_initialize() error, ret = ") + QString::number(ret);
            m_error = OpenError;
            emit error(m_error, Q_FUNC_INFO + QString(": Open Init Error"));
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_initialize() done, ret = ") + QString::number(ret);
            m_boot = true;
        }
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_initialize() skipped");
    }

    qDebug() << msg;
    emit logging(msg, QString('\n'), 1);

    if (false == m_boot)
    {
        return;
    }

    EtalHwCapabilities* p2cap;

    if ((ret = etal_get_capabilities(&p2cap)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_get_capabilities() error, ret = ") + QString::number(ret);
    }

    for (int i = 0; i < 2; i++)
    {
        qDebug() << "tuner " << i << ", type " << p2cap->m_Tuner[i].m_TunerDevice.m_deviceType;

        for (int j = 0; j < p2cap->m_Tuner[i].m_TunerDevice.m_channels; j++)
        {
            qDebug() << "fe " << j << ", standards DAB " << *p2cap->m_Tuner[i].m_standards;
        }
    }

    /* Configure audio path on CMOST */
    EtalAudioInterfTy audioIf;

    memset(&audioIf, 0, sizeof (EtalAudioInterfTy));
    audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
    if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
    {
        Q_FUNC_INFO + QString(": etal_config_audio_path() error, ret = ") + QString::number(ret);
    }

    m_lock.unlock();
}

Etal::~Etal()
{
    QString msg;
    qint32 ret;

    if ((ret = etal_deinitialize()) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_deinitialize() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_deinitialize() done, ret ") + QString::number(ret);
    }

    qDebug() << msg;
    emit logging(msg, QString('\n'), 1);
}

EtalContent& Etal::getContent()
{
    return m_cont;
}

void Etal::setContent(EtalContent const* cntnt)
{
    m_lock.lock();
    m_cont = *cntnt;
    m_lock.unlock();
}

EtalDABInfo& Etal::getEnsembleContent()
{
    return m_dabInfo;
}

void Etal::powerOn()
{
    m_lock.lock();
    // power on sequence
    // band change
    setBand(m_cont.m_band);
    // set frequncy
    setFrequency(m_cont.m_frequency);
    // select service if there is any
    if (isEnsAvailable())
    {
        getEnsembleInfo();
    }

    m_lock.unlock();
}

void Etal::enableSF(bool en)
{
    Q_UNUSED(en);

#if 0
    qint32 ret;
    QString msg;

    m_lock.lock();

    if (en)
    {
        // enable service following
        //   ret = etaltml_ActivateServiceFollowing();

        if (ret != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etaltml_ActivateServiceFollowing() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etaltml_ActivateServiceFollowing() done, ret ") + QString::number(ret);
        }
    }
    else
    {
        // disable service following
        // ret = etaltml_DisableServiceFollowing();

        if (ret != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etaltml_DisableServiceFollowing() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etaltml_DisableServiceFollowing() done, ret ") + QString::number(ret);
        }
    }

    m_lock.unlock();

    emit logging(msg, QString('\n'), 1);
#endif
}

void Etal::setVolume(qint32 vol)
{
    quint32 ret;
    QString msg;

    if ((ret = etal_audio_output_scaling(vol)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_audio_output_scaling() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_audio_output_scaling() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), 1);
}

void Etal::source()
{
    m_lock.lock();
    // source sequence
    // band change
    setBand(m_cont.m_band);
    // set frequncy
    setFrequency(m_cont.m_frequency);
    // select service if there is any
    if (isEnsAvailable())
    {
        getEnsembleInfo();
    }

    m_lock.unlock();
}

void Etal::updateFrequency(quint32 frequency)
{
    m_lock.lock();
    // set frequncy
    setFrequency(frequency);
    // and select service if there is any
    if (isEnsAvailable())
    {
        getEnsembleInfo();
    }

    m_lock.unlock();
}

void Etal::setSelectService()
{
    if (m_cont.m_band != B_DAB3)
    {
        emit logging(Q_FUNC_INFO + QString(": no ensemble available in band"), QString('\n'), 1);
        return;
    }

    // if we are in DAB mode, select also service
    // if it is available
    if (isEnsAvailable())
    {
        //        getEnsembleInfo();
        //        m_services = getServiceList(m_dabInfo.m_ens.m_ueid, true, false);

        bool selectService = (m_dabInfo.m_service.count() == 0) ? true : false;
        EtalDABInfo org_dabInfo = m_dabInfo;

        if (true == updateServiceList())
        {
            if (selectService)
            {
                selectAudioService(m_dabInfo.m_ens.m_ueid, m_cont.m_id);
            }

            setServiceList();
            //            if (m_dabInfo.m_service.count() > 0)
            //            {
            //                setServiceName(m_dabInfo.m_service[m_dabInfo.m_idx].m_label);
            //            }

            qDebug() << "List update";
        }
    }
}

void Etal::selectService()
{
    m_lock.lock();
    setSelectService();
    m_lock.unlock();
}

void Etal::powerOff()
{
    QString msg;
    qint32 ret;

    m_lock.lock();

    // mute
    setMute(true);
    // disable quality monitor
    enableQualMonitor(false);

    if (m_cont.m_band == B_DAB3)
    {
        enableSLS(false);
        enableDLS(false);
    }
    else if (m_cont.m_band == B_FM)
    {
        enableRDS(false);
    }

    m_lock.unlock();

    QThread::sleep(1);
    m_lock.lock();

    // first check data handler and destroy old if needed
    if (m_etal_handler_data != ETAL_INVALID_HANDLE)
    {
        if ((ret = etal_destroy_datapath(&m_etal_handler_data)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_datapath() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_datapath() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);
    }

    // destroy all handlers
    if (m_etal_handler != ETAL_INVALID_HANDLE)
    {
        if ((ret = etal_destroy_receiver(&m_etal_handler)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_receiver() error, ret ") + QString::number(ret);
        }
    }

    // set band to idle
    // idle not supported by etal
    // setBand();
    // change internal state to active
    m_lock.unlock();
}

// set
void Etal::setBand(Band band)
{
    QString msg;
    qint32 ret;
    EtalProcessingFeatures processingFeatures;
    EtalFrequencyBand etal_band;
    EtalReceiverAttr attr;
    EtalAudioSourceTy src;

    if (m_etal_handler != ETAL_INVALID_HANDLE)
    {
        if ((ret = etal_destroy_receiver(&m_etal_handler)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_receiver() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_receiver() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);
    }

    memset(&attr, 0x00, sizeof (EtalReceiverAttr));
    attr.m_FrontEndsSize = 1;
    attr.m_FrontEnds[0] = m_cont.m_fend;

    switch (band)
    {
        case B_FM:
            m_cont.m_mode = FM;
            if (true == m_phase_diversity)
            {
                attr.m_FrontEndsSize = 2;
                attr.m_FrontEnds[1] = m_cont.m_fend + 1;
            }

            etal_band = ETAL_BAND_FMEU;
            break;

        case B_AM:
            m_cont.m_mode = AM;
            etal_band = ETAL_BAND_AM;
            break;

        case B_LW:
            m_cont.m_mode = AM;
            etal_band = ETAL_BAND_LW;
            break;

        case B_MW:
            m_cont.m_mode = AM;
            etal_band = ETAL_BAND_MWEU;
            break;

        case B_SW:
            m_cont.m_mode = AM;
            etal_band = ETAL_BAND_SW;
            break;

        case B_DAB3:
            m_cont.m_mode = DAB;
            etal_band = ETAL_BAND_DAB3;
            break;

        case B_DRM30:
            m_cont.m_mode = DRM;
            etal_band = ETAL_BAND_DRM30;
            break;

        case B_DRM_PLUS:
            m_cont.m_mode = DRM;
            etal_band = ETAL_BAND_DRMP;
            break;

        default:
            qDebug() << "ERROR: setBand(), etal_band not correctly set, defaulted to FM EU";

            etal_band = ETAL_BAND_FMEU;
            break;
    }

    switch (m_cont.m_mode)
    {
        case AM:
            attr.m_Standard = ETAL_BCAST_STD_AM;
            m_cont.m_band = B_AM;
            src = ETAL_AUDIO_SOURCE_STAR_AMFM;
            break;

        case FM:
            attr.m_Standard = ETAL_BCAST_STD_FM;
            m_cont.m_band = B_FM;
            src = ETAL_AUDIO_SOURCE_STAR_AMFM;
            break;

        case HD:
            attr.m_Standard = ETAL_BCAST_STD_HD;
            src = ETAL_AUDIO_SOURCE_AUTO_HD;
            break;

        case DAB:
            attr.m_Standard = ETAL_BCAST_STD_DAB;
            m_cont.m_band = B_DAB3;
            src = ETAL_AUDIO_SOURCE_DCOP_STA660;
            break;

        case DRM:
            attr.m_Standard = ETAL_BCAST_STD_DRM;
            src = ETAL_AUDIO_SOURCE_DCOP_STA660;
            break;

        default:
            qDebug() << "ERROR: setBand(), src not correctly set, defaulted to ETAL_AUDIO_SOURCE_STAR_AMFM";

            src = ETAL_AUDIO_SOURCE_STAR_AMFM;
            break;
    }

    if ((ret = etal_config_receiver(&m_etal_handler, &attr)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_config_receiver() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_config_receiver() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), 1);

    // Select audio source
    if ((ret = etal_audio_select(m_etal_handler, src)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_audio_selec() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_audio_selec() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), 1);

    if ((true == m_phase_diversity) && (B_FM == band))
    {
        processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_FM_VPA;
    }
    else
    {
        processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
    }

    if ((ret = etal_change_band_receiver(m_etal_handler, etal_band, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_change_band_receiver() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_change_band_receiver() done, ret ") + QString::number(ret);
        m_cont.m_band = band;
        emit bandChanged(band);
    }

    enableQualMonitor(true);

    emit logging(msg, QString('\n'), 1);
}

void Etal::setFrequency(quint32 frequency)
{
    qint32 ret;
    QString msg;
    EtalDataPathAttr dataPathAttr;

    ret = etal_tune_receiver(m_etal_handler, frequency);

    switch (ret)
    {
        case ETAL_RET_SUCCESS:
            msg = Q_FUNC_INFO + QString(": etal_tune_receiver() done, ret ") + QString::number(ret);
            m_cont.m_frequency = frequency;
            m_ens_available =  (B_DAB3 == m_cont.m_band) ? true : false;
            m_update_cntr   =  0;
            m_dabInfo.m_idx = -1;
            emit frequencyChanged(m_cont.m_frequency);
            break;

        case ETAL_RET_NO_DATA:
            msg = Q_FUNC_INFO + QString(": etal_tune_receiver() done, no data avialable, ret ") + QString::number(ret);
            m_cont.m_frequency = frequency;
            m_ens_available = false;
            m_dabInfo.m_idx = -1;
            emit frequencyChanged(m_cont.m_frequency);
            break;

        default:
            msg = Q_FUNC_INFO + QString(": etal_tune_receiver() error, ret ") + QString::number(ret);
            m_ens_available = false;
            break;
    }

    // clear persistant
    m_dabInfo.m_service.clear();
    m_dabState.w = 0;
    m_cont.m_name.clear();
    m_radio_text.clear();
    m_services.clear();

    emit logging(msg, QString('\n'), 1);

    // create data path for FM/DAB/HD
    // first check data handler and destroy old if needed
    if (m_etal_handler_data != ETAL_INVALID_HANDLE)
    {
        if ((ret = etal_destroy_datapath(&m_etal_handler_data)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_datapath() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_datapath() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);
    }

    // second check sls data handler and destroy old if needed
    if (m_etal_handler_data_sls != ETAL_INVALID_HANDLE)
    {
        if ((ret = etal_destroy_datapath(&m_etal_handler_data_sls)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_datapath() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_datapath() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);
    }

    // create new one for current mode
    if (m_cont.m_mode == FM)
    {
        m_etal_handler_data = ETAL_INVALID_HANDLE;
        memset(&dataPathAttr, 0x00, sizeof (EtalDataPathAttr));
        dataPathAttr.m_receiverHandle = m_etal_handler;
        dataPathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS;
        dataPathAttr.m_sink.m_context = this;
        dataPathAttr.m_sink.m_BufferSize = sizeof (EtalRDSData);
        dataPathAttr.m_sink.m_CbProcessBlock = etalRDSCallback;

        if ((ret = etal_config_datapath(&m_etal_handler_data, &dataPathAttr)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_config_datapath() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_config_datapath() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);

        enableRDS(true);
    }
    else if (m_cont.m_mode == DAB)
    {
        m_etal_handler_data = ETAL_INVALID_HANDLE;
        memset(&dataPathAttr, 0x00, sizeof (EtalDataPathAttr));
        dataPathAttr.m_receiverHandle = m_etal_handler;
        dataPathAttr.m_dataType = ETAL_DATA_TYPE_TEXTINFO;
        dataPathAttr.m_sink.m_context = this;
        dataPathAttr.m_sink.m_BufferSize = sizeof (EtalTextInfo);
        dataPathAttr.m_sink.m_CbProcessBlock = etalRadiotextCallback;
        if ((ret = etal_config_datapath(&m_etal_handler_data, &dataPathAttr)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_config_datapath() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_config_datapath() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);

        if ((ret = etaltml_start_textinfo(m_etal_handler)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etaltml_start_textinfo() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etaltml_start_textinfo() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);

        // sls enable
        m_etal_handler_data_sls = ETAL_INVALID_HANDLE;
        memset(&dataPathAttr, 0x00, sizeof (EtalDataPathAttr));

        dataPathAttr.m_receiverHandle = m_etal_handler;
        dataPathAttr.m_dataType = ETAL_DATA_TYPE_DATA_SERVICE;
        dataPathAttr.m_sink.m_context = this;
        dataPathAttr.m_sink.m_CbProcessBlock = etalSLSCallback;

        if ((ret = etal_config_datapath(&m_etal_handler_data_sls, &dataPathAttr)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_config_datapath() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_config_datapath() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);

        EtalDataServiceParam par;
        memset(&par, 0x00, sizeof (EtalDataServiceParam));
        quint32 mask = 0;

        ret = etal_enable_data_service(m_etal_handler, ETAL_DATASERV_TYPE_SLS | ETAL_DATASERV_TYPE_SLS_XPAD, &mask,  par);

        if (ret != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_enable_data_service() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_enable_data_service() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);
    }
    else if (m_cont.m_mode == HD)
    {
        msg = QString("HD mode not supported ... ");
        emit logging(msg, QString('\n'), 1);
    }

    m_update_flag = false;
    qDebug() << "Freq "  << m_cont.m_frequency;
}

void Etal::enableSLS(bool en)
{
    QString msg;
    qint32 ret;

    // second check sls data handler and destroy old if needed
    if (m_etal_handler_data_sls != ETAL_INVALID_HANDLE)
    {
        //        ret = etal_disable_data_service(m_etal_handler, ETAL_DATASERV_TYPE_SLS | ETAL_DATASERV_TYPE_SLS_XPAD);

        //        if(ret != ETAL_RET_SUCCESS)
        //        {
        //            msg = Q_FUNC_INFO + QString(": etal_enable_data_service() error, ret ") + QString::number(ret);
        //        }
        //        else
        //        {
        //            msg = Q_FUNC_INFO + QString(": etal_enable_data_service() done, ret ") + QString::number(ret);
        //        }

        if ((ret = etal_destroy_datapath(&m_etal_handler_data_sls)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_datapath() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_datapath() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);
    }

    if (true == en)
    {
        // sls enable
        EtalDataPathAttr dataPathAttr;

        m_etal_handler_data_sls = ETAL_INVALID_HANDLE;
        memset(&dataPathAttr, 0x00, sizeof (EtalDataPathAttr));

        dataPathAttr.m_receiverHandle = m_etal_handler;
        dataPathAttr.m_dataType = ETAL_DATA_TYPE_DATA_SERVICE;
        dataPathAttr.m_sink.m_context = this;
        dataPathAttr.m_sink.m_CbProcessBlock = etalSLSCallback;

        if ((ret = etal_config_datapath(&m_etal_handler_data_sls, &dataPathAttr)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_config_datapath() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_config_datapath() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);

        EtalDataServiceParam par;
        memset(&par, 0x00, sizeof (EtalDataServiceParam));
        quint32 mask = 0;

        ret = etal_enable_data_service(m_etal_handler, ETAL_DATASERV_TYPE_SLS | ETAL_DATASERV_TYPE_SLS_XPAD, &mask,  par);

        if (ret != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_enable_data_service() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_enable_data_service() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);
    }
}

void Etal::enableDLS(bool en)
{
    QString msg;
    qint32 ret;
    EtalDataPathAttr dataPathAttr;

    if (m_etal_handler_data != ETAL_INVALID_HANDLE)
    {
        if ((ret = etal_destroy_datapath(&m_etal_handler_data)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_datapath() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_datapath() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);
    }

    if (en)
    {
        m_etal_handler_data = ETAL_INVALID_HANDLE;
        memset(&dataPathAttr, 0x00, sizeof (EtalDataPathAttr));
        dataPathAttr.m_receiverHandle = m_etal_handler;
        dataPathAttr.m_dataType = ETAL_DATA_TYPE_TEXTINFO;
        dataPathAttr.m_sink.m_context = this;
        dataPathAttr.m_sink.m_BufferSize = sizeof (EtalTextInfo);
        dataPathAttr.m_sink.m_CbProcessBlock = etalRadiotextCallback;
        if ((ret = etal_config_datapath(&m_etal_handler_data, &dataPathAttr)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_config_datapath() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_config_datapath() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);

        if ((ret = etaltml_start_textinfo(m_etal_handler)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etaltml_start_textinfo() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etaltml_start_textinfo() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);
    }
}

void Etal::enableQualMonitor(bool en)
{
    QString msg;
    qint32 ret;

    // destroy quality monitor in any case
    if (ETAL_INVALID_HANDLE != m_etal_handler_qual_mon)
    {
        if ((ret = etal_destroy_reception_quality_monitor(&m_etal_handler_qual_mon)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_destroy_reception_quality_monitor() error, ret ") + QString::number(ret);
        }
    }

    if (true == en)
    {
        EtalBcastQualityMonitorAttr attrQM;

        m_etal_handler_qual_mon = ETAL_INVALID_HANDLE;
        memset(&attrQM, 0x00, sizeof (EtalBcastQualityMonitorAttr));
        attrQM.m_receiverHandle = m_etal_handler;
        attrQM.m_CbBcastQualityProcess = etalQualityMonitor;
        attrQM.m_Context = this;

        if (B_DAB3 == m_cont.m_band)
        {
            attrQM.m_monitoredIndicators[0].m_MonitoredIndicator = EtalQualityIndicator_DabFieldStrength;
        }
        else
        {
            attrQM.m_monitoredIndicators[0].m_MonitoredIndicator = EtalQualityIndicator_FmFieldStrength;
        }

        attrQM.m_monitoredIndicators[0].m_InferiorValue = ETAL_INVALID_MONITOR;
        attrQM.m_monitoredIndicators[0].m_SuperiorValue = ETAL_INVALID_MONITOR; // don't care
        attrQM.m_monitoredIndicators[0].m_UpdateFrequency = 2500;  // millisec

        if ((ret = etal_config_reception_quality_monitor(&m_etal_handler_qual_mon, &attrQM)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_config_reception_quality_monitor() error, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);
    }
}

void Etal::enableRDS(bool en)
{
    quint32 ret;
    QString msg;

    if (en)
    {
        m_pty = 0;
        m_tp  = 0;
        m_ta  = 0;
        m_ms  = 0;
        m_rdsAFlistLength = 0;

        if ((ret = etal_start_RDS(m_etal_handler, 0, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_start_RDS() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_start_RDS() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);

        if ((ret = etaltml_start_decoded_RDS(m_etal_handler, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etaltml_start_decoded_RDS() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etaltml_start_decoded_RDS() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);
    }
    else
    {
        if ((ret = etal_stop_RDS(m_etal_handler)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_stop_RDS() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_stop_RDS() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);

        if ((ret = etaltml_stop_decoded_RDS(m_etal_handler, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etaltml_stop_decoded_RDS() error, ret ") + QString::number(ret);
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etaltml_stop_decoded_RDS() done, ret ") + QString::number(ret);
        }

        emit logging(msg, QString('\n'), 1);
    }
}

void Etal::setMute(bool mute)
{
    qint32 ret;
    QString msg;

    if ((ret = etal_mute(m_etal_handler, mute)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_mute() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_mute() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), 1);
}

void Etal::getEnsembleList()
{
    QString msg;
    qint32 ret;
    EtalEnsembleList ensList;

    ret = etal_get_ensemble_list(&ensList);

    if (ret != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_get_ensemble_list() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_get_ensemble_list() done, ret ") + QString::number(ret);

        qDebug() << "Ens number " << ensList.m_ensembleCount;
        for (quint32 i = 0; i < ensList.m_ensembleCount; i++)
        {
            qDebug() << "\t" << i << "ecc " << QString("%1").arg(ensList.m_ensemble[i].m_ECC, 0, 16);
            qDebug() << "\t" << i << "ID  " << QString("%1").arg(ensList.m_ensemble[i].m_ensembleId, 0, 16);
            qDebug() << "\t" << i << "frq " << ensList.m_ensemble[i].m_frequency;
        }
    }

    emit logging(msg, QString('\n'), 1);
}

quint32 Etal::getCurrentEnsemble()
{
    quint32 ens;
    qint32  ret;
    QString msg;

    if (!m_ens_available)
    {
        emit logging(Q_FUNC_INFO + QString(": no ensemble available"), QString('\n'), 1);
        return 0;
    }

    ret = etal_get_current_ensemble(m_etal_handler, &ens);

    if (ret != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_get_current_ensemble() error, ret ") + QString::number(ret);
        ens = 0;
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_get_current_ensemble() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), 1);

    return ens;
}

QList<quint32> Etal::getServiceList(qint32 ens, bool audioServ, bool dataServ)
{
    EtalServiceList servList;
    QString msg;
    qint32 ret;

    QList<quint32> list;

    // MT 23-11-2017 Initialize servList count to 0
    servList.m_serviceCount = 0;

    if (!m_ens_available)
    {
        emit logging(Q_FUNC_INFO + QString(": no services available"), QString('\n'), 1);
        return list;
    }

    ret = etal_get_service_list(m_etal_handler, ens, audioServ, dataServ, &servList);

    if (ret != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_get_service_list() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_get_service_list() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), 1);

    qDebug() << "Services: " << servList.m_serviceCount;

    for (quint32 i = 0; i < servList.m_serviceCount; i++)
    {
        list << servList.m_service[i];
        qDebug() << "\t" << QString(" %1").arg(servList.m_service[i], 0, 16);
    }

    return list;
}

void Etal::ensembleInfo()
{
    m_lock.lock();

    if (isEnsAvailable())
    {
        getEnsembleInfo();
    }

    m_lock.unlock();
}

bool Etal::selectAudioService(qint32 ens, quint16 id)
{
    qint32      ret;
    QString     msg;

    if (!m_ens_available)
    {
        emit logging(Q_FUNC_INFO + QString(": no services available"), QString('\n'), 1);
        return false;
    }

    ret = etal_service_select_audio(m_etal_handler, ETAL_SERVSEL_MODE_SERVICE, ens, id, 0, 0);

    if (ret != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_service_select_audio() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_service_select_audio() done, ret ") + QString::number(ret) + QString(" id ") + QString::number(id);
    }

    emit logging(msg, QString('\n'), 1);

    return (ret == ETAL_RET_SUCCESS) ? true : false;
}

void Etal::selectDataService(qint32 ens, qint32 id)
{
    Q_UNUSED(ens);
    Q_UNUSED(id);

    if (!m_ens_available)
    {
        emit logging(Q_FUNC_INFO + QString(": no services available"), QString('\n'), 1);
        return;
    }
}

bool Etal::serviceSelect(int servID)
{
    bool ret;

    if (!m_ens_available)
    {
        emit logging(Q_FUNC_INFO + QString(": no services available"), QString('\n'), 1);
        return false;
    }

    ret = selectAudioService(m_dabInfo.m_ens.m_ueid, servID);

    if (true == ret)
    {
        m_cont.m_id = servID;
    }

    return ret;
}

QString Etal::pty_str() const
{
    QString ret;

    switch (m_pty)
    {
        case 1:
            ret = QString("News");
            break;

        case 2:
            ret = QString("Current affairs");
            break;

        case 3:
            ret = QString("Information");
            break;

        case 4:
            ret = QString("Sport");
            break;

        case 5:
            ret = QString("Education");
            break;

        case 6:
            ret = QString("Drama");
            break;

        case 7:
            ret = QString("Culture");
            break;

        case 8:
            ret = QString("Science");
            break;

        case 9:
            ret = QString("Varied");
            break;

        case 10:
            ret = QString("Pop music");
            break;

        case 11:
            ret = QString("Rock music");
            break;

        case 12:
            ret = QString("Easy listening");
            break;

        case 13:
            ret = QString("Light classical");
            break;

        case 14:
            ret = QString("Serious classical");
            break;

        case 15:
            ret = QString("Other music");
            break;

        case 16:
            ret = QString("Weather");
            break;

        case 17:
            ret = QString("Finance");
            break;

        case 18:
            ret = QString("Children's programmes");
            break;

        case 19:
            ret = QString("Social affairs");
            break;

        case 20:
            ret = QString("Region");
            break;

        case 21:
            ret = QString("Phone-in");
            break;

        case 22:
            ret = QString("Travel");
            break;

        case 23:
            ret = QString("Leisure");
            break;

        case 24:
            ret = QString("Jazz music");
            break;

        case 25:
            ret = QString("Country music");
            break;

        case 26:
            ret = QString("National music");
            break;

        case 27:
            ret = QString("Oldies music");
            break;

        case 28:
            ret = QString("Folk music");
            break;

        case 29:
            ret = QString("Documentary");
            break;

        case 30:
            ret = QString("Alarm test");
            break;

        case 31:
            ret = QString("Alarm");
            break;

        default:
            ret = QString(" ");
            break;
    }

    return ret;
}

void Etal::seekUp(qint32 step)
{
    qint32      ret;
    QString     msg;

    m_seek = true;

    // stop rds in fm
    switch (m_cont.m_band)
    {
        case B_FM:
            enableRDS(false);
            break;

        case B_DAB3:
            m_dabState.w = 0;
            m_cont.m_name.clear();
            m_services.clear();
            break;

        default:
            break;
    }

    if ((ret = etal_autoseek_start(m_etal_handler, cmdDirectionUp, step, cmdAudioUnmuted, dontSeekInSPS, true)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etaltml_seek_start() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etaltml_seek_start() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), 1);
}

void Etal::seekDown(qint32 step)
{
    qint32      ret;
    QString     msg;

    m_seek = true;

    // stop rds in fm
    switch (m_cont.m_band)
    {
        case B_FM:
            enableRDS(false);
            break;

        case B_DAB3:
            m_dabState.w = 0;
            m_cont.m_name.clear();
            m_services.clear();
            break;

        default:
            break;
    }

    if ((ret = etal_autoseek_start(m_etal_handler, cmdDirectionDown, step, cmdAudioUnmuted, dontSeekInSPS, true)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etaltml_seek_start() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etaltml_seek_start() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), 1);
}

void Etal::seekStop()
{
    quint32 ret;
    QString msg;

    if ((ret = etal_autoseek_stop(m_etal_handler, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etal_autoseek_stop() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etal_autoseek_stop() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), 1);
}

void Etal::learn()
{
    qint32            ret, step;
    QString           msg;
    EtalFrequencyBand band;

    switch (m_cont.m_band)
    {
        case B_DAB3:
            band = ETAL_BAND_DAB3;
            step = 100;
            break;

        case B_FM:
            band = ETAL_BAND_FMEU;
            step = 100;
            break;

        default:
            band = ETAL_BAND_FMEU;
            step = 100;
            break;
    }

    if ((ret = etaltml_learn_start(m_etal_handler, band, step, 30, normalMode, m_freqList)) != ETAL_RET_SUCCESS)
    {
        msg = Q_FUNC_INFO + QString(": etaltml_learn_start() error, ret ") + QString::number(ret);
    }
    else
    {
        msg = Q_FUNC_INFO + QString(": etaltml_learn_start() done, ret ") + QString::number(ret);
    }

    emit logging(msg, QString('\n'), 1);
}

void Etal::setRadioText(QString dlsStr)
{
    eventDataInterface eventData;
    QByteArray a;

    m_radio_text = dlsStr;

    a.append(dlsStr);
    eventData.dataPtr = (unsigned char *)a.data();
    eventData.size = a.size();
    eventData.eventType = EVENTS_RX_DLS;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Etal::setSLS(QByteArray slsArray)
{
    eventDataInterface eventData;

    // for debug only picture from file
    //        QByteArray slsArray;
    //        // ... fill the array with data ...

    //        QFile file("d:/tmp/sls/fig0.jpg");
    //        file.open(QIODevice::ReadOnly);
    //        slsArray = file.readAll();
    //        file.close();

    eventData.dataPtr = (unsigned char *)slsArray.data();
    eventData.size = slsArray.size();
    eventData.eventType = EVENTS_RX_SLS;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Etal::setEnsembleName()
{
    eventDataInterface eventData;

    EnsembleTableTy ensTable = getEnsTable();

    eventData.dataPtr = (unsigned char *)&ensTable;
    eventData.size = sizeof (EnsembleTableTy);

    eventData.eventType = EVENTS_RX_ENSEMBLE_NAME;
    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Etal::setServiceName(QString servStr)
{
    eventDataInterface eventData;
    QByteArray a;

    m_cont.m_name = servStr;

    a.append(servStr);
    eventData.dataPtr = (unsigned char *)a.data();
    eventData.size = a.size();

    eventData.eventType = EVENTS_RX_SERVICE_NAME;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Etal::setServiceList()
{
    ServiceListTy serviceList = getServices();
    eventDataInterface eventData;

    eventData.dataPtr = (unsigned char *)&serviceList;
    eventData.size = sizeof (ServiceListTy);

    eventData.eventType = EVENTS_UPDATE_SERVICE_LIST;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

EnsembleTableTy Etal::getEnsTable()
{
    EnsembleTableTy ensTable;

    ensTable.ensembleUniqueId = m_dabInfo.m_ens.m_ueid;
    ensTable.EnsECCID = QString("%1").arg(m_dabInfo.m_ens.m_ueid, 0, 16);
    ensTable.ensFrequency = m_cont.m_frequency;
    ensTable.EnsCharset = m_dabInfo.m_ens.m_charset;
    ensTable.EnsChLabel = m_dabInfo.m_ens.m_label;

    return ensTable;
}

ServiceListTy Etal::getServices()
{
    ServiceTy service;

    m_lock.lock();

    m_serviceList.serviceList.clear();

    for (qint32 i = 0; i < m_dabInfo.m_service.count(); i++)
    {
        service.SubChID = 0;
        service.ServiceID = QString("0000%1").arg(m_dabInfo.m_service[i].m_id, 0, 16);
        service.ServiceBitrate = m_dabInfo.m_service[i].m_bitrate;
        service.ServiceCharset = 0;
        service.ServiceLabel = m_dabInfo.m_service[i].m_label;
        service.serviceUniqueId = m_dabInfo.m_service[i].m_id;
        service.ServicePty = 0;

        m_serviceList.serviceList.append(service);
    }

    m_lock.unlock();

    return m_serviceList;
}

bool Etal::updateServiceList()
{
    quint32         ens              = getCurrentEnsemble();

    QList<quint32>  services         = m_services;
    bool            ret              = false;

    getEnsembleInfo();
    getServicesInfo(ens);
    ret = true;

    return ret;
}

void Etal::setSyncStatus(qualityInfoTy* qualInfo)
{
    eventDataInterface eventData;

    eventData.dataPtr = (unsigned char *)qualInfo;
    eventData.size = sizeof (qualityInfoTy);
    eventData.eventType = EVENTS_RX_SYNC_LEVEL;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Etal::setRdsData(EtalRDSData* prds)
{
    eventDataInterface eventData;
    RdsDataSetTableTy  rdsData;

    // memset(&rdsData, 0x0, sizeof(RdsDataSetTableTy));

    if (prds->m_validityBitmap == 0)
    {
        qDebug() << " rds callback no data ... ";
        return;
    }

    m_lock.lock();

    qDebug() << "fg rds callback .... " << prds->m_validityBitmap;

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PI) == ETAL_DECODED_RDS_VALID_PI)
    {
        m_cont.m_id = prds->m_PI;
        qDebug() << "PI " << QString("%1").arg(prds->m_PI, 0, 16);
    }

    //    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_DI) == ETAL_DECODED_RDS_VALID_DI)
    //    {
    //        qDebug() << "DI " << QString("%1").arg(prds->m_DI, 0, 16);
    //    }

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
    {
        m_cont.m_name = prds->m_PS;
        m_cont.m_name.resize(8);
        qDebug() << "ps name " << m_cont.m_name;
    }

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
    {
        m_radio_text = prds->m_RT;
        qDebug() << "RT " << GetRadioText();
    }

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_AF) == ETAL_DECODED_RDS_VALID_AF)
    {
        m_rdsAFlistLength = prds->m_AFListLen;
        for (quint32 i = 0; i < prds->m_AFListLen; i++)
        {
            m_rdsAFlist[i] = prds->m_AFList[i];
        }
    }

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PTY) == ETAL_DECODED_RDS_VALID_PTY)
    {
        m_pty = prds->m_PTY;
        qDebug() << "PTY " << pty_str();
    }

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_TP) == ETAL_DECODED_RDS_VALID_TP)
    {
        m_tp = prds->m_TP;
        qDebug() << "TP " << m_tp;
    }

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_TA) == ETAL_DECODED_RDS_VALID_TA)
    {
        m_ta = prds->m_TA;
        qDebug() << "TA " << m_ta;
    }

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_MS) == ETAL_DECODED_RDS_VALID_MS)
    {
        m_ms = prds->m_MS;
        qDebug() << "MS " << m_ms;
    }

    // fill export structure with current content
    rdsData.PIid = QString("%1").arg(m_cont.m_id, 0, 16);
    rdsData.PSname = m_cont.m_name;
    rdsData.rtText = m_radio_text;
    rdsData.ptyVal = m_pty;
    rdsData.ptyName = pty_str();
    rdsData.tpFlag = m_tp;
    rdsData.taFlag = m_ta;
    rdsData.msFlag = m_ms;
    for (quint32 i = 0; i < 26; i++)
    {
        rdsData.RdsAFList[i] = m_rdsAFlist[i];
    }

#if 0
    qDebug() << "PI " << rdsData.PIid;
    qDebug() << "PS " << rdsData.PSname;
    qDebug() << "PTY " << m_pty;
    qDebug() << "PTY Name" << pty_str();
    qDebug() << "RT " << rdsData.rtText;
#endif

    eventData.dataPtr = (unsigned char *)&rdsData;
    eventData.size = sizeof (RdsDataSetTableTy);
    eventData.eventType = EVENTS_UPDATE_RDS_DATA;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
    m_lock.unlock();
}

void Etal::setFrequencyUpdate(quint32 frequency)
{
    RadioTuneAnswer tuneData;
    eventDataInterface eventData;

    switch (m_cont.m_band)
    {
        case B_AM:
        default:
            tuneData.band = BAND_AM;
            break;

        case B_FM:
            tuneData.band = BAND_FM;
            break;

        case B_DAB3:
            tuneData.band = BAND_DAB3;
            break;
    }

    tuneData.freq = frequency;

    eventData.dataPtr = reinterpret_cast<unsigned char *> (&tuneData);
    eventData.size = sizeof (RadioTuneAnswer);
    eventData.eventType = EVENTS_RX_FREQ;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void Etal::monitorUpdate(EtalBcastQualityContainer* pQuality)
{
    m_lock.lock();

    qDebug() << Q_FUNC_INFO;

    switch (band())
    {
        case B_DAB3:
            monitorUpdateDAB(pQuality);
            break;

        case B_AM:
            monitorUpdateAM(pQuality);
            break;

        case B_FM:
        default:
            monitorUpdateFM(pQuality);
            break;
    }

    m_lock.unlock();
}

void Etal::updateDABState()
{
#if 0
    qDebug() << "#### Updating DAB Start ####";
    if (!m_dabState.b.ens)
    {
        qDebug() << "#### #### Updating Ens info ####";
        getEnsembleInfo();
        m_dabState.b.ens = (!m_dabInfo.m_ens.m_label.isEmpty()) ? 1 : 0;
    }
    else
    {
        qDebug() << "#### #### Skip updating Ens info ####";
    }

    if ((!m_dabState.b.services) && m_dabState.b.ens)
    {
        //
        getEnsembleInfo();
        m_dabState.b.ens = (!m_dabInfo.m_ens.m_label.isEmpty()) ? 1 : 0;

        qDebug() << "#### #### Updating services ####";
        m_services = getServiceList(m_dabInfo.m_ens.m_ueid, true, false);
        qDebug() << "Numb of Services " << m_services.count();
        m_dabState.b.services = (m_services.count()) ? 1 : 0;
    }
    else
    {
        qDebug() << "#### #### Skip updating services ####";
    }

    if (m_dabState.b.ens && m_dabState.b.services)
    {
        if (!m_dabState.b.info)
        {
            qDebug() << "#### #### Updating info ####";
            qDebug() << "   id " << QString("%1").arg(m_cont.m_id, 0, 16);
            setSelectService();
            if (m_dabInfo.m_service.count() == m_services.count())
            {
                if (m_update_cntr >= ETAL_MNGR_DAB_UPDATE_SWITCH_OFF_CNTR)
                {
                    m_dabState.b.info = 1;
                }
                else
                {
                    m_update_cntr++;
                }
            }
            else
            {
                m_update_cntr = 0;
            }

            qDebug() << "#### #### sent  " << m_dabInfo.m_service.count();
            qDebug() << "#### #### avail " << m_services.count();
        }
        else
        {
            qDebug() << "#### #### Skip updating info ####";
        }
    }
    else
    {
        qDebug() << "#### #### Skip updating info ####";
    }

    qDebug() << "####  Updating DAB End  ####";
#endif
}

void Etal::updateDAB(EtalTuneStatus* status)
{
    qualityInfoTy qualInfo;

    if (status->m_receiverHandle != m_etal_handler)
    {
        return;
    }

    if (false == m_seek)
    {
        m_lock.lock();

        qDebug() << "status sync " <<  status->m_sync;
        qDebug() << "status mute " <<  status->m_muteStatus;

        //        qualInfo.sync = (ETAL_MNGR_DAB_SYNCHRONIZED == status->m_sync) ? true : false;
        qualInfo.sync = (status->m_sync >= ETAL_MNGR_DAB_SYNCHRONIZED) ? true : false;

        //    enum BerQualityTy
        //    {
        //        BER_RESERVED                          = 0,
        //        BER_LEVEL_ZERO                        = 1,
        //        BER_LESS_5E_4                         = 2,
        //        BER_LESS_5E_3                         = 3,
        //        BER_LESS_5E_2                         = 4,
        //        BER_LESS_5E_1                         = 5,
        //        BER_GE_5E_1                           = 6,
        //        BER_NOT_AVAILABLE                     = 7
        //    };

        // TODO: apply scaling to BerQualityTy
        qualInfo.fmQualityInfo.qualSNR = (ETAL_MNGR_DAB_SYNCHRONIZED == status->m_sync) ? 1 : 7;
        setSyncStatus(&qualInfo);

        if (ETAL_MNGR_DAB_SYNCHRONIZED == status->m_sync)
        {
            //            m_ens_available = true;
            //            m_update_cntr   = 0;
            //            getEnsembleInfo();
            //            m_services = getServiceList(m_dabInfo.m_ens.m_ueid, true, false);
        }
        else if (ETAL_MNGR_DAB_READY == status->m_sync)
        {
            if (!m_ens_available)
            {
                qDebug() << "DAB available";
                m_ens_available = true;
                m_update_cntr   = 0;
                updateDABState();
            }
        }
        else
        {
            qDebug() << "DAB not available";
            m_ens_available = false;
        }

        m_lock.unlock();
    }
}

qualityInfoTy  Etal::getQuality(EtalBcastQualityContainer* pQuality)
{
    EtalBcastQualityContainer qual;
    QString msg;
    quint32 ret;

    Q_UNUSED(msg);
    Q_UNUSED(ret);

    qDebug() << "Get qual pointer " << QString::number(reinterpret_cast<quint32> (pQuality));

    if (pQuality == NULL)
    {
        // direct read temporary out till 2.5s delay issue is solved
        //                QDateTime nowStartTime = QDateTime::currentDateTime();
        //                qint64 tmpValStartTime = nowStartTime.toMSecsSinceEpoch();

        //                if ((ret = etal_get_reception_quality(m_etal_handler, &qual)) != ETAL_RET_SUCCESS)
        //                {
        //                    msg = Q_FUNC_INFO + QString(": etal_get_channel_quality() error, ret ") + QString::number(ret);
        //                }
        //                else
        //                {
        //                    msg = Q_FUNC_INFO + QString(": etal_get_channel_quality() done, ret ") + QString::number(ret);
        //                }

        //                QDateTime nowEndTime = QDateTime::currentDateTime();
        //                qint64 tmpValEndTime = nowEndTime.toMSecsSinceEpoch();
        //                qint64 deltaTime = tmpValEndTime - tmpValStartTime;

        //                qDebug() << "QUALITY DELTA TIME: " << deltaTime;

        //        emit logging(msg, QString('\n'), 1);
    }
    else
    {
        qual = *pQuality;

        switch (m_cont.m_band)
        {
            case B_DAB3:
#if 1
                m_qual.sync = qual.EtalQualityEntries.dab.m_syncStatus;
                m_qual.qualFstRf = qual.EtalQualityEntries.dab.m_RFFieldStrength;
                m_qual.qualFstBb = qual.EtalQualityEntries.dab.m_BBFieldStrength;

                m_qual.qualFicBer = qual.EtalQualityEntries.dab.m_FicBitErrorRatio;
                m_qual.dabQualityInfo.audioBer = qual.EtalQualityEntries.dab.m_audioBitErrorRatioLevel;
                m_qual.dabQualityInfo.mscBer = qual.EtalQualityEntries.dab.m_MscBitErrorRatio;
                m_qual.dabQualityInfo.ficBer = qual.EtalQualityEntries.dab.m_FicBitErrorRatio;
                m_qual.dabQualityInfo.qualServiceBitRate = 88;

                qDebug() << "%%%%%%%%%%% fst rf   " <<   (qint8)m_qual.qualFstRf;
                qDebug() << "%%%%%%%%%%% fst bb   " <<   (qint8)m_qual.qualFstBb;
                qDebug() << "%%%%%%%%%%% sync     " <<   qual.EtalQualityEntries.dab.m_syncStatus;
                qDebug() << "%%%%%%%%%%% ficber   " <<   qual.EtalQualityEntries.dab.m_FicBitErrorRatio;
                qDebug() << "%%%%%%%%%%% mscber   " <<   qual.EtalQualityEntries.dab.m_MscBitErrorRatio;
                qDebug() << "%%%%%%%%%%% audiober " <<   qual.EtalQualityEntries.dab.m_audioBitErrorRatioLevel;
#else
                if (isEnsAvailable())
                {
                    m_qual.sync = true;
                    m_qual.qualFicBer = 1;
                    m_qual.qualFstRf = quint8(-60);
                    m_qual.qualFstBb = quint8(-40);
                    m_qual.dabQualityInfo.audioBer = 0;
                    m_qual.dabQualityInfo.mscBer = 1;
                    m_qual.dabQualityInfo.ficBer = 2;
                    m_qual.dabQualityInfo.qualServiceBitRate = 88;
                }
                else
                {
                    m_qual.sync = false;
                    m_qual.qualFicBer = 7;
                    m_qual.qualFstRf = quint8(-110);
                    m_qual.qualFstBb = quint8(-150);
                    m_qual.dabQualityInfo.audioBer = 4;
                    m_qual.dabQualityInfo.mscBer = 5;
                    m_qual.dabQualityInfo.ficBer = 6;
                    m_qual.dabQualityInfo.qualServiceBitRate = 88;
                }
#endif
                break;

            case B_AM:
                m_qual.qualFstRf = m_signalStrength = qual.EtalQualityEntries.amfm.m_RFFieldStrength;
                m_qual.qualFstBb = qual.EtalQualityEntries.amfm.m_BBFieldStrength;
                m_qual.fmQualityInfo.qualAdj = qual.EtalQualityEntries.amfm.m_AdjacentChannel;
                m_qual.fmQualityInfo.qualCoChannel = 0;
                m_qual.fmQualityInfo.qualMultiPath = qual.EtalQualityEntries.amfm.m_Multipath;
                m_qual.fmQualityInfo.qualMpxNoise = qual.EtalQualityEntries.amfm.m_UltrasonicNoise;
                m_qual.fmQualityInfo.qualDeviation = qual.EtalQualityEntries.amfm.m_ModulationDetector;
                m_qual.fmQualityInfo.qualStereo = 0;
                m_qual.fmQualityInfo.qualDetune = qual.EtalQualityEntries.amfm.m_FrequencyOffset >> 8;
                m_qual.fmQualityInfo.qualSNR = quint8(((100. * qual.EtalQualityEntries.amfm.m_SNR) / 255.) + 0.5);
                break;

            case B_FM:
            default:
                // #define DEBUG_MONITOR_TEST
#if defined (DEBUG_MONITOR_TEST)
                m_qual.qualFstRf = 55;
                m_qual.qualFstBb = 45;
                // m_qual.fmQualityInfo.qualAdj = qual.EtalQualityEntries.amfm.m_AdjacentChannel;
                m_qual.fmQualityInfo.qualAdj = 128; // ~100%
                m_qual.fmQualityInfo.qualCoChannel = 170; // ~66%
                // m_qual.fmQualityInfo.qualMultiPath = qual.EtalQualityEntries.amfm.m_Multipath;
                m_qual.fmQualityInfo.qualMultiPath = 128;
                // m_qual.fmQualityInfo.qualMpxNoise = qual.EtalQualityEntries.amfm.m_UltrasonicNoise;
                m_qual.fmQualityInfo.qualMpxNoise = 128; // ~50%
                // m_qual.fmQualityInfo.qualDeviation = qual.EtalQualityEntries.amfm.m_ModulationDetector;
                m_qual.fmQualityInfo.qualDeviation = 25; // ~40kHz
                m_qual.fmQualityInfo.qualStereo = 0;
                // m_qual.fmQualityInfo.qualDetune = qual.EtalQualityEntries.amfm.m_FrequencyOffset;
                m_qual.fmQualityInfo.qualDetune = 128; // ~25kHz
                // scale quint8 to % range
                m_qual.fmQualityInfo.qualSNR = 50;
                // quint8(( (100. * qual.EtalQualityEntries.amfm.m_SNR) / 255.)+0.5);
                m_qual.sync = (m_signalStrength > 12) ? 1 : 0;
#else
                qDebug() << "dev raw " << qual.EtalQualityEntries.amfm.m_ModulationDetector << " recalc " << 1.6 * qual.EtalQualityEntries.amfm.m_ModulationDetector << "kHz";
                qDebug() << "det raw " << qual.EtalQualityEntries.amfm.m_FrequencyOffset << " recalc " << 195. * qual.EtalQualityEntries.amfm.m_FrequencyOffset << "Hz";

                m_qual.qualFstRf = m_signalStrength = qual.EtalQualityEntries.amfm.m_RFFieldStrength;
                m_qual.qualFstBb = qual.EtalQualityEntries.amfm.m_BBFieldStrength;
                m_qual.fmQualityInfo.qualAdj = qual.EtalQualityEntries.amfm.m_AdjacentChannel;
                m_qual.fmQualityInfo.qualCoChannel = qual.EtalQualityEntries.amfm.m_coChannel;
                m_qual.fmQualityInfo.qualMultiPath = qual.EtalQualityEntries.amfm.m_Multipath;
                m_qual.fmQualityInfo.qualMpxNoise = qual.EtalQualityEntries.amfm.m_UltrasonicNoise;
                m_qual.fmQualityInfo.qualDeviation = quint8((qual.EtalQualityEntries.amfm.m_ModulationDetector / 1600.) + 0.5);
                m_qual.fmQualityInfo.qualStereo = qual.EtalQualityEntries.amfm.m_StereoMonoReception;
                m_qual.fmQualityInfo.qualDetune = quint8((qual.EtalQualityEntries.amfm.m_FrequencyOffset / 195.) + 0.5);
                // scale quint8 to % range
                m_qual.fmQualityInfo.qualSNR = quint8(((100. * qual.EtalQualityEntries.amfm.m_SNR) / 255.) + 0.5);
                m_qual.sync = (m_signalStrength > 12) ? 1 : 0;
#endif
                break;
        }
    }

    return m_qual;
}

void Etal::monitorUpdateDAB(EtalBcastQualityContainer* pQuality)
{
    Q_UNUSED(pQuality);
    QString msg;

    if (false == m_seek)
    {
        // !TODO: this is workaroud, monitor give wrong fst value
        qDebug() << "DAB Monitor";
        getQuality(pQuality);

        msg = "DAB Monitor\n";
        msg += (true == isEnsAvailable() ? "DAB ens available" : "DAB ens not available");
        msg += " sync " + QString::number(m_qual.sync);
        msg += "\nRF fst " + QString::number((qint8)m_qual.qualFstRf) + \
            "dBm, BB fst " + QString::number((qint8)m_qual.qualFstBb) + \
            ", bit rate " + QString::number(m_qual.dabQualityInfo.qualServiceBitRate) + \
            "kbps, FIC BER ration " + QString::number(m_qual.qualFicBer) + \
            ", MSC BER ration  " + QString::number(m_qual.dabQualityInfo.mscBer) + \
            ", AUDIO BER ratio " + QString::number(m_qual.dabQualityInfo.audioBer);

        emit message(msg, "\n", 2);

        // send quality
        setSyncStatus(&m_qual);

        // update DAB state
        updateDABState();
    }
}

void Etal::monitorUpdateFM(EtalBcastQualityContainer* pQuality)
{
    Q_UNUSED(pQuality);
    QString msg;

    getQuality(pQuality);

    msg = "FM Monitor\n";
#if 0
    msg += "\tfst RF " + QString::number(pQuality->EtalQualityEntries.amfm.m_RFFieldStrength) + " dBuV, " + \
        "fst BB " + QString::number(pQuality->EtalQualityEntries.amfm.m_BBFieldStrength) + " dBuV, " + \
        "coChannel " + QString::number(pQuality->EtalQualityEntries.amfm.m_coChannel) + ",\n" + \
        "\tfreqOffset " + QString::number(pQuality->EtalQualityEntries.amfm.m_FrequencyOffset) + ", " \
                                                                                                 "modulationDetector " + QString::number(pQuality->EtalQualityEntries.amfm.m_ModulationDetector) + ", " \
                                                                                                                                                                                                   "multipath " + QString::number(pQuality->EtalQualityEntries.amfm.m_Multipath) + ",\n" \
                                                                                                                                                                                                                                                                                   "stereo " + QString::number(pQuality->EtalQualityEntries.amfm.m_StereoMonoReception) + ", " \
                                                                                                                                                                                                                                                                                                                                                                          "ultrasonicNoise " + QString::number(pQuality->EtalQualityEntries.amfm.m_UltrasonicNoise) + ", " \
                                                                                                                                                                                                                                                                                                                                                                                                                                                                      "dBuV, snr " + QString::number(m_qual.fmQualityInfo.qualSNR) + "%";
#endif
    emit message(msg, "\n", 2);

    msg  = "\tAFListLen   : " + QString::number(m_rdsAFlistLength) + '\n';
    msg += "\tAFList:\n";

    for (quint32 i = 0; i < m_rdsAFlistLength; i++)
    {
        msg += "\t\t" + QString::number(87500 + 100 * m_rdsAFlist[i]) + " kHz\n";
    }

    emit message(msg, "\n", 2);

    // setSyncStatus(&m_qual);
}

void Etal::monitorUpdateAM(EtalBcastQualityContainer* pQuality)
{
    qualityInfoTy qualInfo;
    QString msg;

    getQuality(pQuality);

    msg = "AM Monitor\n";
    msg += "fst " + QString::number(pQuality->EtalQualityEntries.amfm.m_RFFieldStrength) + "dBuV";
    emit message(msg, "\n", 2);

    //    setSyncStatus(&m_qual);
}

void Etal::seekHandler(void* status)
{
    EtalSeekStatus* seek_status = (EtalSeekStatus *)status;

    qDebug() << "SEEK EVENT freq " << seek_status->m_frequency << " good " << seek_status->m_frequencyFound << " full cycle " << seek_status->m_fullCycleReached << " status " << seek_status->m_status;

    if (seek_status->m_receiverHandle != m_etal_handler)
    {
        // qDebug() << "0 SEEK EVENT freq " << seek_status->m_frequency << " state " << seek_status->m_status;

        // emit signal only in the case of seek result
        if (seek_status->m_status != ETAL_SEEK_RESULT)
        // if(seek_status->m_status != ETAL_SEEK_FINISHED)
        {
            return;
        }

        //        qDebug() << "SEEK EVENT freq " << seek_status->m_frequency << " good " << seek_status->m_frequencyFound << " full cycle " <<
        // seek_status->m_fullCycleReached << " status " << seek_status->m_status;
        //        QString msg;
        //        msg = "SEEK EVENT freq " + QString::number(seek_status->m_frequency) + " good " + QString::number(seek_status->m_frequencyFound) + " full
        // cycle " + QString::number(seek_status->m_fullCycleReached) + " status " + QString::number(seek_status->m_status);
        //        emit logging(msg, QString('\n') , 1);

        memcpy(&m_seek_bg, status, sizeof (EtalSeekStatus));
        emit    seekStationList(&m_seek_bg);
        return;
    }

    m_lock.lock();

    qDebug() << "Seek freq  " <<  seek_status->m_frequency << " seek state " << seek_status->m_status;

    if (ETAL_SEEK_RESULT == seek_status->m_status)
    {
        m_cont.m_frequency =  seek_status->m_frequency;

        // SendTuneResponse(tuneAnswer);

        qDebug() << "Freq update " << m_cont.m_frequency;
        setFrequencyUpdate(m_cont.m_frequency);
    }

    if (((seek_status->m_frequencyFound) || (seek_status->m_fullCycleReached)) && \
        (ETAL_SEEK_FINISHED == seek_status->m_status))
    {
        if (seek_status->m_frequencyFound)
        {
            qDebug() << Q_FUNC_INFO << "SEEK EVENT freq found " << seek_status->m_frequency;
            if (band() == B_DAB3)
            {
                m_ens_available = true;
            }

            sendSeekFreqFoundSignal();
        }

        if (seek_status->m_fullCycleReached)
        {
            qDebug() << Q_FUNC_INFO << "SEEK EVENT ful cycle reached " << seek_status->m_frequency;
            sendSeekFullCycleSignal();
        }

        switch (band())
        {
            case B_DAB3:
                monitorUpdateDAB(&seek_status->m_quality);
                break;

            case B_AM:
                monitorUpdateAM(&seek_status->m_quality);
                break;

            case B_FM:
            default:
                monitorUpdateFM(&seek_status->m_quality);
                enableRDS(true);
                break;
        }
    }

    m_lock.unlock();
}

bool Etal::getEnsembleInfo()
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

    emit logging(msg, QString('\n'), 1);

    qDebug() << Q_FUNC_INFO << "label " << label << " char set " << charset << " ret " << QString::number(ret);

    m_dabInfo.m_ens.m_label.clear();

    if (ret == ETAL_RET_SUCCESS)
    {
        QString ens_label = toQStringUsingCharset(&label[0], charset, -1);

        if (!ens_label.isEmpty())
        {
            m_dabInfo.m_ens.m_label     = ens_label;
            m_dabInfo.m_ens.m_ueid      = ens;
            m_dabInfo.m_ens.m_charset   = charset;
            m_dabInfo.m_ens.m_charFlag  = bitmap;

            //setEnsembleName();
        }
    }

    return (ret == ETAL_RET_SUCCESS) ? true : false;
}

bool Etal::getServicesInfo(qint32 ens)
{
    QString         msg;
    qint32          ret;

    // QList<quint32>  services;

    // only audio services
    // TODO: this update might be removed if etal gives correct servis list ufter tune
    m_services = getServiceList(ens, true, false);

    // clean everything
    m_dabInfo.m_service.clear();

    Q_FOREACH (quint32 item, m_services)
    {
        EtalServiceInfo info;
        EtalServiceComponentList compList;
        EtalDABServiceInfo serviceInfo;
        quint32 dummy;

        ret = etal_get_specific_service_data_DAB(ens, item, &info, &compList, &dummy);

        if (ret != ETAL_RET_SUCCESS)
        {
            msg = Q_FUNC_INFO + QString(": etal_get_specific_service_data_DAB() error, ret ") + QString::number(ret);
            qDebug() << "#### bad pi " << QString("%1").arg(item, 0, 16);
            emit logging(msg, QString('\n'), 1);
            continue; // if error go to the next service
        }
        else
        {
            msg = Q_FUNC_INFO + QString(": etal_get_specific_service_data_DAB() done, ret ") + QString::number(ret);
            qDebug() << "#### good pi " << QString("%1").arg(item, 0, 16);
            emit logging(msg, QString('\n'), 1);
        }

        switch (info.m_serviceLabelCharset)
        {
            case 0x0:
            case 0x6:
            case 0xf:
                serviceInfo.m_label = toQStringUsingCharset(&info.m_serviceLabel[0], info.m_serviceLabelCharset, -1);
                break;

            default:
                serviceInfo.m_label = QString("not handled");
                break;
        }

        serviceInfo.m_bitrate   = info.m_serviceBitrate;
        serviceInfo.m_id        = item;

        qDebug() << "label    " << serviceInfo.m_label  << ",bit rate " << serviceInfo.m_bitrate << " ,charset " << info.m_serviceLabelCharset;

        m_dabInfo.m_service.append(serviceInfo);
    }

    return (m_dabInfo.m_service.count()) ? true : false;
}

void etalRDSCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext)
{
    Q_UNUSED(status);
    Q_UNUSED(dwActualBufferSize);

    EtalRDSData* prds = (EtalRDSData *)pBuffer;
    Etal* petal = (Etal *)pvContext;

    qDebug() << Q_FUNC_INFO;
    petal->setRdsData(prds);
}

void etalRadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext)
{
    Q_UNUSED(status);
    Q_UNUSED(dwActualBufferSize);
    Q_UNUSED(pvContext);

    EtalTextInfo* radio_text = (EtalTextInfo *)pBuffer;
    Etal* petal = (Etal *)pvContext;

    qDebug() << Q_FUNC_INFO;

    if (radio_text->m_serviceNameIsNew)
    {
        petal->setServiceName(petal->toQStringUsingCharset(radio_text->m_serviceName, radio_text->m_currentInfoCharset, -1));
        qDebug() << "Radio text Service name: " << petal->toQStringUsingCharset(radio_text->m_serviceName, radio_text->m_currentInfoCharset, -1);
    }

    if (radio_text->m_currentInfoIsNew)
    {
        petal->setRadioText(petal->toQStringUsingCharset(radio_text->m_currentInfo, radio_text->m_currentInfoCharset, -1));
        qDebug() << "Radio text Current Info: " << petal->toQStringUsingCharset(radio_text->m_currentInfo, radio_text->m_currentInfoCharset, -1);
    }
}

quint32 cntr = 0;

void etalUserNotificationHandler(void* context, ETAL_EVENTS dwEvent, void* pstatus)
{
    Etal* petal = (Etal *)context;
    QString msg;

    qDebug() << Q_FUNC_INFO;

    if (dwEvent == ETAL_INFO_TUNE)
    {
        EtalTuneStatus* status = (EtalTuneStatus *)pstatus;
        qDebug() << Q_FUNC_INFO;
        qDebug() << "freq    "  << status->m_stopFrequency;
        qDebug() << "state   "  << status->m_sync;
        qDebug() << "service "  << status->m_serviceId;

        petal->updateDAB(status);
    }
    else if (ETAL_INFO_SEEK == dwEvent)
    {
        petal->seekHandler(pstatus);
    }
    else if (ETAL_INFO_LEARN == dwEvent)
    {
        qDebug() << "learn call back";

        EtalLearnStatusTy* status = (EtalLearnStatusTy *)pstatus;

        if (ETAL_LEARN_FINISHED == status->m_status)
        {
            qDebug() << "Learn freq. numb: " << status->m_nbOfFrequency;
            qDebug() << "Cntr: "  << cntr;

            for (quint32 i = 0; i < status->m_nbOfFrequency; ++i)
            {
                qDebug() << "freq " <<  petal->m_freqList[i].m_frequency << ", fst " << petal->m_freqList[i].m_fieldStrength;
            }

            petal->sendSeekFreqFoundSignal();
        }
        else
        {
            qDebug() << "freq " <<  status->m_frequency << " status " << status->m_status;
            cntr++;
        }
    }
    else if (ETAL_INFO_SEAMLESS_ESTIMATION_END == dwEvent)
    {
        EtalSeamlessEstimationStatus* seamless_estimation_status = (EtalSeamlessEstimationStatus *)pstatus;

        msg = "Seamless estimation end, state " + QString(seamless_estimation_status->m_status);

        qDebug() << msg;

        emit petal->logging(msg, QString('\n'), 1);
    }
    else if (ETAL_INFO_SEAMLESS_SWITCHING_END == dwEvent)
    {
        EtalSeamlessSwitchingStatus* seamless_switching_status = (EtalSeamlessSwitchingStatus *)pstatus;

        msg = "Seamless switching end, state " + QString::number(seamless_switching_status->m_status);

        qDebug() << msg;

        emit petal->logging(msg, QString('\n'), 1);
    }
    else if (ETAL_INFO_SERVICE_FOLLOWING_NOTIFICATION_INFO == dwEvent)
    {
        EtalTuneServiceIdStatus* tuneServiceIdStatus = (EtalTuneServiceIdStatus *)pstatus;

        msg = "Service following event, freq " + QString::number(tuneServiceIdStatus->m_freq) \
            + ", is DAB " + QString::number(tuneServiceIdStatus->m_freqIsDab) \
            + ", AF available " + QString::number(tuneServiceIdStatus->m_AFisAvailable) \
            + ", AF sync " + QString::number(tuneServiceIdStatus->m_AFisSync);

        qDebug() << msg;

        emit petal->logging(msg, QString('\n'), 1);
    }
    else
    {
        qDebug() << "Unhandled event " << dwEvent;
    }
}

void etalSLSCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext)
{
    Q_UNUSED(status);

    qDebug() << "sls callback";

    QByteArray sls((const char *)&pBuffer[4], dwActualBufferSize);
    Etal* petal = (Etal *)pvContext;
    EtalGenericDataServiceRaw* DataServiceRawBuffer;

    DataServiceRawBuffer = (EtalGenericDataServiceRaw *)pBuffer;

    switch (DataServiceRawBuffer->m_dataType)
    {
        case ETAL_DATASERV_TYPE_SLS_XPAD:
            qDebug() << "Data Service SLS XPAD callback received packet size " << dwActualBufferSize;
            petal->setSLS(sls);
            break;

        case ETAL_DATASERV_TYPE_SLS:
            qDebug() << "Data Service SLS callback received packet size " << dwActualBufferSize;
            break;

        default:
            qDebug() << "unknown event ... ";
            break;
    }
}

void etalQualityMonitor(EtalBcastQualityContainer* pQuality, void* vpContext)
{
    Etal* petal = (Etal *)vpContext;

    petal->monitorUpdate(pQuality);
}

#endif // #if (defined CONFIG_USE_ETAL)
