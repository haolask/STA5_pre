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
#include <QMessageBox>

#include "filemanager.h"

FilesManager::FilesManager(QString _instancerName)
{
    Q_UNUSED(_instancerName);
}

FilesManager::~FilesManager()
{

}

QStringList FilesManager::LoadFile(QString fullFileName)
{
    QStringList  linesList;

    QFile file(fullFileName);

    if (!file.exists())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("LoadFile Methode: Message Error");
        msgBox.setText("Configuration file Not Found Error!");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);

        file.close();

        return linesList;
    }
    else
    {
        //open existing file
        if(file.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            //read all file
            QByteArray  DataBuffer;
            DataBuffer = file.readAll();

            //split file into lines
            QString t(DataBuffer);
            linesList = t.split("\n");

            //close file
            file.close();
        }

        //clear the blank and void lines
        int i=0;
        while (i<linesList.size())
        {
            QString lineStr = linesList.at(i);

            if (lineStr.isEmpty())//empty() and null("")
            {
                linesList.removeAt(i);
            }

            i++;
        }

        i=0;
        while (i<linesList.size())
        {
            QString lineStr = linesList.at(i);
            if(lineStr.isEmpty())//empty() and null("")
            {
                linesList.removeAt(i);
            }

            i++;
        }

        if (file.size() == 0)
        {
            file.close();
        }
    }

    return linesList;
}

void FilesManager::SaveFile(QString fullFileName, QStringList linesList)
{
    // Write on a new config.ini file with the fileLineList data
    QFile file(fullFileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        // Write file
        QString s(linesList.join("/n"));
        file.write(s.toLocal8Bit());

        // Close file
        file.close();
    }
    else
    {
        // Opens file errors
    }
}

void FilesManager::SaveFile(QString fullFileName, QString str)
{
    // Write on a new config.ini file with the fileLineList data
    QFile file(fullFileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        // Write file
        file.write(str.toLocal8Bit());

        // Close file
        file.close();
    }
    else
    {
        // Ppens file errors
    }
}

void FilesManager::TokenizeFile(QString separatorStr, QStringList linesList)
{
    QStringList tokens; // Parse cycle
    QString lineStr;    // Line string
    int lineIndex = 0;  // Line counter

    while (lineIndex <= linesList.size() - 1)
    {
        //get new line to parse
        lineStr = linesList.at(lineIndex );

        //clear spaces
        lineStr = lineStr.simplified();//remove ASCII characters '\t', '\n', '\v', '\f', '\r', and ' '.

        //check current line
        if (lineStr.startsWith(" "))//space
        {
            lineIndex++;
        }
        else if (lineStr.startsWith("\n"))//end of line
        {
            lineIndex++;
        }
        else if (lineStr.startsWith("#"))//comment
        {
            lineIndex++;
        }
        else if (lineStr.isEmpty())//empty() and null("")
        {
            lineIndex++;
        }
        else
        {
            // Tokenize current line
            QString dataLine(lineStr);
            tokens = dataLine.split(separatorStr, QString::SkipEmptyParts);

            if(tokens.size() == 1)//
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle("TokenizeFile Methode: Message Error");
                msgBox.setText("Configuration file sintax Error!");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                //if(msgBox.exec() == QMessageBox::Yes){
                // do something
                // }
                // else
                //{
                // do something else
                //}

                return;
            }

            //extract key from line
            QString key = tokens.at(0);
            key = key.simplified();//remove ASCII characters '\t', '\n', '\v', '\f', '\r', and ' '.

            //extract value from line
            QString value = tokens.at(1);
            value = value.simplified();//remove ASCII characters '\t', '\n', '\v', '\f', '\r', and ' '.

            //fill chunk
            ChunkTy chunk;
            chunk.key.append(key);
            chunk.value.append(value);

            //save chunk in global ChunkList
            ChunkList.insert(lineIndex,chunk);

            //qDebug() << "--TokenizeFile: line "<< lineIndex << " ChunkList.size()= " << ChunkList.size();
            //qDebug() << "---ChunkList.key = "<< ChunkList.at(lineIndex).key;
            //qDebug() << "---ChunkList.value = "<< ChunkList.at(lineIndex).value;

            lineIndex++;
        }
    }
}

void FilesManager::SetKeyValue(QString key, QString newValue)
{
     int i = 0;

     while (i < ChunkList.size())
     {
         ChunkTy chunk = ChunkList.at(i);

         if (chunk.key == key)
         {
             chunk.value = newValue;
             ChunkList.replace(i,chunk);
             return;
         }

        i++;
     }
}

