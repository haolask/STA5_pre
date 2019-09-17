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
#include <QApplication>

#include "launchwindow.h"

/***************************************************************************
* Function : main
****************************************************************************/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle( "plastique" );
    a.setPalette( a.style()->standardPalette() );

    LaunchWindow *w = new LaunchWindow(0,QString("LaunchWindow"));

    Q_UNUSED(w);


    return a.exec();
}

