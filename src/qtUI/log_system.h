#ifndef LOG_SYSTEM_H
#define LOG_SYSTEM_H

#include <QTimer>
#include <QFile>
#include <QDate>

class LogSystem : public QObject
{
    Q_OBJECT
public:
    LogSystem(QString _strLogDir);
    ~LogSystem();

    QFile* logFile;

private:
    QString strLogDir;
    QDate curDate;
    QTimer curTimer;

private slots:
    void TimerUpdate();
};

#endif // LOG_SYSTEM_H
