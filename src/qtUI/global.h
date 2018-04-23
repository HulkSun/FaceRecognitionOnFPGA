#ifndef GLOBAL_H
#define GLOBAL_H

/* QT header */
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QDateTime>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollBar>
#include "log_system.h"

struct CameraConfig{
    QString name;
    QString url;
};

struct QPersonInfo
{
    QString id;
    QString name;
    QString sex;
    QDateTime timestamp;
};

struct QFaceImageInfo
{
    QString id;
    std::vector<float> feature;
    QString path;
};

struct SearchPersonInfo
{
    std::string id;
    std::string name;
    std::string sex;
    std::string path;
    float sim;
};

struct ShowPersonInfo
{
    int camearId;
    QString name;
    QString sex;
    QDateTime datetime;
    QPixmap snapshot;
    QString path;
    float sim;
};

extern LogSystem* logSystem;
extern int GpuId;
extern int DataDim;
extern int VideoNum;
extern std::string strModelDir;
extern QString QstrLogDir;
extern QVector<CameraConfig> Cameras;

void MessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);
bool ReadConfig(QString iniPath);
bool HistoryDbConnect();

#endif // GLOBAL_H
