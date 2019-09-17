
#ifndef UTILITIES_H
#define UTILITIES_H

#include <QObject>
#include "common.h"

typedef enum
{
    ASCII_FORMAT,
    HEX_FORMAT,
    BINARY_FORMAT
} OutputFormat;

class Utilities : public QObject
{
    Q_OBJECT
public:
    explicit        Utilities(QObject *parent = 0);
                    ~Utilities();

    QByteArray      processTextFormat(QString &text, OutputFormat out_mode);
    QByteArray      processTextFormat(QByteArray &text, OutputFormat out_mode);
    QString         checkForZeroes(QString data, qint32 size);
    QString         insertSpaces(QString &data, qint32 every_x_chars);
    QString         formatData(quint8 data);
    QString         formatDataNoSpace(quint8 data);
    QString         formatDataDecNoSpace(quint8 data);
    QString         formatData16(quint16 data);
    QString         formatData16NoSpace(quint16 data);
    QString         formatDataDec16NoSpace(quint16 data);
    QString         formatData24(quint32 data);
    QString         formatData24NoSpace(quint32 data);
    QString         formatDataDec24NoSpace(quint32 data);
    QString         formatData32(quint32 data);
    QString         formatData32NoSpace(quint32 data);
    QString         formatDataDec32NoSpace(quint32 data);
    QString         convertDecToHex(QString &data);
    QString         convertFloatIntoHex(QString &data);
    QStringList     splitMultipleCommandsLine(QString &data);
    QString         bytesToSring(QByteArray &data);
    bool            checkForMultipleCommandsLine(QString &data);
    QString         getNextStringFromCurrentPosition(quint8 starting_position, QString &str);
    QString         removeUselessSpaces(QString &str, bool trim_starting_ending_spaces_flag);
    QString         splitDatain2charsCells(QString &str, bool trim_starting_ending_spaces_flag);
    quint16         revertData16(quint16);
    quint32         revertData32(quint32);

    QString         revertDataString(QString &data);
    QString         Calculate_Decimal(quint8, QString );

    double          ConvHex4Float (quint8, quint8, quint8, quint8);
    double          ConvHex3Float (quint8, quint8, quint8);
    qint32          ConvFloat2Hex (float datof);
    qint32          ConvFloat2_32bitHex (double datof);
    QString         computeComplementChar (quint8);

    QString         recode_ext_ascii_chars(QString inp_string, quint8 charset_type);
    QString         getQStringfromQByte(QByteArray data_in );
    quint8          revertBitsInByte(quint8 data_in);


};

#endif // UTILITIES_H
