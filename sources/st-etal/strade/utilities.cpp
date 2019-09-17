/****************************************************************************
**
** Copyright (C) 2013 STMicroelectronics. All rights reserved.
**
** This file is part of DAB Test Application
**
** Author: Marco Barboni (marco.barboni@st.com)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation or under the
** terms of the Qt Commercial License Agreement. The respective license
** texts for these are provided with the open source and commercial
** editions of Qt.
**
****************************************************************************/
#include "utilities.h"
//#include "cmd_mngr_base.h"

Utilities::Utilities(QObject *parent) : QObject(parent)
{ }

Utilities::~Utilities()
{ }

QString Utilities::Calculate_Decimal(quint8 nBytes, QString InValore)
{
    quint8 i;
    QString tppp, retstr;

    tppp = convertDecToHex(InValore);

    while (tppp.length() < (2 * nBytes))
    {
        tppp = "0" + tppp;
    }

    if (tppp.length() > (2 * nBytes))
    {
        tppp = tppp.right(2 * nBytes);
    }

    retstr = "";

    for (i = 1;  i < (tppp.length() - 1); i += 2)
    {
        retstr = retstr + " " + tppp.mid(i, 2);
    }

    return retstr;
}

QByteArray Utilities::processTextFormat(QString&text, OutputFormat out_format)
{
    QString commandTextEdit_tmp;
    // QString tempstring, tmp2string;

    //    //to manage decimal notation composed by X#val (X=nb bytes val=value)
    //    quint8 i,ii,ij;
    //    bool ok;
    //    while (text.indexOf( "#",0 ) > 0 )
    //    {
    //        i= text.indexOf( "#",0 );
    //        tempstring = text.at(i - 1);
    //        ii=tempstring.toInt(&ok,10);
    //        tempstring = text.right( text.length()- i);
    //        if (tempstring.indexOf(" ",0) > 0)
    //            tmp2string = tempstring.left(tempstring.indexOf(" ",0));
    //        else
    //            tmp2string = tempstring;
    //        ij=tmp2string.length();
    //        tmp2string = Calculate_Decimal(ii, tmp2string);
    //        text = text.left(i - 3) + tmp2string + tempstring.right( tempstring.length() - ij);
    //    }

    // Remove any ending comment on the string
    quint32 i;

    i = text.indexOf("//", 2);

    if (i > 0)
    {
        text = text.left(i - 1);
    }

    if (out_format == ASCII_FORMAT)
    {
        commandTextEdit_tmp = text;
    }
    else if (out_format == HEX_FORMAT)
    {
        QByteArray dec_values = text.toUtf8();
        QString temp_string;
        int i = 0, len = dec_values.length();

        while (i < len)
        {
            int value = (int)dec_values[i++];

            if (value != 10)
            {
                temp_string.setNum(value, 16);
            }
            else
            {
                temp_string = '\n';
            }

            commandTextEdit_tmp.append(temp_string.toUpper());
        }
    }
    else if (out_format == BINARY_FORMAT)
    {
        QByteArray dec_values = text.toUtf8();
        QString temp_string;
        int i = 0, len = dec_values.length();

        while (i < len)
        {
            int value = (int)dec_values[i++];

            if (value != 10)
            {
                temp_string.setNum(value, 2);

                // Append starting zeroes if needed
                QString tmp;
                tmp.fill(QChar('0'), (8 - temp_string.length())).append(temp_string);
                temp_string = tmp;
            }
            else
            {
                temp_string = '\n';
            }

            commandTextEdit_tmp.append(temp_string);

            if (i < len)
            {
                commandTextEdit_tmp.append(SEPARATOR_STRING);
            }
        }
    }

    return commandTextEdit_tmp.toUtf8();
}

