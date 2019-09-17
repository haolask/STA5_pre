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
#ifndef FILESMANAGER_H
#define FILESMANAGER_H

#include <QObject>
#include <QFileDialog>
#include "common.h"

class FilesManager
{
    public:
        explicit FilesManager(QString _fileMngrInstancerName = "Not Available");
        ~FilesManager();

        QList<ChunkTy> GetChunkListObj(){return ChunkList;}
        void ClearChunkList(){ChunkList.clear();}

        QStringList LoadFile(QString fullFileName);
        void TokenizeFile(QString separatorStr, QStringList linesList);

        void SaveFile(QString fullFileName, QStringList linesList);
        void SaveFile(QString fullFileName, QString str);

        QString ChunkToString();
        QStringList ChunkToStringList();

        void SetKeyValue(QString key, QString newValue);
        QString GetKeyValue(QString key);
        QFile::FileError open(QString &filename, QFile::OpenMode mode, bool must_exist);
        void close(void);
        QFile::FileError seek(quint32 position);
        QFile::FileError seekLine(quint32 line);
        QFile::FileError readLine(char* data, quint32 size);
        QFile::FileError readLine(QString& lineString);

    private:
        QList<ChunkTy>      ChunkList;
        void Test();
        void ex_1(QString testDescription);
        QFile               *fp;
        quint32             current_line_length;
        quint32             current_line;
};

#endif // SETTINGSMANAGER_H
