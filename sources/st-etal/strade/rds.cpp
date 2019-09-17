#include <iostream>
#include <fstream>

#include "rds.h"

#define RDS_BLOCK_A             0
#define RDS_BLOCK_B             1
#define RDS_BLOCK_C             2
#define RDS_BLOCK_D             3

#define AF_NO_FREQ              205
#define RDS_NUMBER_AF           25

#define RT_MASK_COMPLETE        0xFFFFFFFF

#define INVALID_RDS_DATA        -1

enum rdsBlockType
{
    RDS_BLOCK_0A     = 0,
    RDS_BLOCK_1A     = 1,
    RDS_BLOCK_2A     = 2,
    RDS_BLOCK_3A     = 3,
    RDS_BLOCK_4A     = 4,
    RDS_BLOCK_5A     = 5,
    RDS_BLOCK_6A     = 6,
    RDS_BLOCK_7A     = 7,
    RDS_BLOCK_8A     = 8,
    RDS_BLOCK_9A     = 9,
    RDS_BLOCK_AA     = 10,
    RDS_BLOCK_BA     = 11,
    RDS_BLOCK_CA     = 12,
    RDS_BLOCK_DA     = 13,
    RDS_BLOCK_EA     = 14,
    RDS_BLOCK_FA     = 15,
    RDS_BLOCK_0B     = 16,
    RDS_BLOCK_1B     = 17,
    RDS_BLOCK_2B     = 18,
    RDS_BLOCK_3B     = 19,
    RDS_BLOCK_4B     = 20,
    RDS_BLOCK_5B     = 21,
    RDS_BLOCK_6B     = 22,
    RDS_BLOCK_7B     = 23,
    RDS_BLOCK_8B     = 24,
    RDS_BLOCK_9B     = 25,
    RDS_BLOCK_AB     = 26,
    RDS_BLOCK_BB     = 27,
    RDS_BLOCK_CB     = 28,
    RDS_BLOCK_DB     = 29,
    RDS_BLOCK_EB     = 30,
    RDS_BLOCK_FB     = 31
};

void RdsDecoder::RdsInit()
{
    gotNewRdsInfo = false;
    rdsSegAddrM1 = INVALID_RDS_DATA;
    rdsSegAddrM2 = INVALID_RDS_DATA;
    rdsPsM1.clear();
    rdsPsM2.clear();
    // Init rds
    nbRdsErrorBits = 2;
    rdsDataSet.PIid.clear();
    rdsDataSet.PSname.clear();
    rdsDataSet.tpFlag = false;
    rdsDataSet.taFlag = false;
    taOffFlgCnt = 0;
    taOnFlgCnt = 0;
    rdsDataSet.ptyVal = 0;
    rdsDataSet.diVal = 0;
    rdsDataSet.countryName.clear();
    rdsDataSet.ECC_label.clear();
    rdsDataSet.rtText.clear();
    Rt_mask = 0;

    memset(rdsDataSet.RdsAFList, AF_NO_FREQ, (RDS_NUMBER_AF + 1));
    rdsAFenable = true;
}

void RdsDecoder::ClearRdsData()
{
    ClearRdsAllLabels();
    ClearRdsAllData();
}

bool RdsDecoder::NewDataAvailable()
{
    return gotNewRdsInfo;
}

int RdsDecoder::GetNewData(RdsDataSetTableTy& dataBuffer)
{
    dataBuffer = rdsDataSet;

    gotNewRdsInfo = false;

    return 0;
}

void RdsDecoder::ProcessRawData(QByteArray rawdata)
{
    QByteArray currRdsData;

    // TODO: This function knows about the data format, should not be here

    // Discharge first word
    rawdata = rawdata.right(rawdata.length() - 3);

    // Discharge last word (checksum)
    rawdata = rawdata.left(rawdata.length() - 3);

    // Initialize counter RDS blocks to value 2 since CountRds=3 blk A, CountRds=2 BlkD CountRds=1 BlkC CountRds=0 BlkB
    CountRds = 2;

    // First extract the read notification register (first word)
    currRdsData = rawdata.left(3);

    if (true == CheckValidBlocksRds(currRdsData))
    {
        rawdata = rawdata.right(rawdata.length() - 3);

        while (rawdata.length() >= 3)
        {
            currRdsData = rawdata.left(3);
            rawdata = rawdata.right(rawdata.length() - 3);

            UpdateRdsBlock(currRdsData);
        }
    }
}