QByteArray Utilities::processTextFormat(QByteArray&data, OutputFormat out_format)
{
    QByteArray commandTextEdit_tmp;

    if (out_format == ASCII_FORMAT)
    {
        commandTextEdit_tmp.resize(data.size());

        for (int k = 0; k < data.length(); k++)
        {
            // If a character is a non printable character
            // then replace it with a SPACE character
            if ((char)data[k] < 20)
            {
                commandTextEdit_tmp[k] = (char)20;
            }
            else
            {
                commandTextEdit_tmp[k] = (char)data[k];
            }
        }
    }
    else if (out_format == HEX_FORMAT)
    {
        QByteArray dec_values = data;
        QString temp_string;
        int i = 0, len = dec_values.length();

        while (i < len)
        {
            int value = (unsigned char)dec_values[i++];

            temp_string.setNum(value, 16);
            QString tmp;
            tmp.fill(QChar('0'), (2 - temp_string.length())).append(temp_string);
            temp_string = tmp;

            commandTextEdit_tmp.append(temp_string.toUpper());
        }
    }
    else if (out_format == BINARY_FORMAT)
    {
        QByteArray dec_values = data;
        QString temp_string;
        int i = 0, len = dec_values.length();

        while (i < len)
        {
            int value = (int)dec_values[i++];

            if (value != 10)
            {
                temp_string.setNum(value, 2);

                // Append starting zeroes if needed
                QString tmp;
                tmp.fill(QChar('0'), (8 - temp_string.length())).append(temp_string);
                temp_string = tmp;
            }
            else
            {
                temp_string = '\n';
            }

            commandTextEdit_tmp.append(temp_string);

            if (i < len)
            {
                commandTextEdit_tmp.append(SEPARATOR_STRING);
            }
        }
    }

    return commandTextEdit_tmp;
}

QString Utilities::checkForZeroes(QString data, qint32 size)
{
    qint32 data_size = data.size();

    if (data_size <= 0)
    {
        return data;
    }

    if ((data_size <= size) && (data_size > 0))
    {
        QString data_tmp;

        // Added needed leading zeroes
        for (int i = 0; i < size - data_size; i++)
        {
            data_tmp.append('0');
        }

        data_tmp.append(data);

        return data_tmp;
    }
    else // (data_size > size)
    {
        // Trunc data to 'size'
        if ((data.left(1)) == "F" || (data.left(1)) == "f")
        {
            // Negative number
            return data.right(size);
        }
        else
        {
            // Positive number
            return data.left(size);
        }
    }
}

QString Utilities::insertSpaces(QString&data, qint32 every_x_chars)
{
    QString data_tmp;
    qint32  data_size = data.size();

    if ((data_size > every_x_chars) && (data_size > 0))
    {
        while (data_size >= every_x_chars)
        {
            data_tmp = data.right(every_x_chars) + SEPARATOR_STRING + data_tmp;
            data.remove(data.length() - every_x_chars, every_x_chars);
            data_size -= every_x_chars;
        }

        data_tmp = data_tmp.trimmed();

        return data_tmp;
    }
    else
    {
        return data;
    }
}

QString Utilities::formatData(quint8 data)
{
    QString byte;

    byte.sprintf("%02X", data);

    return SEPARATOR_STRING + byte;
}

QString Utilities::formatDataNoSpace(quint8 data)
{
    QString byte;

    byte.sprintf("%02X", data);

    return byte;
}

QString Utilities::formatDataDecNoSpace(quint8 data)
{
    QString byte;

    byte.sprintf("%02d", data);

    return byte;
}

QString Utilities::formatData16(quint16 data)
{
    QString half_word;

    half_word.sprintf("%04X", data);

    return SEPARATOR_STRING + half_word;
}

QString Utilities::formatData16NoSpace(quint16 data)
{
    QString half_word;

    half_word.sprintf("%04X", data);

    return half_word;
}

QString Utilities::formatDataDec16NoSpace(quint16 data)
{
    QString half_word;

    half_word.sprintf("%04d", data);

    return half_word;
}

QString Utilities::formatData24(quint32 data)
{
    QString word;

    word.sprintf("%06X", data & 0xFFFFFF);

    return SEPARATOR_STRING + word;
}

QString Utilities::formatData24NoSpace(quint32 data)
{
    QString word;

    word.sprintf("%06X", data & 0xFFFFFF);

    return word;
}

QString Utilities::formatDataDec24NoSpace(quint32 data)
{
    QString word;

    word.sprintf("%06d", data & 0xFFFFFF);

    return word;
}

QString Utilities::formatData32(quint32 data)
{
    QString word;

    word.sprintf("%08X", data);

    return SEPARATOR_STRING + word;
}

QString Utilities::formatData32NoSpace(quint32 data)
{
    QString word;

    word.sprintf("%08X", data);

    return word;
}

