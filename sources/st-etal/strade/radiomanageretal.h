#ifndef RADIOMANAGERETAL_H
#define RADIOMANAGERETAL_H

#include <QObject>

#include "radiomanagerbase.h"

class RadioManagerEtal : public QObject
{
    Q_OBJECT
public:
    explicit RadioManagerEtal(QObject *parent = 0);

signals:

public slots:
};

#endif // RADIOMANAGERETAL_H
