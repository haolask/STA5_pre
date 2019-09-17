#ifndef RDS_H_
#define RDS_H_

#include "common.h"

enum rdsErrorTypes
{
    RDS_ERR_BLOCK_A     = 0,
    RDS_ERR_BLOCK_B     = 1,
    RDS_ERR_BLOCK_C     = 2,
    RDS_ERR_BLOCK_D     = 3,
    RDS_ERR_WRONGSEQ    = 4
};

struct rdsErrorData
{
    int errCount;
    int wrongBlockA;
    int wrongBlockB;
    int wrongBlockC;
    int wrongBlockD;
    int wrongSequence;
};

class RdsDecoder
{
    public:
        RdsDecoder()
        {
            RdsInit();
        }

        void ClearRdsData();
        bool NewDataAvailable();
        int GetNewData(RdsDataSetTableTy& dataBuffer);
        void ProcessRawData(QByteArray rawdata);
        void SetExtendedCountryCode();
        void UpdateRdsBlock(QByteArray strgRds);
        void setRdsFmTuneFrequency(unsigned int fmRdsFreq);

    private:
        void RdsInit();
        void Rt_ExtSegment();
        void Rt_ExtData(quint8 block);
        void Rt_Method2ExtData(quint8 block);
        void ClearAllAuxData();
        void ClearRdsAllLabels();
        void ClearRdsAllData();

        QString SetCTData(quint32 cT_MDJ, quint32 cT_HH, quint32 cT_MM, quint32 cT_offset);

        void UpdateBlockErrDisplay();
        bool CheckValidDataRds(quint8 idx);
        bool CheckValidBlocksRds(QByteArray readNotifReg);
        void Inc_BlkErrRate(rdsErrorTypes errType);
        bool IsRdsAFEnabled();

        void AF_MethAExt(quint8 newAFH, quint8 newAFL);
        void AF_MethBExt(quint8 newAFH, quint8 newAFL);
        void AF_HeaderExt(quint8 newAFL);
        void AF_Insert(quint8 nvalore);
        void AF_Extract(quint8 RdsAfNewH, quint8 RdsAfNewL);

        int nbRdsErrorBits;
        RdsDataSetTableTy rdsDataSet;
        bool rdsAFenable;

        int CountRds;
        int previousRdsBlock;
        quint8 PsAddr;
        quint8 ptynAddr;
        QString ptynStr;
        QString NewPdata;
        quint8 maskPsAddr;
        bool gotNewRdsInfo;
        quint8 ExtCCode;
        int CT_MDJ;
        int CT_HH;
        int CT_MM;
        int CT_offset;
        int totRdsBlks;
        int rdsDisplayState;
        int rdsStatus;
        rdsErrorData ErrBlks;
        quint8 rdsBlkHist;
        quint8 rdsCurBlock;
        qint8 rdsGrpType;
        QString rdsPsM1;
        QString rdsPsM2;
        quint8 rdsGrpCntM1;
        quint8 rdsGrpCntM2;
        qint8 rdsSegAddrM1;
        qint8 rdsSegAddrM2;
        QString rdsPiHist;

        quint8 GroupRds;
        bool   Rt_type2B;
        quint8 Rt_Segment;
        quint8 Rt_Segment_Last;
        quint32 Rt_mask;
        QByteArray RT_String;
        QByteArray RT_CopyString;

        quint8 BuffIn[3];
        quint8 AfFirst;
        bool AF_MethDetected_flg;
        bool AFHeader_ON_flg;
        bool AF_MethBOn_flg;  // 'true' = MethodB, 'false' = MethodA
        quint8 Curr_NbAF;
        bool afHexCode;
        quint8 fmTuneFreq;
        quint8 taOnFlgCnt;
        quint8 taOffFlgCnt;
};

#endif // RDS_H_

// End of file
