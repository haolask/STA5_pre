#ifndef RADIOMANAGERMW_H
#define RADIOMANAGERMW_H

#include <QObject>

#include "radiomanagerbase.h"

class RadioManagerMw : public QObject
{
    Q_OBJECT
public:
    explicit RadioManagerMw(QObject *parent = 0);

signals:

public slots:
};

#endif // RADIOMANAGERMW_H
