#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QLineEdit>
#include <QRadioButton>
#include <QScrollBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QPainter>
#include <QListView>
#include <QtMath>
#include <QVector>
#include "global.h"
#include "handle_worker.h"
#include "center_face.h"
#include "qcustomplot.h"
#define LABELWIDTH 1280         // 1280,     630 = 1280*((128-2)/256)
#define LABELHEIGHT 720         // 720,     354 = 720*((128-2)/256)
#define MAXROWCOUNT 10
#define FaceInfoTableRowHeight 90

class TrackWidget : public QWidget
{
    Q_OBJECT

public:
    TrackWidget(QWidget *parent = 0);
    ~TrackWidget();
    QPixmap MatToQPixmap(cv::Mat &cvImg);
    void closeEvent(QCloseEvent *e);
    void InitTimeTakeBar();

protected:
  void paintEvent(QPaintEvent *);

private:
    QComboBox *video_src_combox;
    QLineEdit *url_lineedit;
    QPushButton *start_btn;
    QPushButton *choose_video_btn;
    QLabel *video_label;
    QTableWidget *face_info_table;
//    QLabel* suspect_label;
//    QLabel* dispatch_label;
    QLabel* dispatch_num_label;
    QLabel* query_result_num_label;
    QLabel* check_match_num_label;
    QPushButton* history_search_btn;
//    QVector<QLabel*> video_label_vec;
    bool stopFlag;
//    QVector<bool> playFlag_vec;
    int playId;
	
    QCustomPlot *custom_plot;
    QCPBars *detectBar;
    QCPBars *alignBar;
    QCPBars *extractBar;
    QCPBars *queryBar;

signals:
    void StartPlayLocalVideoSig(QString url);
    void StartPlayCameraSig(int userId, QString url);
//    void StartPlayCameraSig();
    void StopPlaySig(int userId);

private slots:
    void ChangeLineEditText(QString);
    void ChangeLineEditText(int);
    void StartPlayVideo(bool);
    void ChooseVideoPath(bool);
    void InitPlayerSlot(int, bool);
    void UpdateImageSlot(int userId, QPixmap &pixmap);
    void UpdateImageSlot(int userId, QImage);
    void ShowInTableSlot(int userId, const QString &name, const QString& sex, const QString &imagePath, const QPixmap &shotFace, const float cosSim, int preFaceNum);
    void ShowTimeTakeBarSlot(int userId, QVector<double>);
};

#endif // MAINWIDGET_H
