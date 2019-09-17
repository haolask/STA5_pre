#include "radiomanagerbase.h"

RadioManagerBase::RadioManagerBase()
{
    dabFreqList << 174928 << 176640 << 178352 << 180064 << 181936 \
                << 183648 << 185360 << 187072 << 188928 << 190640 \
                << 192352 << 194064 << 195936 << 197648 << 199360 \
                << 201072 << 202928 << 204640 << 206352 << 208064 \
                << 209936 << 210096 << 211648 << 213360 << 215072 \
                << 216928 << 217088 << 218640 << 220352 << 222064 \
                << 223936 << 224096 << 225648 << 227360 << 229072 \
                << 230784 << 232496 << 234208 << 235776 << 237488 \
                << 239200;

    dab3ChannelList << "5A" << "5B" << "5C" << "5D" << "6A" << "6B" << "6C" << "6D"
                    << "7A" << "7B" << "7C" << "7D" << "8A" << "8B" << "8C" << "8D"
                    << "9A" << "9B" << "9C" << "9D" << "10A" << "10N" << "10B" << "10C"
                    << "10D" << "11A" << "11N" << "11B" << "11C" << "11D" << "12A" << "12N"
                    << "12B" << "12C" << "12D" << "13A" << "13B" << "13C" << "13D" << "13E"
                    << "13F" << "0";
}

RadioManagerBase::~RadioManagerBase()
{ }

quint32 RadioManagerBase::getDab3IndexFromFrequency(int freq)
{
    int index = 0;

    if (index < dabFreqList.length() && index >= 0)
    {
        index = dabFreqList.indexOf(freq);
    }

    return index;
}

quint32 RadioManagerBase::getDab3FrequencyFromIndex(int index)
{
    int freq = 0;

    if (index < dabFreqList.length() && index >= 0)
    {
        freq = dabFreqList.at(index);
    }

    return freq;
}

QString RadioManagerBase::getDab3ChannelFromIndex(int index)
{
    QString dabChName;

    if (-1 == index)
    {
        return "NA";
    }

    if (index < dab3ChannelList.size() && index >=0)
    {
        dabChName = dab3ChannelList[index];
    }
    else
    {
        dabChName = "NA";
    }

    return dabChName;
}

quint32 RadioManagerBase::getFreqFromIndex(BandTy band, quint32 index, CountryTy country)
{
    quint32 freq;

    switch (band)
    {
        case BAND_DAB3:
            freq = getDab3FrequencyFromIndex(index);
            break;

        case BAND_AM:
            if (COUNTRY_US == country)
            {
                freq = 530 + (10 * index);
            }
            else
            {
                freq = 522 + (9 * index);
            }
            break;

        case BAND_FM:
        default:
            freq = 87500 + (100 * index);
            break;
    }

    return freq;
}

quint32 RadioManagerBase::getIndexFromFreq(BandTy band, quint32 freq)
{
    quint32 index;

    switch (band)
    {
        case BAND_DAB3:
            index = getDab3IndexFromFrequency(freq);
            break;

        case BAND_AM:
            index = (freq - 522) / 9;
            break;

        case BAND_FM:
        default:
            index = (freq - 87500) / 100;
            break;
    }

    return index;
}
