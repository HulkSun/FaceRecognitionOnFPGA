#ifndef MYQLABEL_H
#define MYQLABEL_H

#include <QLabel>
#include <QPainter>

class MyQLabel : public QLabel
{
    Q_OBJECT
public:
    MyQLabel();

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // MYQLABEL_H