QString FilesManager::GetKeyValue(QString key)
{
     int i = 0;

     while (i < ChunkList.size())
     {
         ChunkTy chunk = ChunkList.at(i);

         if (chunk.key == key)
         {
             return chunk.value;
         }

        i++;
     }

     return QString("0");
}

QStringList FilesManager::ChunkToStringList()
{
    QStringList strList;
    int i = 0;

    while (i < ChunkList.size())
    {
        ChunkTy chunk;
        chunk = ChunkList.at(i);
        strList.append(chunk.key + "=" + chunk.value + "\n");
        i++;
    }

    return strList;
}

QString FilesManager::ChunkToString()
{
    QString str;
    int i = 0;

    while (i < ChunkList.size())
    {
        ChunkTy chunk;
        chunk = ChunkList.at(i);
        str.append(chunk.key + "=" + chunk.value + "\n");
        i++;
    }

    return str;
}

void FilesManager::ex_1(QString testDescription)
{
    Q_UNUSED(testDescription)
    //load setting file end return the list of all your lines
    QStringList linesList = LoadFile("fullFileName");

    //tokenize line by line using "=" separator. The couple <key, value> is named chunk.
    //All the couple are saved in the global data list named ChunkList
    TokenizeFile("=", linesList);

    //search the key_name(ex. my_key1, my_key2) and change the corresponding key_value
    SetKeyValue("my_key1", "new_value1");
    SetKeyValue("my_key2", "new_value2");
    SetKeyValue("my_key3", "new_value3");

    //get all the chunck into the ChunkList and create a unique string
    //save the string into file
    SaveFile("fullFileName", ChunkToString());
}

void FilesManager::Test()
{
    ex_1("Change three key value on the settings file");
}

QFile::FileError FilesManager::open(QString &filename, QFile::OpenMode mode, bool must_exist)
{
    QFile::FileError error = QFile::NoError;

    current_line = 0;

    bool file_exist = QFile().exists(filename);

    if (!file_exist && must_exist)
    {
        error = QFile::OpenError;
    }
    else
    {
        fp = new QFile(filename);

        if (!fp->open(mode))
        {
            error = QFile::OpenError;
        }
    }

    return error;
}

void FilesManager::close(void)
{
    if (fp->handle() != -1)
    {
        fp->close();
    }

    fp = 0;
}

QFile::FileError FilesManager::seek(quint32 position)
{
    if (!fp->isOpen())
    {
        return QFile::OpenError;
    }

    if (position == 0)
        current_line = 0;

    fp->seek((int)position);
    return QFile::NoError;
}

QFile::FileError FilesManager::seekLine(quint32 line)
{
    QFile::FileError error = QFile::NoError;
    qint64 read_bytes = 0;
    quint32 i = 0;

    if (!fp->isOpen())
    {
        error = QFile::OpenError;

        return error;
    }

    fp->seek(0);

    char temp[255];

    while ((i++ < line) && (read_bytes >= 0))
    {
        read_bytes = fp->readLine(temp, MAX_FILE_LINE_LENGTH);
    }

    i--;

    if ((i == line) && (read_bytes >= 0))
    {
        current_line = line;
        error = QFile::NoError;
    }
    else
    {
        current_line = 0;
        error = QFile::ReadError;
    }

    return error;
}

QFile::FileError FilesManager::readLine(char* data, quint32 size)
{
    QFile::FileError return_code = QFile::NoError;

    if (!fp->isOpen())
    {
        current_line_length = -1;

        return QFile::OpenError;
    }

    int ret = fp->readLine(data, size);

    // No error (0) or Only "n" bytes were read (>0)
    if (ret >= 0)
    {
        return_code = QFile::NoError;
        current_line++;
    }
    else if (ret == -1)
    {
        // Nothing was read
        return_code = QFile::ReadError;
    }

    current_line_length = ret;

    return return_code;
}

QFile::FileError FilesManager::readLine(QString &lineString)
{
    QFile::FileError return_code = QFile::NoError;
    char data[65535];

    if (!fp->isOpen())
    {
        current_line_length = -1;

        return QFile::OpenError;
    }

    int ret = fp->readLine(data, sizeof(data));

    lineString = QString(data).trimmed();

    // Skip Commented lines
    if (lineString.startsWith("//"))
    {
        lineString.clear();
        ret = 0;
    }

    // Nothing was read (EOF reached) //or NULL string
    if (ret == -1)// || lineString.isEmpty())
    {
        return_code = QFile::ReadError;
    }
    else if (ret >= 0)
    {
        // No error (0) or Only "n" bytes were read (>0)
        return_code = QFile::NoError;
        current_line++;
    }

    current_line_length = ret;

    return return_code;
}
