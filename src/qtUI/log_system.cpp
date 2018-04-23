#include <QDate>
#include <QDir>
#include <QtDebug>
#include <QMessageBox>
#include <QCoreApplication>
#include "log_system.h"

LogSystem::LogSystem(QString _strLogDir)
{
    strLogDir = _strLogDir;
    QDir logDir(strLogDir);
    if (!logDir.exists()) {
        logDir.mkpath(strLogDir);
    }
    logFile = new QFile(strLogDir + "/" + QDate::currentDate().toString("yyyy-MM-dd") + ".log");
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "日志文件创建失败!";
    }
    logFile->write(QString("\r\n").toLatin1());

    curDate = QDate::currentDate();
    connect(&curTimer, SIGNAL(timeout()), this, SLOT(TimerUpdate()));
    curTimer.start(1000);
}

void LogSystem::TimerUpdate()
{
    if (QDate::currentDate() > curDate) {
        curDate = QDate::currentDate();
        logFile->flush();
        logFile->close();
        logFile = new QFile(strLogDir + "/" + QDate::currentDate().toString("yyyy-MM-dd") + ".log");
        if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
            qDebug() << "日志文件创建失败!";
        }
        logFile->write(QString("\r\n").toLatin1());
    }
}

LogSystem::~LogSystem()
{
    logFile->flush();
    logFile->close();
}