QString Utilities::formatDataDec32NoSpace(quint32 data)
{
    QString word;

    word.sprintf("%08d", data);

    return word;
}

quint16 Utilities::revertData16(quint16 data)
{
    quint8 data_msb = (data >> 8) & 0xFF;
    quint8 data_lsb = data & 0xFF;

    return (quint16)((data_lsb << 8) | (data_msb));
}

quint32 Utilities::revertData32(quint32 data)
{
    quint8 data_msb = (data >> 24) & 0xFF;
    quint8 data_1   = (data >> 16) & 0xFF;
    quint8 data_2   = (data >> 8) & 0xFF;
    quint8 data_lsb = data & 0xFF;

    return (quint32)((data_lsb << 24) |
                     (data_2 << 16)   |
                     (data_1 <<  8)   |
                     (data_msb));
}

QString Utilities::revertDataString(QString&data)
{
    QString result_string;
    quint32 size = data.size();

    // Check if size is greater than zero and
    // the number of characters are even
    if ((size > 0) && (((size / 2) % 2) == 0))
    {
        for (int i = size - 1; i >= 0; i -= 2)
        {
            result_string.append(QString(data.at(i - 1)));
            result_string.append(QString(data.at(i)));
        }
    }

    return result_string;
}

QString Utilities::bytesToSring(QByteArray&arcArray)
{
    QString myQString;
    QTextStream myStream(&myQString);

    myStream << hex << qSetPadChar('0') << qSetFieldWidth(2) << right;

    for (int i = 0; i < arcArray.size(); i += 2)
    {
        myStream << static_cast <unsigned char> (arcArray.at(i));
        myStream << static_cast <unsigned char> (arcArray.at(i + 1));
    }

    return myQString.toUpper();
}

QString Utilities::convertDecToHex(QString&data)
{
    QStringList data_list = data.toUpper().split(" ", QString::SkipEmptyParts);
    QString data_result;
    int pos, i, decimal_number, bytes_num;
    bool ok;

    for (i = 0; i < data_list.count(); i++)
    {
        pos = data_list[i].indexOf("#");

        if (pos > 0)
        {
            bytes_num = data_list[i].mid(0, pos).toUInt(&ok);

            if (bytes_num > 4)
            {
                bytes_num = 4;
            }

            decimal_number = data_list[i].mid(pos + 1, -1).toInt(&ok);
            data_list[i] = QString::number(decimal_number, 16);

            if (true == ok)
            {
                data_list[i] = checkForZeroes(data_list[i], bytes_num * 2);
            }
        }
    }

    data_result = data_list.join(SEPARATOR_STRING).toUpper();

    return data_result;
}

QString Utilities::convertFloatIntoHex(QString&data)
{
    QStringList data_list = data.split(" ", QString::SkipEmptyParts);
    QString data_result;
    float float_number;
    int pos, i, bytes_num;
    bool ok;

    for (i = 0; i < data_list.count(); i++)
    {
        pos = data_list[i].indexOf("%");

        if (pos >= 0)
        {
            // Computation allowed only as 24 bit(bytes_num=3) or 32 bit(bytes_num=4)
            if (pos > 0)
            {
                bytes_num = data_list[i].mid(0, pos).toUInt(&ok);
            }
            else
            {
                bytes_num = 3;
            }

            if (bytes_num > 4)
            {
                bytes_num = 4;
            }
            else if (bytes_num < 3)
            {
                bytes_num = 3;
            }

            float_number = (data_list[i].mid(pos + 1, -1)).toDouble(&ok);

            data_list[i] = QString::number(ConvFloat2Hex(float_number), 16);

            if (true == ok)
            {
                data_list[i] = checkForZeroes(data_list[i], bytes_num * 2);
            }
        }
    }

    data_result = data_list.join(SEPARATOR_STRING);

    return data_result;
}

QStringList Utilities::splitMultipleCommandsLine(QString&data)
{
    QString data_tmp = QString(data);
    QStringList command_list = QStringList(data_tmp);
    int current_pos = 0;
    int pos = 0, index = 0;
    QString tmp;

    // If the command line contains multiple commands
    // then clear the default return value
    if (data_tmp.indexOf("|") > 0)
    {
        command_list.clear();
    }
    else
    {
        return command_list;
    }

    while (pos != -1)
    {
        pos = data_tmp.indexOf("|");

        if (pos == -1)
        {
            command_list.append(data_tmp.mid(current_pos, -1).trimmed().toUpper());
        }
        else
        {
            command_list.append(data_tmp.mid(current_pos, pos - current_pos).trimmed().toUpper());
        }

        data_tmp = data_tmp.mid(pos + 1, -1).trimmed();

        current_pos = 0;

        index++;
    }

    return command_list;
}

