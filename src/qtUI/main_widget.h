#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QStackedWidget>
#include <QThread>
#include <QMutex>
#include <QVector>
#include "blacklist_database.h"
#include "windows_title.h"
#include "title_widget.h"
#include "track_widget.h"
#include "handle_worker.h"
#include "blacklist_widget.h"

// opencv
#include <opencv2/opencv.hpp>

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    MainWidget(QWidget *parent = 0);
    ~MainWidget();
    bool TrainPCA(QString);
    bool AddFaceDataSet(QString);

protected:
  void paintEvent(QPaintEvent *);

private:
    QStackedWidget *stacked_widget;
    WindowsTitle *windows_title;
    TitleWidget *title_widget;
    TrackWidget *track_widget;
    BlackListWidget *blacklist_widget;
    QThread *blacklist_thread;
    QVector<QThread*> handle_thread_vec;

signals:
    void StartPlayCameraSig(int _userId, QString);

private slots:
    void turnPage(int current_page);
    void MinWindowsSlot();
};

#endif // MAIN_WIDGET_H
