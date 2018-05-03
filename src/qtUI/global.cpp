#include "global.h"
#include "blacklist_database.h"
#include "mtcnn.h"
#include "center_face.h"
#include <QFile>
#include <QMutex>
#include <QSettings>
#include <QVector>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "featureExtractor.h"

BlackListDataBase *blacklist_database;
MTCNN *MTCNNDetector;
//  CenterFace *CenterExtractor;
FeatureExtractor &FPGAExtractor = FeatureExtractor::getInstance();
cv::PCA pca;
QMutex qmtx;
int GpuId;
int DataDim;
int VideoNum;
std::string strModelDir;
QString QstrLogDir;
QVector<CameraConfig> Cameras;
LogSystem *logSystem;

void MessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString message = QTime::currentTime().toString("[hh:mm:ss] ");
    switch (type)
    {
    case QtDebugMsg:
        break;
    case QtWarningMsg:
        message += QString("Warning: ");
        break;
    case QtCriticalMsg:
        message += QString("Critical: ");
        break;
    case QtFatalMsg:
        message += QString("Fatal: ") + QString(context.file) + QString(", Line: ") + QString(context.line) + ", ";
        break;
    }
    message += msg + QString("\r\n");

    static QMutex logMtx;
    logMtx.lock();
    logSystem->logFile->write(message.toLatin1());
    logSystem->logFile->flush();
    logMtx.unlock();
}

bool ReadConfig(QString iniPath)
{
    QFile iniFile(iniPath);
    if (!iniFile.exists())
    {
        qDebug() << "\"config.ini\" not exist.";
        return false;
    }

    QSettings iniConfigRead(iniPath, QSettings::IniFormat);

    GpuId = iniConfigRead.value("GpuId").toString().toInt();
    DataDim = iniConfigRead.value("DataDim").toString().toInt();

    iniConfigRead.beginGroup("IpCamera");
    QStringList cameraList = iniConfigRead.childGroups();
    for (int i = 0; i < cameraList.size(); ++i)
    {
        iniConfigRead.beginGroup(cameraList.at(i));
        CameraConfig camera;
        camera.name = iniConfigRead.value("CameraName").toString();
        camera.url = iniConfigRead.value("Url").toString();
        Cameras.push_back(camera);
        iniConfigRead.endGroup();
    }
    iniConfigRead.endGroup();
    VideoNum = Cameras.size();

    iniConfigRead.beginGroup("Model");
    strModelDir = iniConfigRead.value("ModelDir", "../model").toString().toStdString();
    iniConfigRead.endGroup();

    iniConfigRead.beginGroup("Log");
    QstrLogDir = iniConfigRead.value("LogDir", "./logs").toString();
    iniConfigRead.endGroup();

    return true;
}

bool HistoryDbConnect()
{
    QSqlDatabase HistoryDb = QSqlDatabase::addDatabase("QSQLITE");
    HistoryDb.setDatabaseName("./HistoryInfo.db");
    if (!HistoryDb.open())
    {
        qDebug() << "不能打开数据库。";
        return false;
    }
    QSqlQuery query;
    query.exec("create table HistoryInfo(CameraName QString, "
               "PersonId QString, PersonName QString, PersonSex QString, TimeStamp QString, SimDegree float, "
               "FramePath QString, SnapshotPath QString, IdentifyPath QString)");
    return true;
}