bool Utilities::checkForMultipleCommandsLine(QString&data)
{
    bool result = false;
    int pos = data.indexOf("|");

    if ((pos > 0) && (data.mid(pos + 1, -1).length() > 0))
    {
        result = true;
    }

    return result;
}

QString Utilities::getNextStringFromCurrentPosition(quint8 starting_position, QString&str)
{
    quint16 space_position;
    QString return_value;

    space_position = str.indexOf(SPACE_STRING, starting_position);
    return_value = str.mid(starting_position, space_position);

    return return_value;
}

QString Utilities::removeUselessSpaces(QString&str, bool trim_starting_ending_spaces_flag)
{
    int i;
    QString return_value;
    QString previous_value, current_value;

    if (str.length() == 0 || str.length() == 1)
    {
        return_value = str;

        return return_value;
    }

    previous_value = str.mid(0, 1);

    for (i = 0; i < str.length(); i++)
    {
        current_value = str.mid(i, 1);

        if ((previous_value == SPACE_STRING) && (current_value == SPACE_STRING))
        {
            continue;
        }

        previous_value = current_value;

        return_value.append(current_value);
    }

    if (trim_starting_ending_spaces_flag)
    {
        return_value = return_value.trimmed();
    }

    return return_value;
}

QString Utilities::splitDatain2charsCells(QString&str, bool trim_starting_ending_spaces_flag)
{
    int i, countchars;
    QString return_value, rxxx;
    QString previous_value, pvalue1, current_value;

    if (str.length() == 0 || str.length() == 1)
    {
        return_value = str;

        return return_value;
    }

    rxxx = "";
    previous_value = str.mid(0, 1);
    countchars = 1;

    for (i = 1; i < str.length(); i++)
    {
        current_value = str.mid(i, 1);
        countchars++;

        if (current_value == SPACE_STRING)
        {
            if (countchars == 2)
            {
                countchars = 0;

                if (previous_value == SPACE_STRING)
                {
                    continue;
                }
                else
                {
                    return_value.append("0");
                    rxxx = rxxx + "0";
                }

                return_value.append(previous_value);
                rxxx = rxxx + previous_value;
            }

            if (countchars == 3)
            {
                countchars = 0;

                if (pvalue1 == SPACE_STRING)
                {
                    continue;
                }
                else
                {
                    return_value.append(pvalue1);
                    rxxx = rxxx + pvalue1;
                }

                return_value.append(previous_value);
                return_value.append(SPACE_STRING);
                rxxx = rxxx + previous_value + " ";
            }
        }
        else
        {
            if (countchars == 2)
            {
                pvalue1 = previous_value;
            }

            if (countchars == 3)
            {
                countchars = 1;

                if (previous_value == SPACE_STRING)
                {
                    return_value.append("0");

                    return_value.append(current_value);
                    rxxx = rxxx + "0" + current_value;
                }
                else
                {
                    return_value.append(pvalue1);
                    return_value.append(previous_value);
                    return_value.append(SPACE_STRING);
                    rxxx = rxxx + pvalue1 + previous_value + " ";
                }
            }
        }

        previous_value = current_value;
    }

    if (countchars == 1)
    {
        return_value.append("0");
        return_value.append(previous_value);
        rxxx = rxxx + "0" + previous_value;
    }

    if (countchars == 2)
    {
        return_value.append(pvalue1);
        return_value.append(previous_value);
        rxxx = rxxx + pvalue1 + previous_value;
    }

    if (trim_starting_ending_spaces_flag)
    {
        return_value = return_value.trimmed();
    }

    return return_value;
}

double Utilities::ConvHex4Float(quint8 dato1, quint8 dato2, quint8 dato3, quint8 dato4)
{
    double  fixed;
    qint32 value;
    qint32 sign_mask, signtest;

    value = (256 * 256 * 256 * dato1) + (256 * 256 * dato2) + (256 * dato3) + dato4;
    sign_mask = 0x80000000;
    signtest  = sign_mask & value;

    if (signtest == 0)
    {
        fixed = value / 2147483648.0;
    }
    else
    {
        fixed = value / 2147483648.0;  // - 2.0;
    }

    return fixed;
}

