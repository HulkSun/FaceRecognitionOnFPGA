#ifndef TITLE_WIDGET_H
#define TITLE_WIDGET_H

#include <QSignalMapper>
#include <QFont>
#include "global.h"
class TitleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TitleWidget(QWidget *parent = 0);
    ~TitleWidget();

private:
    QPushButton *track_button;
    QPushButton *blacklist_button;
signals:
    void turnPage(int current_page);
private slots:
    void turnPage(QString current_page);
};

#endif // TITLE_WIDGET_H
