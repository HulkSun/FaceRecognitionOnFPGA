#include "my_qlabel.h"

MyQLabel::MyQLabel()
{
}

void MyQLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    int width = this->width();
    int height = this->height();
    QPainter painter(this);
    painter.setPen(QPen(QColor(66,169,235), 3));

    painter.drawLine(QPointF(0, 0), QPointF(width/8, 0));           // 左上角横线
    painter.drawLine(QPointF(0, 0), QPointF(0, height/8));          // 左上角竖线

    painter.drawLine(QPointF(width, 0), QPointF(width*7/8, 0));           // 右上角横线
    painter.drawLine(QPointF(width, 0), QPointF(width, height/8));          // 右上角竖线

    painter.drawLine(QPointF(0, height), QPointF(width/8, height));           // 左下角横线
    painter.drawLine(QPointF(0, height), QPointF(0, height*7/8));          // 左下角竖线

    painter.drawLine(QPointF(width, height), QPointF(width*7/8, height));           // 右下角横线
    painter.drawLine(QPointF(width, height), QPointF(width, height*7/8));          // 右下角竖线
}