double Utilities::ConvHex3Float(quint8 dato1, quint8 dato2, quint8 dato3)
{
    double  fixed;
    long int value;
    long int sign_mask, signtest;

    value = (256 * 256 * dato1) + (256 * dato2) + dato3;
    sign_mask = 0x800000;
    signtest  = sign_mask & value;

    if (signtest == 0)
    {
        fixed = value / 8388608.0;
    }
    else
    {
        fixed = value / 8388608.0 - 2.0;
    }

    return fixed;
}

qint32 Utilities::ConvFloat2Hex(float datof)
{
    double  tmpp;

    tmpp = (datof * 0x800000);

    if (tmpp > 0x7FFFFF)
    {
        tmpp = 0x7FFFFF;
    }

    if (tmpp < -0x800000)
    {
        tmpp = -0x800000;
    }

    return (qint32)(tmpp) & 0x00FFFFFF;
}

QString Utilities::computeComplementChar(quint8 tmmi)
{
    QString tmp2;

    switch (tmmi)
    {
        case 15:
        {
            tmp2 = "0";
        }
        break;

        case 14:
        {
            tmp2 = "1";
        }
        break;

        case 13:
        {
            tmp2 = "2";
        }
        break;

        case 12:
        {
            tmp2 = "3";
        }
        break;

        case 11:
        {
            tmp2 = "4";
        }
        break;

        case 10:
        {
            tmp2 = "5";
        }
        break;

        case 9:
        {
            tmp2 = "6";
        }
        break;

        case 8:
        {
            tmp2 = "7";
        }
        break;

        case 7:
        {
            tmp2 = "8";
        }
        break;

        case 6:
        {
            tmp2 = "9";
        }
        break;

        case 5:
        {
            tmp2 = "A";
        }
        break;

        case 4:
        {
            tmp2 = "B";
        }
        break;

        case 3:
        {
            tmp2 = "C";
        }
        break;

        case 2:
        {
            tmp2 = "D";
        }
        break;

        case 1:
        {
            tmp2 = "E";
        }
        break;

        case 0:
        {
            tmp2 = "F";
        }
        break;

        default:
        {
            // No code
        }
        break;
    }

    return tmp2;
}

qint32 Utilities::ConvFloat2_32bitHex(double datof)
{
    double tmpp;
    double tmpx;

    tmpp = datof;

    if ((tmpp < 1) && (tmpp > 0))
    {
        tmpp = tmpp * 0x80000000;

        if (tmpp > 2147483647.99996)
        {
            tmpp = 0x7FFFFFFF;
        }
    }
    else
    {
        if ((tmpp > -1) && (tmpp < 0))
        {
            tmpx = tmpp * 0x80000000 * (-1);

            if (tmpx > 2147483647.99996)
            {
                tmpp = 0x80000000;
            }
            else
            {
                tmpp = -tmpx;
            }
        }
        else
        {
            if (tmpp == 0)
            {
                tmpp = 0x00000000;
            }
            else
            {
                if (tmpp == 1)
                {
                    tmpp = 0x7FFFFFFF;
                }
                else
                {
                    if (tmpp == -1)
                    {
                        tmpp = 0x80000000;
                    }
                }
            }
        }
    }

    return (qint32)(tmpp) & 0xFFFFFFFF;
}