void RdsDecoder::SetExtendedCountryCode()
{
    QList<QString> rdsCountrieslist;
    rdsCountrieslist << "USA" << "GREECE " << "IRELAND" << "POLAND" << "SWITZERLAND" << "ITALY" << "BELGIUM" <<
        "LUXEMBOURG" << "NETHERLAND" << "DENMARK" << "AUSTRIA" << "HUNGARY" << "UK" <<
        "GERMANY" << "SPAIN" << "FRANCE";

    QList<QString> rdsCountriesE0list;
    rdsCountriesE0list << "USA" << "GREECE " << "ALGERIA" << "ANDORRA" << "ISRAEL" << "ITALY" << "BELGIUM" <<
        "RUSSIA" << "PALESTINE" << "ALBANIA" << "AUSTRIA" << "HUNGARY" << "MALTA" <<
        "GERMANY" << "SPAIN" << "EGYPT";

    QList<QString> rdsCountriesE1list;
    rdsCountriesE1list << "USA" << "GREECE " << "CYPRUS" << "SANMARINO" << "SWITZERLAND" << "JORDAN" << "FINLAND" <<
        "LUXEMBOURG" << "BULGARIA" << "FAROE" << "GIBRALTAR" << "IRAQ" << "UK" <<
        "LIBYA" << "ROMANIA" << "FRANCE";

    QList<QString> rdsCountriesE2list;
    rdsCountriesE2list << "USA" << "MOROCCO " << "CZECH REPUBLIC" << "POLAND" << "VATICAN_STATE" << "SLOVAKIA" << "BELGIUM" <<
        "SYRIAN" << "TUNISIA" << "DENMARK" << "ICELAND" << "MONACO" << "LITHUANIA" <<
        "YUGOSLAVIA" << "SPAIN" << "NORWAY";

    QList<QString> rdsCountriesE3list;
    rdsCountriesE3list << "USA" << "GREECE " << "IRELAND" << "TURKEY" << "MACEDONIA" << "TAJIKISTAN" << "BELGIUM" <<
        "LUXEMBOURG" << "NETHERLAND" << "LATVIA" << "LEBANON" << "AZERBAIJAN" << "CROATIA" <<
        "KAZAKHSTAN" << "SWEDEN" << "BELARUS";

    QList<QString> rdsCountriesE4list;
    rdsCountriesE4list << "USA" << "MOLDOVA " << "ESTONIA" << "KYRGHYZSTAN" << "SWITZERLAND" << "ITALY" << "UKRAINE" <<
        "LUXEMBOURG" << "PORTUGAL" << "SLOVENIA" << "ARMENIA" << "UZBEKISTAN" << "UK" <<
        "GEORGIA" << "TURKMENISTAN" << "BOSNIA_HERZEGOVINA";

    QList<QString> rdsCountriesD0list;
    rdsCountriesD0list << "USA" << "CAMEROON " << "CENTRAL_AFRICA_REPUB" << "DJIBOUTI" << "MADAGASCAR" << "MALI" << "ANGOLA" <<
        "EQUATORIAL_GUINEA" << "GABON" << "REPUBLIC_GUINEA" << "SOUTH_AFRICA" << "BURKINA FASO" << "CONGO" <<
        "TOGO" << "BENIN" << "MALAWI";

    QList<QString> rdsCountriesD1list;
    rdsCountriesD1list << "USA" << "NAMIBIA " << "LIBERIA" << "GHANA" << "MAURITANIA" << "SAO_TOME_PRINCIPE" << "CAPE_VERDE" <<
        "SENEGAL" << "GAMBIA" << "BURUNDI" << "ASCENSION_ISLAND" << "BOTSWANA" << "COMOROS" <<
        "TANZANIA" << "ETHIOPIA" << "NIGERIA";

    QList<QString> rdsCountriesD2list;
    rdsCountriesD2list << "USA" << "SIERRA_LEONE " << "ZIMBABWE" << "MOZAMBIQUE" << "UGANDA" << "SWAZILAND" << "KENYA" <<
        "SOMALIA" << "NIGER" << "CHAD" << "GUINEA_BISSAU" << "DEMOCR_REPUBLIC_CONGO" << "COTE_D'IVOIRE" <<
        "ZANZIBAR" << "ZAMBIA" << "NORWAY";

    QList<QString> rdsCountriesD3list;
    rdsCountriesD3list << "USA" << "_____ " << "_____" << "WESTERN_SAHARA" << "CABINDA" << "RWANDA" << "LESOTHO" <<
        "_____" << "SECHELLES" << "LATVIA" << "MAURITUIS" << "_____" << "SUDAN" <<
        "_____" << "_____" << "BELARUS";

    QList<QString> rdsCountriesA0list;
    rdsCountriesA0list << "USA" << "USA" << "USA" << "USA" << "USA" << "USA" << "USA" <<
        "USA" << "USA" << "USA" << "USA" << "USA" << "_____" <<
        "USA" << "USA" << "_____";

    QList<QString> rdsCountriesA1list;
    rdsCountriesA1list << "USA" << "GREECE " << "CYPRUS" << "SANMARINO" << "SWITZERLAND" << "JORDAN" << "FINLAND" <<
        "LUXEMBOURG" << "BULGARIA" << "FAROE" << "GIBRALTAR" << "CANADA" << "CANADA" <<
        "CANADA" << "CANADA" << "GREENLAND";

    QList<QString> rdsCountriesA2list;
    rdsCountriesA2list << "USA" << "ANGUILLA " << "ANTIGUA_AND_BARBUDA" << "ECUADOR" << "FALKLAND_ISLANDS" << "BARBADOS" << "BELIZE" <<
        "CAYMAN_ISLANDS" << "COSTA_RICA" << "CUBA" << "ARGENTINA" << "BRAZIL" << "BERMUDA" <<
        "NETHERLANDS_ANTILLES" << "GUADELOUPE" << "BAHAMAS";

    QList<QString> rdsCountriesA3list;
    rdsCountriesA3list << "USA" << "BOLIVIA " << "COLOMBIA" << "JAMAICA" << "MARTINIQUE" << "GUIANA" << "PARAGUAY" <<
        "NICARAGUA" << "NETHERLAND" << "PANAMA" << "DOMINICA" << "DOMINICAN_REPUBLIC" << "CHILE" <<
        "GRENADA" << "TURKS_CAICOS_ISLANDS" << "GUYANA";

    QList<QString> rdsCountriesA4list;
    rdsCountriesA4list << "USA" << "GUATEMALA " << "HONDURAS" << "ARUBA" << "SWITZERLAND" << "MONTSERRAT" << "TRINIDAD_TOBAGO" <<
        "PERU" << "SURINAME" << "URUGUAY" << "SAINT KITTS" << "SAINT_LUCIA" << "EL_SALVADOR" <<
        "HAITI" << "VENEZUELA" << "BOSNIA_HERZEGOVINA";

    QList<QString> rdsCountriesA5list;
    rdsCountriesA5list << "USA" << "MOLDOVA " << "ESTONIA" << "POLAND" << "SWITZERLAND" << "ITALY" << "UKRAINE" <<
        "LUXEMBOURG" << "PORTUGAL" << "SLOVENIA" << "AUSTRIA" << "MEXICO" << "SAINT_VINCENT" <<
        "MEXICO" << "MEXICO" << "MEXICO";

    QList<QString> rdsCountriesF0list;
    rdsCountriesF0list << "USA" << "AUSTRALIA_CAPITAL_TERRITORY" << "NEW_SOUTH_WALES" << "VICTORIA" << "QEENSLAND" << "SOUTH_AUSTRALIA" << "WESTERN_AUSTRALIA" <<
        "TASMANIA" << "NORTHERN_TERRITORY" << "SAUDI_ARABIA" << "AFGHANISTAN" << "MYANMAR_BURMA" << "CHINA" <<
        "KOREA_NORTH" << "BAHRAIN" << "MALAYSIA";
    QList<QString> rdsCountriesF1list;
    rdsCountriesF1list << "USA" << "KIRIBATI" << "BHUTAN" << "BANGLADESH" << "PAKISTAN" << "FIJI" << "OMAN" <<
        "NAURU" << "IRAN" << "NEW_ZEALAND" << "SOLOMON_ISLANDS" << "BRUNEI_DARUSSALAM" << "SRI_LANKA" <<
        "TAIWAN" << "KOREA_SOUTH" << "HONG_KONG";
    QList<QString> rdsCountriesF2list;
    rdsCountriesF2list << "USA" << "KUWAIT" << "QATAR" << "CAMBODIA" << "WESTERN_SAMOA" << "INDIA" << "MACAU" <<
        "VIETNAM" << "PHILIPPINES" << "JAPAN" << "SINGAPORE" << "MALDIVES" << "SAINT_VINCENT" <<
        "UAE" << "NEPAL" << "VANUATU";
    QList<QString> rdsCountriesF3list;
    rdsCountriesF3list << "USA" << "LAOS" << "THAILAND" << "TONGA" << "SWITZERLAND" << "ITALY" << "UKRAINE" <<
        "LUXEMBOURG" << "PORTUGAL" << "PAPUA_NEW_GUINEA" << "AUSTRIA" << "YEMEN" << "SAINT_VINCENT" <<
        "MEXICO" << "MICRONESIA" << "MONGOLIA";

    bool ok;

    if (ExtCCode == 0)
    {
        rdsDataSet.ECC_label.clear();
    }
    else
    {
        rdsDataSet.ECC_label = "ecc=" + QString::number(ExtCCode, 16);
    }

    if (true == rdsDataSet.PIid.isEmpty())
    {
        rdsDataSet.countryName.clear();
    }
    else
    {
        switch (ExtCCode)
        {
            case 0xE0:
                rdsDataSet.countryName = rdsCountriesE0list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0:
                rdsDataSet.countryName = rdsCountrieslist.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xE1:
                rdsDataSet.countryName = rdsCountriesE1list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xE2:
                rdsDataSet.countryName = rdsCountriesE2list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xE3:
                rdsDataSet.countryName = rdsCountriesE3list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xE4:
                rdsDataSet.countryName = rdsCountriesE4list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xD0:
                rdsDataSet.countryName = rdsCountriesD0list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xD1:
                rdsDataSet.countryName = rdsCountriesD1list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xD2:
                rdsDataSet.countryName = rdsCountriesD2list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xD3:
                rdsDataSet.countryName = rdsCountriesD3list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xA0:
                rdsDataSet.countryName = rdsCountriesA0list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xA1:
                rdsDataSet.countryName = rdsCountriesA1list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xA2:
                rdsDataSet.countryName = rdsCountriesA2list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xA3:
                rdsDataSet.countryName = rdsCountriesA3list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xA4:
                rdsDataSet.countryName = rdsCountriesA4list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xA5:
                rdsDataSet.countryName = rdsCountriesA5list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xF0:
                rdsDataSet.countryName = rdsCountriesF0list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xF1:
                rdsDataSet.countryName = rdsCountriesF1list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xF2:
                rdsDataSet.countryName = rdsCountriesF2list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            case 0xF3:
                rdsDataSet.countryName = rdsCountriesF3list.at(rdsDataSet.PIid.left(1).toInt(&ok, 16));
                break;

            default:
                // No code
                break;
        }
    }
}

