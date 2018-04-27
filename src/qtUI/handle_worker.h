#ifndef HANDLEWORKER_H
#define HANDLEWORKER_H

#include <QThread>
#include <QApplication>
#include <QMutex>
#include <QDir>
#include <QSqlRecord>
#include <QSqlQuery>
#include <cv.h>
#include <opencv2/opencv.hpp>
#include <highgui.h>
#include <opencv2/features2d.hpp>
#include <fstream>
#include <unistd.h>
#include "mtcnn.h"
#include "center_face.h"
#include "blacklist_database.h"
#include "global.h"
#include "kcftracker.hpp"

#ifndef INT64_C
#define INT64_C
#define UINT64_C
#endif
//引入 ffmpeg 头文件
#ifdef __cplusplus
extern "C"{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavutil/avstring.h>
#include <libavdevice/avdevice.h>
#ifdef __cplusplus
}
#endif

#define Cycle 5
#define LocThr 20

class HandleWorker : public QObject
{
    Q_OBJECT

public:
    HandleWorker(int _userId);
    ~HandleWorker();
    void MTCNNDetect(cv::Mat &img);
    void Tracking(cv::Mat& img);
    QPixmap MatToQPixmap(cv::Mat &cvImg);
    bool StartStream(QString url);
    bool InitStream(QString url);
    void PlayStream(int frameNum);
    void StopStream();
    void RepaintRect(QPainter *painter, QRect rect, QColor color);

private:
    int userId;
    double threshold[3] = {0.7, 0.8, 0.9};
    double factor = 0.5;
    int minSize = 80;   // 120
    QTime tim;

    bool stopFlag;
    int preFaceNum;
    std::vector<SearchPersonInfo> framePersonInfo;
    std::vector<cv::Rect> frameLocationInfo;

    CenterFace *CenterE;
    MTCNN* MtcnnD;

    /*与 ffmpeg 有关的变量*/
    QMutex mutex;
    AVFormatContext *pAVFormatContext;
    AVCodecContext *pAVCodecContext;
    AVFrame *pAVFrame;
    AVFrame *pAVFrameBGR;
    SwsContext *pSwsContext;
    AVPacket pAVPacket;
    AVDictionary *options = NULL;
    int videoStreamIndex;
    int videoWidth;
    int videoHeight;
    int videoSize;

    std::vector<KCFTracker> trackerVec;         // kcf track

signals:
    void InitPlayerSig(int _userId, bool);
    void ShowInTableSig(int _userId, const QString &name, const QString& sex, const QString &imagePath, const QPixmap &shotFace, const float cosSim, int preFaceNum);
//    void ShowInTableSig(ShowPersonInfo, int);
    void UpdateImageSig(int _userId, QPixmap &pixmap);
    void SendImageSig(int _userId, QImage);
    void SendTimeTakeSig(int _userId, QVector<double>);

private slots:
    void StartPlayLocalVideoSlot(QString url);
    void StartPlayCameraSlot(int _userId, QString url);
    void StopPlaySlot(int _userId);
};

#endif // HANDLEWORKER_H