QString Utilities::recode_ext_ascii_chars(QString inp_string, quint8 charset_type)
{
    quint16 i;
    uint  jj;
    bool ok;
    QString out_string;

    out_string.clear();

#if 0
    static uint CharSetEbuLatin1_Ty [] =
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
        0x20, 0x21, 0x22, 0x23, 0xA4, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,  //5
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,  //7
        0xE1, 0xE0, 0xE9, 0xE8, 0xED, 0xEC, 0xF3, 0xF2, 0xFA, 0xF9, 0xD1, 0xC7, 0x15E, 0xDF, 0xA1, 0x132,  //8
        0xE2, 0xE4, 0xEA, 0xEB, 0xEE, 0xEF, 0xF4, 0xF6, 0xFB, 0xFC, 0xF1, 0xE7, 0x15F, 0x11F, 0x3F, 0x133,  // 3F = ? means char not found in Unicode
        0xAA, 0x3B1, 0xA9, 0x2030, 0x11E, 0x11B, 0x148, 0x151, 0x3C0, 0x3F, 0xA3, 0x24, 0x2190, 0x2191, 0x2192, 0x2193,  //a
        0xBA, 0xB9, 0xB2, 0xB3, 0xB1, 0x130, 0x144, 0x171, 0x3BC, 0xBF, 0xF7, 0xB0, 0xBC, 0xBD, 0xBE, 0xA7,
        0xC1, 0xC0, 0xC9, 0xC8, 0xCD, 0xCC, 0xD3, 0xD2, 0xDA, 0xD9, 0x158, 0x10C, 0x160, 0x17D, 0x110, 0x3F,  //c
        0xC2, 0xC4, 0xCA, 0xCB, 0xCE, 0xCF, 0xD4, 0xD6, 0xDB, 0xDC, 0x159, 0x10D, 0x161, 0x17E, 0x111, 0x3F,
        0xC3, 0xC5, 0xC6, 0x152, 0x177, 0xDD, 0xD5, 0xD8, 0xDE, 0x3F, 0x154, 0x106, 0x15A, 0x179, 0x2213, 0x3F,  //e
        0xE3, 0xE5, 0xE6, 0x153, 0x175, 0xFD, 0xF5, 0xF8, 0xFE, 0x3F, 0x155, 0x107, 0x15B, 0x17A, 0x3F, 0x3F
    };