// Clear all RDS labels
void RdsDecoder::ClearRdsAllLabels()
{
    gotNewRdsInfo = false;
    rdsSegAddrM1 = INVALID_RDS_DATA;
    rdsSegAddrM2 = INVALID_RDS_DATA;
    rdsDataSet.PSname.clear();
    rdsPsM1.clear();
    rdsPsM2.clear();
    rdsDataSet.PIid.clear();
    rdsDataSet.ECC_label.clear();
    rdsDataSet.countryName.clear();
    maskPsAddr = 0;
    rdsDataSet.ptyVal = 0;
    rdsDataSet.ptyName.clear();
    rdsDataSet.tpFlag = false;
    rdsDataSet.taFlag = false;
    rdsDataSet.msFlag = false;
}

// Clear all RDS data (new PI detected or user changeFreq)
void RdsDecoder::ClearRdsAllData()
{
    ExtCCode = 0;

    SetExtendedCountryCode();

    Rt_Segment_Last = 0;

    RT_String.clear();
    RT_CopyString.clear();

    rdsDataSet.diVal = 0;
    rdsDataSet.rtText.clear();
    Rt_mask = 0;

    ClearAllAuxData();
}

void RdsDecoder::UpdateRdsBlock(QByteArray strgRds)
{
    QString tmpStr;

    QList<QString> rdsPtylist;
    rdsPtylist << "None" << "News" << "Current Affairs" << "Information" << "Sport" << "Education" << "Drama" <<
        "Cultures" << "Science" << "Varied Speech" << "Pop Music" << "Rock Music" << "Easy Listening" <<
        "Light Classic M" << "Serious Classic" << "Other Music" << "Weather and Metr" << "Finance" <<
        "Children Progs" << "Social Affairs" << "Religion" << "Phone In" << "Travel - Touring"
               << "Leisure - Hobby" << "Jazz Music" << "Country Music" << "National Music" << "Oldies Music"
               << "Folk Music" << "Documentary" << "Alarm Test" << "Alarm - Alarm!";

    QList<QString> rbdsPtylist;
    rbdsPtylist << "None" << "News" << "Information" << "Sports" << "Talk" << "Rock" <<
        "Classic Rock" << "Adult Hits" << "Soft Rock" << "Top 40" << "Country" << "Oldies" <<
        "Soft" << "Nostalgia" << "Jazz" << "Classical" << "Rhythm and Blues" <<
        "Soft Rhyt Blues" << "Foreign Language" << "Religious Music" << "Religious Talk" << "Personality"
                << "Public" << "College " << "Unassigned" << "Unassigned" << "Unassigned" << "Unassigned"
                << "Unassigned" << "Weather" << "Emergency Test" << "Alert - Alert!";

    if (true == strgRds.isEmpty())
    {
        qDebug() << "WARNING: No RDS data detected";

        return;
    }

    while (strgRds.length() < 3)
    {
        strgRds = "0" + strgRds;
    }

    BuffIn[0] = strgRds[0];
    BuffIn[1] = strgRds[1];
    BuffIn[2] = strgRds[2];

    //b23       reserved
    //b22-20    error count: 000 = no_error, 001 = single err, 010 = double err, 110 = uncorrectable, 111 = unavailable
    //b19       reserved
    //b18       C type
    //b17-16    blk ID
    //b15..0    RDS data
    nbRdsErrorBits = 2;  // Number error bits force = 2 (could be 0, 1, 2)

    // Track expected blocks, so we know if something was missed
    rdsCurBlock = (rdsCurBlock + 1) & 0x03; // 0-3 counter, representing A-D

    if ((0x00 == rdsCurBlock) || (0x01 == rdsCurBlock))
    {
        if ((RDS_BLOCK_0A == rdsGrpType) || (RDS_BLOCK_0B == rdsGrpType))
        {
            // We missed a D block, when PS was expected
            rdsSegAddrM1 = INVALID_RDS_DATA;
            rdsSegAddrM2 = INVALID_RDS_DATA;
        }

        rdsGrpType = INVALID_RDS_DATA; // Set invalid until we see valid B
    }

    if (true == CheckValidDataRds(nbRdsErrorBits))
    {
        switch (BuffIn[0] & 0x03)
        {
            // Block A
            case RDS_BLOCK_A:
                rdsBlkHist = 0x01; // Saw A; Remove B, C, & D flags
                rdsCurBlock = 0x00; // Current Block is A

                // We increment this counter that is not really a counter but it is used
                // to check the correct sequence of blocks
                CountRds++;

                tmpStr = QString::fromLocal8Bit(strgRds.mid(1, 2).toHex().toUpper());

                // Require Valid PI Code to be received 2x
                if (tmpStr != rdsPiHist)
                {
                    rdsPiHist = tmpStr;
                }
                else
                {
                    if (rdsDataSet.PIid != tmpStr)
                    {
                        ClearRdsAllData();

                        previousRdsBlock = RDS_BLOCK_A;

                        // Saw PI >= 2 times, and not equal to what is displayed
                        rdsDataSet.PIid = tmpStr;

                        gotNewRdsInfo = true;
                    }
                    else
                    {
                        if (CountRds != 3)
                        {
                            if (previousRdsBlock != RDS_BLOCK_B)
                            {
                                // Error
                            }

                            previousRdsBlock = RDS_BLOCK_A;

                            qDebug() << "RDS: CNT RDS BLOCK A = " << CountRds;
                            qDebug() << "ERROR RDS: WRONG RDS BLOCK A";

                            Inc_BlkErrRate(RDS_ERR_WRONGSEQ); // RDS_ERR_BLOCK_A
                        }
                        else
                        {
                            previousRdsBlock = RDS_BLOCK_A;
                        }
                    }
                }
                break; // End of Block A

            // Block B
            case RDS_BLOCK_B:
                if (0x01 != rdsCurBlock)
                {
                    // B was not expected
                    rdsBlkHist = 0x2; // Saw B, but not A
                }
                else
                {
                    rdsBlkHist = rdsBlkHist | 0x02; // Saw B
                    rdsBlkHist = rdsBlkHist & 0x03; // Remove C & D Flags
                }

                rdsCurBlock = 0x01; // Current Block is B

                // This is the first valid block: we reset the counter for correct sequence
                if (previousRdsBlock != RDS_BLOCK_A)
                {
                    // Error (not to be exposed)
                    qDebug() << "ERROR RDS: wrong block B";

                    Inc_BlkErrRate(RDS_ERR_WRONGSEQ);
                }

                CountRds = 0;

                previousRdsBlock = RDS_BLOCK_B;

                if (BuffIn[1] & 0x08)
                {
                    GroupRds = 16 + (BuffIn[1] & 0xF0) / 16;
                }
                else
                {
                    GroupRds = (BuffIn[1] & 0xF0) / 16;
                }

                rdsGrpType = GroupRds;

                if ((GroupRds == RDS_BLOCK_0A) || (GroupRds == RDS_BLOCK_0B) || (GroupRds == RDS_BLOCK_FB))
                {
                    if (BuffIn[2] & 0x10)  //TA flag  status
                    {
                        if (false == rdsDataSet.taFlag)
                        {
                            taOnFlgCnt++;
                            qDebug() << "taOnFlgCnt = " << taOnFlgCnt;
                            if (taOnFlgCnt >= 2)
                            {
                                rdsDataSet.taFlag = true;

                                if (false == rdsDataSet.PIid.isEmpty())
                                {
                                    qDebug() << "taOnFlgCnt  resetted to 0 -> TA ON ";
                                    taOnFlgCnt = 0;
                                    gotNewRdsInfo = true;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (true == rdsDataSet.taFlag)
                        {
                            taOffFlgCnt++;
                            qDebug() << "taOffFlgCnt = " << taOffFlgCnt;
                            if (taOffFlgCnt >= 2)
                            {
                                rdsDataSet.taFlag = false;
                                if (false == rdsDataSet.PIid.isEmpty())
                                {
                                    qDebug() << "taOffFlgCnt  resetted to 0 -> TA OFF ";
                                    taOffFlgCnt = 0;
                                    gotNewRdsInfo = true;
                                }
                            }
                        }
                    }

                    if (BuffIn[1] & 0x04)  //TP flag  status
                    {
                        if (false == rdsDataSet.tpFlag)
                        {
                            rdsDataSet.tpFlag = true;
                            if (false == rdsDataSet.PIid.isEmpty())
                            {
                                gotNewRdsInfo = true;
                            }
                        }
                    }
                    else
                    {
                        if (true == rdsDataSet.tpFlag)
                        {
                            rdsDataSet.tpFlag = false;
                            if (false == rdsDataSet.PIid.isEmpty())
                            {
                                gotNewRdsInfo = true;
                            }
                        }
                    }

                    // MS flag status not yet implemented
                    quint8 tmpPtyval = 8 * (BuffIn[1] & 0x03) + (BuffIn[2] & 0xE0) / 0x20;
                    if (rdsDataSet.ptyVal != tmpPtyval)
                    {
                        rdsDataSet.ptyVal = tmpPtyval;

                        rdsDataSet.ptyName = rdsPtylist[rdsDataSet.ptyVal];

                        if (false == rdsDataSet.PIid.isEmpty())
                        {
                            gotNewRdsInfo = true;
                        }
                    }
                }

                switch (GroupRds)
                {
                    case RDS_BLOCK_0A:
                    case RDS_BLOCK_0B:
                        PsAddr = BuffIn[2] & 0x03;

                        if ((0 == PsAddr) || (1 == PsAddr - rdsSegAddrM1))
                        {
                            // Require Method 1 segments to be seen in proper order
                            rdsSegAddrM1 = PsAddr;
                        }
                        else
                        {
                            rdsSegAddrM1 = INVALID_RDS_DATA;
                        }

                        if ((0 == PsAddr) || (1 == PsAddr - rdsSegAddrM2))
                        {
                            // Require Method 2 segments to be seen in proper order
                            rdsSegAddrM2 = PsAddr;
                        }
                        else
                        {
                            rdsSegAddrM2 = INVALID_RDS_DATA;
                        }

                        switch (PsAddr)
                        {
                            case 0x00:
                                if (BuffIn[2] & 0x04)
                                {
                                    rdsDataSet.diVal |= 0x08;
                                }
                                else
                                {
                                    rdsDataSet.diVal &= 0xF7;
                                }
                                break;

                            case 0x01:
                                if (BuffIn[2] & 0x04)
                                {
                                    rdsDataSet.diVal |= 0x04;
                                }
                                else
                                {
                                    rdsDataSet.diVal &= 0xFB;
                                }
                                break;

                            case 0x02:
                                if (BuffIn[2] & 0x04)
                                {
                                    rdsDataSet.diVal |= 0x02;
                                }
                                else
                                {
                                    rdsDataSet.diVal &= 0xFD;
                                }
                                break;

                            case 0x03:
                                if (BuffIn[2] & 0x04)
                                {
                                    rdsDataSet.diVal |= 0x01;
                                }
                                else
                                {
                                    rdsDataSet.diVal &= 0xFE;
                                }
                                break;
                        }

                        //rdsAux->SetRdsDivalue(diVal);
                        break;

                    case RDS_BLOCK_2A:
                        Rt_type2B = false;
                        Rt_ExtSegment();
                        break;

                    case RDS_BLOCK_2B:
                        Rt_type2B = true;
                        Rt_ExtSegment();
                        break;

                    case RDS_BLOCK_4A:
                        CT_MDJ = 128 * 256 * (BuffIn[2] & 0x03);
                        break;

                    case RDS_BLOCK_AA:
                        ptynAddr = BuffIn[2] & 0x01;
                        break;

                    default:
                        // No code
                        break;
                }
                break; // End of Block B

            // Block C (C*)
            case RDS_BLOCK_C:
                if (RDS_BLOCK_C != rdsCurBlock)
                {
                    // C was not expected
                    rdsBlkHist = rdsBlkHist & 0x1; // Remove B & D flags
                    rdsGrpType = INVALID_RDS_DATA; // If we didn't see B, then Grp Type cannot be valid
                }

                rdsBlkHist = rdsBlkHist | 0x04; // Saw C
                rdsBlkHist = rdsBlkHist & 0x07; // Remove D Flag
                rdsCurBlock = RDS_BLOCK_C; // Current Block is C (Or C')

                // Check correct sequence
                CountRds++;

                if (previousRdsBlock != RDS_BLOCK_B)
                {
                    // Error
                }

                previousRdsBlock = RDS_BLOCK_C;

                if (CountRds == 1)
                {
                    switch (GroupRds)
                    {
                        case RDS_BLOCK_0A:
                            if (true == IsRdsAFEnabled())
                            {
                                AF_Extract(BuffIn[1], BuffIn[2]);
                            }
                            break;

                        case RDS_BLOCK_1A:
                            // If transmitted extended country code, then store it
                            if ((BuffIn[1] & 0x70) == 0)
                            {
                                ExtCCode = BuffIn[2];

                                SetExtendedCountryCode();
                            }

                            //if (rdsAux->isRdsTmcEnabled())
                            //{
                            //    rdsAux->setTmcData(GroupRds,1,BuffIn[1], BuffIn[2]);
                            //}
                            break;

                        case RDS_BLOCK_2A:
                            Rt_ExtData(0);
                            break;

                        case RDS_BLOCK_4A:
                            CT_MDJ = CT_MDJ + 128 * (BuffIn[1]) + (BuffIn[2] & 0x0FE) / 2;
                            CT_HH = 16 * (BuffIn[2] & 0x01);
                            break;

                        case RDS_BLOCK_3A:
                        case RDS_BLOCK_8A:
                            //if (rdsAux->isRdsTmcEnabled())
                            //{
                            //    rdsAux->setTmcData(GroupRds,1,BuffIn[1], BuffIn[2]);
                            //}
                            break;

                        case RDS_BLOCK_AA:
                            if (ptynAddr == 0)
                            {
                                ptynStr = (strgRds.right(2)).data() + ptynStr.right(6);
                            }
                            else
                            {
                                ptynStr = ptynStr.left(4) + (strgRds.right(2)).data() + ptynStr.right(2);
                            }

                            //rdsAux->setPtynString(ptynStr);
                            break;

                        case RDS_BLOCK_FB:
                            break;
                    }
                }
                else
                {
                    qDebug() << "ERROR RDS: wrong block C";

                    Inc_BlkErrRate(RDS_ERR_WRONGSEQ);

                    CountRds = 1;
                }
                break; // End of Block C (C*)

            // Block D
            case RDS_BLOCK_D:
                if (RDS_BLOCK_D != rdsCurBlock)
                {
                    // D was not expected - we know we did not see C
                    rdsBlkHist = rdsBlkHist & 0x03; // Remove C flag
                }

                rdsBlkHist = rdsBlkHist | 0x08; // Saw D
                rdsCurBlock = RDS_BLOCK_D; // Current Block is D

                // Check for correct sequence
                CountRds++;

                if (previousRdsBlock != RDS_BLOCK_C)
                {
                    // Error
                }

                previousRdsBlock = RDS_BLOCK_D;

                if (CountRds == 2)
                {
                    switch (GroupRds)
                    {
                        case RDS_BLOCK_0A:
                        case RDS_BLOCK_0B:
                            switch (PsAddr)
                            {
                                case 0:
                                    NewPdata = (strgRds.right(2)).data() + rdsDataSet.PSname.right(6);
                                    maskPsAddr |= 1;
                                    break;

                                case 1:
                                    NewPdata = rdsDataSet.PSname.left(2) +
                                        (strgRds.right(2)).data() + rdsDataSet.PSname.right(4);
                                    maskPsAddr |= 2;
                                    break;

                                case 2:
                                    NewPdata = rdsDataSet.PSname.left(4) +
                                        (strgRds.right(2)).data() + rdsDataSet.PSname.right(2);
                                    maskPsAddr |= 4;
                                    break;

                                case 3:
                                    NewPdata = rdsDataSet.PSname.left(6) + (strgRds.right(2)).data();
                                    maskPsAddr |= 8;
                                    break;
                            }

                            break;

                        case RDS_BLOCK_2A:
                        case RDS_BLOCK_2B:
                            Rt_ExtData(1);
                            break;

                        case RDS_BLOCK_4A:
                            CT_HH = CT_HH + (BuffIn[1] & 0xF0) / 16;
                            CT_MM = 4 * (BuffIn[1] & 0x0F);
                            CT_MM = CT_MM + (BuffIn[2] & 0xC0) / 64;
                            CT_offset = BuffIn[2] & 0x3F;
                            break;

                        case RDS_BLOCK_1A:
                        case RDS_BLOCK_3A:
                        case RDS_BLOCK_8A:
                            break;

                        case RDS_BLOCK_AA:
                            if (ptynAddr == 0)
                            {
                                ptynStr = ptynStr.left(2) + (strgRds.right(2)).data() + ptynStr.right(4);
                            }
                            else
                            {
                                ptynStr = ptynStr.left(6) + (strgRds.right(2)).data();
                            }

                            break;

                        case RDS_BLOCK_FB:
                            break;
                    }
                }
                else
                {
                    qDebug() << "ERROR RDS: wrong Block D";

                    Inc_BlkErrRate(RDS_ERR_WRONGSEQ);

                    CountRds = 2;
                }
                break; // End of Block
        }

        totRdsBlks++;

        UpdateBlockErrDisplay();
    }
    else
    {
        // Invalid block
        rdsBlkHist = rdsBlkHist & ~(0x1 << rdsCurBlock); // Clear history bit for the block we should have seen

        qDebug() << "ERROR RDS: wrong data";
    }

    if (RDS_BLOCK_D == rdsCurBlock)
    {
        // Block D Expected
        if ((RDS_BLOCK_0B == rdsGrpType) || (RDS_BLOCK_0A == rdsGrpType))
        {
            // Group Type 0A or 0B; Contains PS Data
            if (RDS_BLOCK_FA == rdsBlkHist)
            {
                // ABCD Present; valid for Method 1
                if (rdsSegAddrM1 == 0)
                {
                    // DI Segment is in order, and new PS Name
                    rdsPsM1 = strgRds.right(2);
                }
                else if (rdsSegAddrM1 > 0)
                {
                    // DI Segment is in order, append new chars
                    rdsPsM1 = rdsPsM1 + strgRds.right(2);
                }

                if (3 == rdsSegAddrM1)
                {
                    // Save the PSname applying Method1

                    rdsDataSet.PSname = rdsPsM1;

                    rdsSegAddrM1 = INVALID_RDS_DATA;
                    gotNewRdsInfo = true;
                }
            }
            else
            {
                // ABCD Not Present; Invalid for Method 1
                rdsPsM1.clear();
                rdsSegAddrM1 = INVALID_RDS_DATA;
            }

            if (0x0A == (rdsBlkHist & 0x0A))
            {
                // BD Present; valid for Method 2
                if (rdsSegAddrM2 == 0)
                {
                    // DI Segment is in order, and new PS Name
                    rdsPsM2 = strgRds.right(2);
                }
                else if (rdsSegAddrM2 > 0)
                {
                    // DI Segment is in order, append new chars
                    rdsPsM2 = rdsPsM2 + strgRds.right(2);
                }

                if (3 == rdsSegAddrM2)
                {
                    // Save the PSname applying Method2
                    rdsDataSet.PSname = rdsPsM2;

                    rdsSegAddrM2 = INVALID_RDS_DATA;
                    gotNewRdsInfo = true;
                }
            }
            else
            {
                // BD Not Present; Invalid for Method 2
                rdsPsM2.clear();
                rdsSegAddrM2 = INVALID_RDS_DATA;
            }
        }

        rdsBlkHist = 0;
        rdsGrpType = INVALID_RDS_DATA;
    }
}

void RdsDecoder::setRdsFmTuneFrequency(unsigned int fmRdsFreq)
{
    // Only FM EU for the moment
    fmTuneFreq = (quint8)((fmRdsFreq - 87500) / 100);
}

void RdsDecoder::UpdateBlockErrDisplay()
{
    if (totRdsBlks == 100)
    {
        if (ErrBlks.errCount > 0)
        {
            qDebug() << "Nb ERROR RDS blocks(on last 100 received): " << ErrBlks.errCount;
        }

        totRdsBlks = 0;

        ErrBlks.errCount = 0;
        ErrBlks.wrongBlockA = 0;
        ErrBlks.wrongBlockB = 0;
        ErrBlks.wrongBlockC = 0;
        ErrBlks.wrongBlockD = 0;
        ErrBlks.wrongSequence = 0;
    }
}

bool RdsDecoder::CheckValidDataRds(quint8 idx)
{
    if (((BuffIn[0] & 0x70) / 16) <= idx)
    {
        return true;
    }

    return false;
}

bool RdsDecoder::CheckValidBlocksRds(QByteArray readNotifReg)
{
#if 0
    // If RDS Buffer Overflow
    if ((readNotifReg[0] & 0x20) == 0x20)
    {
        if (true == Shared::freeFreqProg_enable)
        {
            static long overfCnt = 0;
            if (overfCnt >0)
            {
                ui->rdsOverflow_label->setHidden(false);
                overfCnt++;
            }
        }
        else
        {
            ui->rdsOverflow_label->setHidden(false);
        }
    }
#endif

    if (((readNotifReg[0] & 0xF0) == 0xD0) || ((readNotifReg[0] & 0xF0) == 0x50))
    {
        return true;
    }

    return false;
}

void RdsDecoder::Inc_BlkErrRate(rdsErrorTypes errType)
{
    switch (errType)
    {
        case RDS_ERR_BLOCK_A:
            ErrBlks.wrongBlockA++;
            break;

        case RDS_ERR_BLOCK_B:
            ErrBlks.wrongBlockB++;
            break;

        case RDS_ERR_BLOCK_C:
            ErrBlks.wrongBlockC++;
            break;

        case RDS_ERR_BLOCK_D:
            ErrBlks.wrongBlockD++;
            break;

        case RDS_ERR_WRONGSEQ:
            ErrBlks.wrongSequence++;
            break;

        default:
            // No code
            break;
    }

    ErrBlks.errCount++;

    // qDebug() << "Inc RDS Err Blk Count = " << ErrBlks.errCount;
}

void RdsDecoder::Rt_ExtSegment()
{
    Rt_Segment = BuffIn[2] & 0x1F;
}

void RdsDecoder::Rt_ExtData(quint8 block)
{
    #define FLAG_A_B     0x10

    if ((Rt_Segment & FLAG_A_B) != (Rt_Segment_Last & FLAG_A_B))
    {
        // For Group 2B we use same MASK 32 bit as for Group 2A, but filling to 1 the 16MSB
        if (true == Rt_type2B)
        {
            Rt_mask = 0xFFFF0000;
        }
        else
        {
            Rt_mask = 0x00000000;
        }

        qDebug() << "@@@@@@@@  CLEAR RT TEXT ";

        for (int i = 0; i < 64; i++)
        {
            RT_String[i] = 0x20;
            RT_CopyString[i] = 0x20;
        }
    }

    bool rtTruncate = false;

    // We decode  Radio texts respectively of 32 chars (Group 2B) or 64 chars (Group 2A)
    if (true == Rt_type2B)
    {
        // 'RT_String' is the RT displayed in radioPanel;
        // we considere valid if received two times same value among last three acquisitions
        if (((quint8)(RT_CopyString[2 * (Rt_Segment & 0x0F)]) == BuffIn[1]) &&
            ((quint8)(RT_CopyString[2 * (Rt_Segment & 0x0F) + 1]) == BuffIn[2]))
        {
            RT_String[2 * (Rt_Segment & 0x0F)] = BuffIn[1];
            RT_String[2 * (Rt_Segment & 0x0F) + 1] = BuffIn[2];
            Rt_mask |=  (0x01 << (Rt_Segment & 0x0F));
        }
        else
        {
            RT_CopyString[2 * (Rt_Segment & 0x0F)] = BuffIn[1];
            RT_CopyString[2 * (Rt_Segment & 0x0F) + 1] = BuffIn[2];
        }

        // If "endString" char (0x0D) is received two times among last three acquisitions  then fill the RT string with blank chars
        if (((Rt_mask >> (Rt_Segment & 0x0F)) & 0x01) != 0)
        {
            if ((quint8)(RT_String[2 * (Rt_Segment & 0x0F)]) == 0x0D)
            {
                RT_String[2 * (Rt_Segment & 0x0F) + 1] = 0x20;
                rtTruncate = true;
            }
            else if ((quint8)(RT_String[2 * (Rt_Segment & 0x0F) + 1]) == 0x0D)
            {
                rtTruncate = true;
            }
            if ((true == rtTruncate) && (Rt_Segment < 0x0F))
            {
                for (int i = (1 + (Rt_Segment & 0x0F)); i<16; i++)
                {
                    Rt_mask |=  (0x01 << (i));
                    RT_String[2 * i] = 0x20;
                    RT_String[2 * i + 1] = 0x20;
                }
            }
        }
    }
    else
    {
        // 'RT_String' is the RT displayed in radioPanel;
        // we considere valid if received two times same value among last three acquisitions
        if (((quint8)(RT_CopyString[4 * (Rt_Segment & 0x0F) + 2 * block]) == BuffIn[1]) &&
            ((quint8)(RT_CopyString[4 * (Rt_Segment & 0x0F) + 1 + 2 * block]) == BuffIn[2]))
        {
            RT_String[4 * (Rt_Segment & 0x0F) + 2 * block] = BuffIn[1];
            RT_String[4 * (Rt_Segment & 0x0F) + 1 + 2 * block] = BuffIn[2];
            Rt_mask |=  (0x01 << (2 * (Rt_Segment & 0x0F) + block));
        }
        else
        {
            RT_CopyString[4 * (Rt_Segment & 0x0F) + 2 * block] = BuffIn[1];
            RT_CopyString[4 * (Rt_Segment & 0x0F) + 1 + 2 * block] = BuffIn[2];
        }

        // If "endString" char (0x0D) is received two times among last three acquisitions  then fill the RT string with blank chars
        if (((Rt_mask >> (2 * (Rt_Segment & 0x0F) + block)) & 0x01)!= 0)
        {
            if ((quint8)(RT_String[4 * (Rt_Segment & 0x0F) + 2 * block]) == 0x0D)
            {
                RT_String[4 * (Rt_Segment & 0x0F) + 1 + 2 * block] = 0x20;
                rtTruncate = true;
            }
            else if ((quint8)(RT_String[4 * (Rt_Segment & 0x0F) + 1 + 2 * block]) == 0x0D)
            {
                rtTruncate = true;
            }

            if ((true == rtTruncate) && (Rt_Segment < 0x0F))
            {
                for (int i = (1 + (Rt_Segment & 0x0F)); i<16; i++)
                {
                    Rt_mask |=  (0x03 << (2 * i));
                    RT_String[4 * i] = 0x20;
                    RT_String[4 * i + 1] = 0x20;
                    RT_String[4 * i + 2] = 0x20;
                    RT_String[4 * i + 3] = 0x20;
                }
            }
        }
    }

    if (false == rdsDataSet.PIid.isEmpty())
    {
        if (Rt_mask == RT_MASK_COMPLETE)
        {
            if (RT_String.data() != rdsDataSet.rtText)
            {
                rdsDataSet.rtText = RT_String.data();
                gotNewRdsInfo = true;
                qDebug() << "@@@ " << rdsDataSet.rtText;
            }
        }
    }

    Rt_Segment_Last = Rt_Segment;
}

void RdsDecoder::ClearAllAuxData()
{
    AFHeader_ON_flg = false;
    AF_MethDetected_flg = false;
    AF_MethBOn_flg = false;

    memset(rdsDataSet.RdsAFList, AF_NO_FREQ, (RDS_NUMBER_AF + 1));

    qDebug() << "RDS AF LIST cleared";

    Curr_NbAF = 0;
}

QString RdsDecoder::SetCTData(quint32 cT_MDJ, quint32 cT_HH, quint32 cT_MM, quint32 cT_offset)
{
    QString CtString;
    int YY, Yyear, MM, Mmonth, DD, kk;

    CtString = QString::number(cT_HH) + " " + QString::number(cT_MM) + " ";

    if (cT_offset & 0x20)
    {
        CtString = CtString + "-" + QString::number((cT_offset & 0x1F) / 2);
    }
    else
    {
        CtString = CtString + "-" + QString::number(cT_offset / 2);
    }

    // MJD decode
    YY = int ((cT_MDJ - 15078.2) / 365.25);
    MM = int ((cT_MDJ - 14956.1 - int (YY * 365.25)) / 30.6001);
    DD = cT_MDJ - 14956 - int (YY * 365.25) - int (MM * 30.6001);

    if ((MM == 14) || (MM == 15))
    {
        kk = 1;
    }
    else
    {
        kk = 0;
    }

    Yyear = 1900 + YY + kk;
    Mmonth = MM - 1 - kk * 12;

    CtString = CtString + QString::number(DD, 10) + "." +
        QString::number(Mmonth, 10) + "." + QString::number(Yyear, 10);

    return CtString;
}

bool RdsDecoder::IsRdsAFEnabled()
{
    if (true == rdsAFenable)
    {
        return true;
    }

    return false;
}

void RdsDecoder::AF_MethAExt(quint8 newAFH, quint8 newAFL)
{
    AF_Insert(newAFH);

    AF_Insert(newAFL);

    AF_MethBOn_flg = false; // It is AF method A
}

void RdsDecoder::AF_MethBExt(quint8 newAFH, quint8 newAFL)
{
    AF_MethBOn_flg = true; // It is AF method B

    if (newAFH == AfFirst)
    {
        AF_Insert(newAFL);

        return;
    }

    if (newAFL == AfFirst)
    {
        AF_Insert(newAFH);

        return;
    }

    AfFirst = AF_NO_FREQ;
    AF_MethDetected_flg = false;
    AFHeader_ON_flg = false;
    AF_MethBOn_flg = false;
}

void RdsDecoder::AF_HeaderExt(quint8 newAFL)
{
    // Store only the AF list having the current Freq as Tune frequency
    if (fmTuneFreq == newAFL)
    {
        AFHeader_ON_flg = true;
        AfFirst = newAFL;
    }
}

void RdsDecoder::AF_Insert(quint8 nvalore)
{
    int i, jj;
    QString curFstr;

    for (i = 0; i < (RDS_NUMBER_AF - 1); i++)
    {
        if (rdsDataSet.RdsAFList[i] == nvalore)
        {
            return;
        }

        if (rdsDataSet.RdsAFList[i] == AF_NO_FREQ)
        {
            rdsDataSet.RdsAFList[i] = nvalore;

            if (true == afHexCode)
            {
                curFstr = (QString::number(nvalore, 16)).toUpper();
            }
            else
            {
                jj = 10 * (87.5 + 0.1 * nvalore);
                curFstr = QString::number(jj, 10);
            }

            Curr_NbAF++;

            qDebug() << "RDS: curr_NbAF: " << Curr_NbAF;

            return;
        }
    }
}

void RdsDecoder::AF_Extract(quint8 RdsAfNewH, quint8 RdsAfNewL)
{
    if (AFHeader_ON_flg == true)
    {
        if (AF_MethDetected_flg == false)
        {
            if (RdsAfNewH < 205)
            {
                AF_MethDetected_flg = true;

                if ((RdsAfNewH == AfFirst) || (RdsAfNewL == AfFirst))
                {
                    AF_MethBOn_flg = true;
                    AF_Insert(AfFirst);
                    AF_MethBExt(RdsAfNewH, RdsAfNewL);
                }
                else
                {
                    if (true == AF_MethBOn_flg)
                    {
                        return;
                    }
                }

                AF_MethBOn_flg = false;

                AF_Insert(AfFirst);

                AF_MethAExt(RdsAfNewH, RdsAfNewL);
            }

            return;
        }
        else
        {
            if (RdsAfNewH < 205)
            {
                if (true == AF_MethBOn_flg)
                {
                    AF_MethBExt(RdsAfNewH, RdsAfNewL);
                }
                else
                {
                    AF_MethAExt(RdsAfNewH, RdsAfNewL);
                }

                return;
            }
        }
    }

    if ((RdsAfNewH > 223) && (RdsAfNewH < 250))
    {
        AF_HeaderExt(RdsAfNewL);
    }
}

// End of file