#endif

    static const unsigned short ebuLatinToUcs2 [] =
    {
        /* 0x00 - 0x07 */ 0x00,   0x0118, 0x012e, 0x0172, 0x0102, 0x0116, 0x010e, 0x0218,
        /* 0x08 - 0x0f */ 0x021a, 0x010a, 0x0a,   0x0b,   0x0120, 0x0139, 0x017b, 0x0143,
        /* 0x10 - 0x17 */ 0x0105, 0x0119, 0x012f, 0x0173, 0x0103, 0x0117, 0x010f, 0x0219,
        /* 0x18 - 0x1f */ 0x021b, 0x010b, 0x0147, 0x011a, 0x0121, 0x013a, 0x017c, 0x1f,

        /* 0x20 - 0x27 */ 0x20,   0x21,   0x22,   0x23,   0x0142, 0x25,   0x26,   0x27,
        /* 0x28 - 0x2f */ 0x28,   0x29,   0x2a,   0x2b,   0x2c,   0x2d,   0x2e,   0x2f,
        /* 0x30 - 0x37 */ 0x30,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,
        /* 0x38 - 0x3f */ 0x38,   0x39,   0x3a,   0x3b,   0x3c,   0x3d,   0x3e,   0x3f,
        /* 0x40 - 0x47 */ 0x40,   0x41,   0x42,   0x43,   0x44,   0x45,   0x46,   0x47,
        /* 0x48 - 0x4f */ 0x48,   0x49,   0x4a,   0x4b,   0x4c,   0x4d,   0x4e,   0x4f,
        /* 0x50 - 0x57 */ 0x50,   0x51,   0x52,   0x53,   0x54,   0x55,   0x56,   0x57,
        /* 0x58 - 0x5f */ 0x58,   0x59,   0x5a,   0x5b,   0x016e, 0x5d,   0x0141, 0x5f,
        /* 0x60 - 0x67 */ 0x0104, 0x61,   0x62,   0x63,   0x64,   0x65,   0x66,   0x67,
        /* 0x68 - 0x6f */ 0x68,   0x69,   0x6a,   0x6b,   0x6c,   0x6d,   0x6e,   0x6f,
        /* 0x70 - 0x77 */ 0x70,   0x71,   0x72,   0x73,   0x74,   0x75,   0x76,   0x77,
        /* 0x78 - 0x7f */ 0x78,   0x79,   0x7a,   0x00ab, 0x016f, 0x00bb, 0x013d, 0x0126,

        /* 0x80 - 0x87 */ 0xe1,   0xe0,   0xe9,   0xe8,   0xed,   0xec,   0xf3,   0xf2,
        /* 0x88 - 0x8f */ 0xfa,   0xf9,   0xd1,   0xc7,   0x015e, 0xdf,   0xa1,   0x0132,
        /* 0x90 - 0x97 */ 0xe2,   0xe4,   0xea,   0xeb,   0xee,   0xef,   0xf4,   0xf6,
        /* 0x98 - 0x9f */ 0xfb,   0xfc,   0xf1,   0xe7,   0x015f, 0x011f, 0x0131, 0x0133,
        /* 0xa0 - 0xa7 */ 0xaa,   0x03b1, 0xa9,   0x2030, 0x011e, 0x011b, 0x0148, 0x0151,
        /* 0xa8 - 0xaf */ 0x03c0, 0x20ac, 0xa3,   0x24,   0x2190, 0x2191, 0x2192, 0x2193,
        /* 0xb0 - 0xb7 */ 0xba,   0xb9,   0xb2,   0xb3,   0xb1,   0x0130, 0x0144, 0x0171,
        /* 0xb8 - 0xbf */ 0xb5,   0xbf,   0xf7,   0xb0,   0xbc,   0xbd,   0xbe,   0xa7,
        /* 0xc0 - 0xc7 */ 0xc1,   0xc0,   0xc9,   0xc8,   0xcd,   0xcc,   0xd3,   0xd2,
        /* 0xc8 - 0xcf */ 0xda,   0xd9,   0x0158, 0x010c, 0x0160, 0x017d, 0xd0,   0x13f,
        /* 0xd0 - 0xd7 */ 0xc2,   0xc4,   0xca,   0xcb,   0xce,   0xcf,   0xd4,   0xd6,
        /* 0xd8 - 0xdf */ 0xdb,   0xdc,   0x0159, 0x010d, 0x0161, 0x017e, 0x0111, 0x0140,
        /* 0xe0 - 0xe7 */ 0x00c3, 0x00c5, 0x00c6, 0x0152, 0x0177, 0xdd,   0xd5,   0xd8,
        /* 0xe8 - 0xef */ 0xde,   0x014a, 0x0154, 0x0106, 0x015a, 0x0179, 0x0166, 0xf0,
        /* 0xf0 - 0xf7 */ 0xe3,   0xe5,   0xe6,   0x0153, 0x0175, 0xfd,   0xf5,   0xf8,
        /* 0xf8 - 0xff */ 0xfe,   0x014b, 0x0155, 0x0107, 0x015b, 0x017a, 0x0167, 0xff
    };

    switch (charset_type)
    {
        case 0x00: // Complete EBU Latin based repertoire
        {
            for (i = 0; i < (inp_string.length()); i++)
            {
                jj = inp_string.mid(2 * i, 2).toUpper().toUInt(&ok, 16);
                out_string += QChar(ebuLatinToUcs2[jj]);
            }
#if 0
            s = QString();
            for (i = 0; i < length; i++)
            {
                s [i] = QChar(ebuLatinToUcs2 [((uint8_t *)buffer)[i]]);
            }
 #endif
        }
        break;

        case 0x06: // ISO Latin Alphabet No. 1 (see ISO/IEC 8859-1 [17]) (0x04 from specs, to be verified)
        {
            for (i = 0; i < (inp_string.length() - 1); i++)
            {
                out_string = out_string + inp_string.mid(i, 1);
            }
        }
        break;

        case 0x0F: // ISO/IEC 10646 [15] using UTF-8 transformation format
        {
            out_string = inp_string;

            qDebug() << "************************************ charset F ";
        }
        break;

        default:
        {
            // No code
        }
        break;
    }

    return out_string;
}

QString Utilities::getQStringfromQByte(QByteArray data_in)
{
    QString tttm;
    quint8 value;
    int j;

    tttm = "";

    for (j = 0; j < data_in.length(); j++)
    {
        value = (quint8)data_in[j];

        if (value == 0)
        {
            tttm.append("00");
        }
        else
        {
            tttm.append(QString(data_in.mid(j, 1)).toLatin1().toHex().toUpper());
        }
    }

    return tttm;
}

quint8 Utilities::revertBitsInByte(quint8 data_in)
{
    quint8 value = 0;
    quint8 i;

    for (i = 0; i < 8; i++)
    {
        if (data_in & (0x01 << (7 - i)))
        {
            value |= (0x01 << i);
        }
    }

    return value;
}
