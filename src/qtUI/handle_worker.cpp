#include "handle_worker.h"
extern MTCNN *MTCNNDetector;
extern BlackListDataBase *blacklist_database;
extern cv::PCA pca; // pca
extern QMutex qmtx;

/********************************************/
/*             class HandleWorker              */
/********************************************/
HandleWorker::HandleWorker(int _userId)
{
    userId = _userId;
    stopFlag = false;
    preFaceNum = 0; // 用来记录上一帧里人脸个数
    // CenterE = NULL;
    MtcnnD = NULL;
}

void HandleWorker::StopPlaySlot(int _userId)
{
    if (userId != _userId)
        return;
    stopFlag = true;
}

void HandleWorker::StartPlayLocalVideoSlot(QString url)
{
    // if (CenterE == NULL)
    //     CenterE = new CenterFace(strModelDir);
  

    if (MtcnnD == NULL)
        MtcnnD = new MTCNN(strModelDir);

    stopFlag = false;
    cv::VideoCapture capture;
    if (!capture.open(url.toStdString()))
    {
        qDebug() << "Cann't open file" << url;
        return;
    }
    double frame_rate = capture.get(CV_CAP_PROP_FPS);
    int delayTime = 1000 / frame_rate;
    int chaseTime = 0;
    int frameNum = 0;
    long startTime = cv::getTickCount();
    while (1)
    {
        if (stopFlag)
            break;
        long t0 = cv::getTickCount();
        cv::Mat frame;
        if (!capture.read(frame))
            break;

        QTime time;
        time.start();
        if (frameNum % Cycle == 0)
        {
            //            qDebug() << "------------------------";
            MTCNNDetect(frame);
            //            qDebug() << "mtcnn takes" << time.elapsed();
        }
        else
        {
            Tracking(frame);
            //            qDebug() << "kalman takes" << time.elapsed();
        }

        long t1 = cv::getTickCount();
        int dealTime = ((t1 - t0) / cv::getTickFrequency()) * 1000; // millisecond
        // 帧率控制
        if (dealTime < delayTime)
        {
            if ((delayTime - dealTime) >= chaseTime)
            {
                usleep((delayTime - dealTime - chaseTime) * 1000);
                chaseTime = 0;
            }
            else
            {
                chaseTime -= (delayTime - dealTime);
            }
        }
        else
        {
            chaseTime += dealTime - delayTime;
        }
        ++frameNum;
    }
    long endTime = cv::getTickCount();
    qDebug() << "frameNum" << frameNum;
    qDebug() << "总时间" << (endTime - startTime) / cv::getTickFrequency();
    capture.release();
    emit InitPlayerSig(userId, true);
    return;
}

void HandleWorker::StartPlayCameraSlot(int _userId, QString url)
{
    if (userId != _userId)
        return;
    // if (CenterE == NULL)
    //     CenterE = new CenterFace(strModelDir);
    if (MtcnnD == NULL)
        MtcnnD = new MTCNN(strModelDir);

    tim.start();
    stopFlag = false;
    int delayTime = 33;
    int chaseTime = 0;
    int frameNum = 0;
    if (!StartStream(url))
        return;
    QTime allTime;
    allTime.start();
    while (1)
    {
        if (stopFlag)
            break;
        long t0 = cv::getTickCount();
        //获取视频帧
        PlayStream(frameNum);
        long t1 = cv::getTickCount();
        int dealTime = ((t1 - t0) / cv::getTickFrequency()) * 1000;
        // 帧率控制
        if (dealTime < delayTime)
        {
            if ((delayTime - dealTime) >= chaseTime)
            {
                usleep((delayTime - dealTime - chaseTime) * 1000);
                chaseTime = 0;
            }
            else
            {
                chaseTime -= (delayTime - dealTime);
            }
        }
        else
        {
            chaseTime += dealTime - delayTime;
        }
        ++frameNum;
    }
    StopStream();
    emit InitPlayerSig(userId, true);
    qDebug() << "frameNum" << frameNum;
    qDebug() << "总时间" << allTime.elapsed();
}

/*获取视频流*/
bool HandleWorker::StartStream(QString url)
{
    videoStreamIndex = -1;
    av_register_all(); //注册库中所有可用的文件格式和解码器
    avdevice_register_all();
    avformat_network_init();                     //初始化网络流格式，使用 RTSP 网络流时必须先执行
    pAVFormatContext = avformat_alloc_context(); //申请一个 AVFormatContext结构的内存，并进行简单初始化
    pAVFrame = av_frame_alloc();
    pAVFrameBGR = av_frame_alloc();
    if (pAVFrame == NULL)
    {
        qDebug() << "给解码的图片分配空间失败";
        return false;
    }
    if (!this->InitStream(url))
    {
        qDebug() << "初始化视频流失败";
        return false;
    }
    return true;
}

/**
 * @deprecated AVPicture is deprecated. Use AVFrame or imgutils functions instead.
 */
bool HandleWorker::InitStream(QString url)
{
    if (url.isEmpty())
        return false;
    if (av_stristart(url.toStdString().c_str(), "rtsp", NULL))
    {
        av_dict_set(&options, "rtsp_transport", "tcp", 0);
    }
    ////打开视频流
    //// For IPCarema
    // int result = avformat_open_input(&pAVFormatContext, url.toStdString().c_str(), NULL, &options);
    //// For WebCam
    AVInputFormat *inputFmt = av_find_input_format("video4linux2");
    if (NULL == inputFmt)
        std::cout << "Null point!" << std::endl;
    int result = avformat_open_input(&pAVFormatContext, "/dev/video0", inputFmt, NULL);

    if (result < 0)
    {
        qDebug() << "打开视频流失败 " << result;
        return false;
    }
    //获取视频流信息
    result = avformat_find_stream_info(pAVFormatContext, NULL);
    if (result < 0)
    {
        qDebug() << "获取视频流信息失败";
        return false;
    }
    //获取视频流索引
    videoStreamIndex = -1;
    for (uint i = 0; i < pAVFormatContext->nb_streams; ++i)
    {
        if (pAVFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1)
    {
        qDebug() << "获取视频流索引失败";
        return false;
    }
    //获取视频流的分辨率大小
    pAVCodecContext = pAVFormatContext->streams[videoStreamIndex]->codec;
    videoWidth = pAVCodecContext->width;
    videoHeight = pAVCodecContext->height;
    //由AVFrame转 cv::Mat 时用AV_PIX_FMT_BGR24
    //由AVFrame转 QImage 时用AV_PIX_FMT_BGR24
    videoSize = avpicture_get_size(AV_PIX_FMT_BGR24, videoWidth, videoHeight); //AV_PIX_FMT_RGB24
    avpicture_alloc((AVPicture *)pAVFrameBGR, AV_PIX_FMT_BGR24, videoWidth, videoHeight);
    AVCodec *pAVCodec;
    //获取视频流解码器
    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);
    pSwsContext = sws_getContext(videoWidth, videoHeight, pAVCodecContext->pix_fmt, videoWidth, videoHeight, AV_PIX_FMT_BGR24, SWS_BICUBIC, 0, 0, 0);
    //打开对应的解码器
    result = avcodec_open2(pAVCodecContext, pAVCodec, NULL);
    if (result < 0)
    {
        qDebug() << "打开解码器失败";
        return false;
    }
    qDebug() << "初始化视频流成功";
    return true;
}

/**
 * @deprecated avcodec_decode_video2() is deprecated. Use avcodec_send_packet() and avcodec_receive_frame() instead.
 * @deprecated av_free_packet() is deprecated. Use av_packet_unref() instead.
 */
void HandleWorker::PlayStream(int frameNum)
{
    cv::Mat frame(cv::Size(videoWidth, videoHeight), CV_8UC3, cv::Scalar(0));
    if (av_read_frame(pAVFormatContext, &pAVPacket) >= 0)
    { //获取一帧视频的压缩数据
        if (pAVPacket.stream_index == videoStreamIndex)
        {
            avcodec_send_packet(pAVCodecContext, &pAVPacket);
            avcodec_receive_frame(pAVCodecContext, pAVFrame);
            sws_scale(pSwsContext, (const uint8_t *const *)pAVFrame->data, pAVFrame->linesize, 0, videoHeight, pAVFrameBGR->data, pAVFrameBGR->linesize);
            memcpy(frame.data, pAVFrameBGR->data[0], videoSize);

            if (frameNum % Cycle == 0)
            {
                MTCNNDetect(frame);
            }
            else
            {
                Tracking(frame);
            }
            // Mat mat(avctx->height, avctx->width, CV_8UC3, pAVFrameBGR->data[0], pAVFrameBGR->linesize[0]);
            // QImage image(pAVFrameBGR->data[0], videoWidth, videoHeight, QImage::Format_RGB888);
        }
    }
    av_packet_unref(&pAVPacket); //释放资源
}

void HandleWorker::StopStream()
{
    avformat_free_context(pAVFormatContext);
    av_frame_free(&pAVFrame);
    // av_frame_free(&pAVFrameBGR);
    sws_freeContext(pSwsContext);
    av_dict_free(&options);
}

/* MYTCNN */
void HandleWorker::MTCNNDetect(cv::Mat &img)
{
    static QTime lastTimeTake(QTime::currentTime());
    double detectTake = 0;
    double alignTake = 0;
    double extractTake = 0;
    double queryTake = 0;

    trackerVec.clear();
    QTime time;
    time.start();
    std::vector<FaceInfo> faceInfo;
    MtcnnD->Detect(img, faceInfo, minSize, threshold, factor);
    detectTake = time.elapsed();
    //    qDebug() << QString::number(userId) << " | detect takes" << time.elapsed();
    cv::Mat initImage = img.clone();
    QPixmap pixmap = MatToQPixmap(img);
    QPainter painter(&pixmap);
    std::vector<SearchPersonInfo> prePersonInfo = framePersonInfo;
    framePersonInfo.clear();
    int numFace = faceInfo.size();
    for (int i = 0; i != numFace; ++i)
    {
        float x = faceInfo[i].bbox.x1;
        float y = faceInfo[i].bbox.y1;
        float h = faceInfo[i].bbox.x2 - faceInfo[i].bbox.x1 + 1;
        float w = faceInfo[i].bbox.y2 - faceInfo[i].bbox.y1 + 1;
        if (x < 0)
            x = 0;
        if (y < 0)
            y = 0;
        if (x + h > img.rows)
            h = img.rows - x - 1;
        if (y + w > img.cols)
            w = img.cols - y - 1;
        cv::Mat face;

        QTime alignTime;
        alignTime.start();
        face = MtcnnD->Align(initImage, faceInfo[i].facePts); // face could be NULL
        alignTake += alignTime.elapsed();
        //         qDebug() << QString::number(userId) << " | align time" << alignTime.elapsed();

        if (face.empty())
        {
            img(cv::Rect(y, x, w, h)).copyTo(face);
        }

        alignTime.restart();
        //use centerFace
        // std::vector<float> feature1024 = CenterE->ExtractFeature(face);
        // cv::Mat dataMat = cv::Mat(feature1024);
        // qmtx.lock();
        // cv::Mat dst = pca.project(dataMat.t());
        // qmtx.unlock();
        // std::vector<float> feature;
        // feature.assign((float *)dst.datastart, (float *)dst.dataend);

        //use FPGA
        std::vector<float> feature = FPGAE.extractFeature(img);
        extractTake += alignTime.elapsed();
        //        qDebug() << QString::number(userId) << " | extract feature" << alignTime.elapsed();

        alignTime.restart();
        qmtx.lock();
        SearchPersonInfo personInfo = blacklist_database->QueryPersonByFeature(feature);
        qmtx.unlock();
        queryTake += alignTime.elapsed();
        //        qDebug() << QString::number(userId) << " | query time" << alignTime.elapsed();

        float center_x = y + h / 2;
        float center_y = x + w / 2;
        for (unsigned int i = 0; i != frameLocationInfo.size(); ++i)
        {
            float preCenter_x = frameLocationInfo[i].x + frameLocationInfo[i].height / 2;
            float preCenter_y = frameLocationInfo[i].y + frameLocationInfo[i].width / 2;
            if (qAbs(preCenter_x - center_x) + qAbs(preCenter_y - center_y) < LocThr)
            {
                if (personInfo.sim == -1)
                {
                    personInfo.name = prePersonInfo[i].name;
                    personInfo.path = prePersonInfo[i].path;
                    personInfo.sim = prePersonInfo[i].sim;
                    break;
                }
                if (personInfo.name != prePersonInfo[i].name)
                {
                    personInfo.name = (personInfo.sim > prePersonInfo[i].sim) ? personInfo.name : prePersonInfo[i].name;
                    break;
                }
            }
        }

        if (personInfo.sim == -1)
        {
            RepaintRect(&painter, QRect(y, x, w, h), QColor(255, 255, 0));
        }
        else
        {
            //            ShowPersonInfo showPersonInfo;
            //            showPersonInfo.camearId = userId;
            //            showPersonInfo.name = QString::fromStdString(personInfo.name);
            //            showPersonInfo.sex = QString::fromStdString(personInfo.sex);
            //            showPersonInfo.datetime = QDateTime::currentDateTime();
            //            showPersonInfo.snapshot = pixmap.copy(y, x, w, h);
            //            showPersonInfo.path = QString::fromStdString(personInfo.path);
            //            showPersonInfo.sim = personInfo.sim;
            //            emit ShowInTableSig(showPersonInfo, preFaceNum);
            QPixmap qface = pixmap.copy(y, x, w, h);
            emit ShowInTableSig(userId, QString::fromStdString(personInfo.name),
                                QString::fromStdString(personInfo.sex),
                                QString::fromStdString(personInfo.path),
                                qface,
                                personInfo.sim,
                                preFaceNum);
            RepaintRect(&painter, QRect(y, x, w, h), QColor(255, 0, 0));
            painter.setBrush(QColor(255, 0, 0, 100));
            int rect_w = QString::fromStdString(personInfo.name).length() * 28;
            painter.drawRect(QRect(y + w / 2 - rect_w / 2, x - 30, rect_w, 30));
            QFont font = painter.font();
            font.setPointSize(20);
            painter.setFont(font);
            painter.setPen(QPen(QColor(255, 255, 255), 2));
            painter.drawText(y + w / 2 - rect_w / 2, x - 5, QString::fromStdString(personInfo.name));

            // // 保存图像，并写入数据库
            // if (tim.elapsed() > 2000)
            // { // 与上次保存的时间超过1秒再保存
            //     QDir qdir;
            //     QString saveDir = "./saveData";
            //     if (!qdir.exists(saveDir))
            //         qdir.mkdir(saveDir);
            //     QString fileDir = saveDir + "/" + QString::fromStdString(personInfo.name);
            //     if (!qdir.exists(fileDir))
            //         qdir.mkdir(fileDir);
            //     QDateTime tempDataTime = QDateTime::currentDateTime();
            //     QString filePath = fileDir + "/" + QString::number(userId) + "_" + QString::fromStdString(personInfo.name) + "_" + tempDataTime.toString("yyyy-MM-dd_hh:mm:ss") + ".jpg";
            //     pixmap.save(filePath); /* 保存帧图像 */
            //     QString snapshotFilePath = fileDir + "/" + QString::number(userId) + "_" + QString::fromStdString(personInfo.name) + "_" + tempDataTime.toString("yyyy-MM-dd_hh:mm:ss") + "_snapshot.jpg";
            //     QPixmap facePixmap = MatToQPixmap(face);
            //     facePixmap.save(snapshotFilePath); /* 保存快照 */

            //     QSqlQuery query;
            //     query.prepare("insert into HistoryInfo (CameraName, PersonId, PersonName, PersonSex, TimeStamp, FramePath, SnapshotPath, IdentifyPath) "
            //                   "values (:CameraName, :PersonId, :PersonName, :PersonSex, :TimeStamp, :FramePath, :SnapshotPath, :IdentifyPath)");
            //     query.bindValue(":CameraName", Cameras[userId].name);
            //     query.bindValue(":PersonId", QString::fromStdString(personInfo.id));
            //     query.bindValue(":PersonName", QString::fromStdString(personInfo.name));
            //     query.bindValue(":PersonSex", QString::fromStdString(personInfo.sex));
            //     query.bindValue(":TimeStamp", tempDataTime.toString("yyyy-MM-dd_hh:mm:ss"));
            //     query.bindValue(":SimDegree", personInfo.sim);
            //     query.bindValue(":FramePath", filePath);
            //     query.bindValue(":SnapshotPath", snapshotFilePath);
            //     query.bindValue(":IdentifyPath", QString::fromStdString(personInfo.path));
            //     query.exec();
            //     query.finish();
            //     //                query.exec("select CameraName, PersonName from HistoryInfo");
            //     //                while (query.next()) {
            //     //                    QString cameraName = query.value(0).toString();
            //     //                    QString personName = query.value(1).toString();
            //     //                    qDebug() << "camera: " << cameraName << "| person: " << personName;
            //     //                }
            //     tim.restart();
            // }
        }

        framePersonInfo.push_back(personInfo);
        KCFTracker kcfTracker(true, false, true, false); // kcf tracking
        kcfTracker.init(cv::Rect(y, x, w, h), img);      // kcf tracking
        trackerVec.push_back(kcfTracker);                // kcf tracking
                                                         //        qDebug() << "-------------------------------------------";
    }

    if (numFace != 0 && lastTimeTake.elapsed() > 250)
    {
        lastTimeTake.restart();
        QVector<double> timeTake;
        timeTake << detectTake << alignTake / numFace << extractTake / numFace << queryTake / numFace;
        //        emit SendTimeTakeSig(userId, timeTake);
    }

    preFaceNum = numFace;
    emit UpdateImageSig(userId, pixmap);
    return;
}

void HandleWorker::Tracking(cv::Mat &img)
{
    QPixmap pixmap = MatToQPixmap(img);
    QPainter painter(&pixmap);
    frameLocationInfo.clear();
    int numFace = trackerVec.size(); // kcf tracking
    for (int i = 0; i != numFace; ++i)
    {
        cv::Rect kcfRect = trackerVec[i].update(img); // kcf tracking
        frameLocationInfo.push_back(kcfRect);
        float x = kcfRect.y;
        float y = kcfRect.x;
        float w = kcfRect.width;
        float h = kcfRect.height;
        SearchPersonInfo personInfo = framePersonInfo[i];
        if (personInfo.sim == -1)
        {
            RepaintRect(&painter, QRect(y, x, w, h), QColor(255, 255, 0));
        }
        else
        {
            //            ShowPersonInfo showPersonInfo;
            //            showPersonInfo.camearId = userId;
            //            showPersonInfo.name = QString::fromStdString(personInfo.name);
            //            showPersonInfo.sex = QString::fromStdString(personInfo.sex);
            //            showPersonInfo.datetime = QDateTime::currentDateTime();
            //            showPersonInfo.snapshot = pixmap.copy(y, x, w, h);
            //            showPersonInfo.path = QString::fromStdString(personInfo.path);
            //            showPersonInfo.sim = personInfo.sim;
            //            emit ShowInTableSig(showPersonInfo, preFaceNum);
            QPixmap qface = pixmap.copy(y, x, w, h);
            emit ShowInTableSig(userId, QString::fromStdString(personInfo.name),
                                QString::fromStdString(personInfo.sex),
                                QString::fromStdString(personInfo.path),
                                qface,
                                personInfo.sim,
                                preFaceNum);
            RepaintRect(&painter, QRect(y, x, w, h), QColor(255, 0, 0));
            painter.setBrush(QColor(255, 0, 0, 100));
            int rect_w = QString::fromStdString(personInfo.name).length() * 28;
            painter.drawRect(QRect(y + w / 2 - rect_w / 2, x - 30, rect_w, 30));
            QFont font = painter.font();
            font.setPointSize(20);
            painter.setFont(font);
            painter.setPen(QPen(QColor(255, 255, 255), 2));
            painter.drawText(y + w / 2 - rect_w / 2, x - 5, QString::fromStdString(personInfo.name));
        }
    }
    preFaceNum = numFace;
    emit UpdateImageSig(userId, pixmap);
    return;
}

void HandleWorker::RepaintRect(QPainter *painter, QRect rect, QColor color)
{
    int x = rect.x();
    int y = rect.y();
    int w = rect.width();
    int h = rect.height();
    painter->setPen(QPen(color, 4));

    painter->drawLine(QPointF(x, y), QPointF(x + w / 6, y)); // 左上角横线
    painter->drawLine(QPointF(x, y), QPointF(x, y + h / 6)); // 左上角竖线

    painter->drawLine(QPointF(x + w, y), QPointF(x + w * 5 / 6, y)); // 右上角横线
    painter->drawLine(QPointF(x + w, y), QPointF(x + w, y + h / 6)); // 右上角竖线

    painter->drawLine(QPointF(x, y + h), QPointF(x + w / 6, y + h)); // 左下角横线
    painter->drawLine(QPointF(x, y + h), QPointF(x, y + h * 5 / 6)); // 左下角竖线

    painter->drawLine(QPointF(x + w, y + h), QPointF(x + w * 5 / 6, y + h)); // 右下角横线
    painter->drawLine(QPointF(x + w, y + h), QPointF(x + w, y + h * 5 / 6)); // 右下角竖线

    painter->setPen(QPen(painter->pen().color(), 1));
    painter->drawLine(QPointF(x, y + h / 6), QPointF(x, y + h * 5 / 6));         // 左边框
    painter->drawLine(QPointF(x + w / 6, y), QPointF(x + w * 5 / 6, y));         // 上边框
    painter->drawLine(QPointF(x + w, y + h / 6), QPointF(x + w, y + h * 5 / 6)); // 右边框
    painter->drawLine(QPointF(x + w / 6, y + h), QPointF(x + w * 5 / 6, y + h)); // 下边框
}

QPixmap HandleWorker::MatToQPixmap(cv::Mat &cvImg)
{
    switch (cvImg.type())
    {
    case CV_8UC4: // 8-bit, 4 channel
    {
        QImage image(cvImg.data,
                     cvImg.cols, cvImg.rows,
                     static_cast<int>(cvImg.step),
                     QImage::Format_ARGB32);

        return QPixmap::fromImage(image);
    }
    case CV_8UC3: // 8-bit, 3 channel
    {
        QImage image(cvImg.data,
                     cvImg.cols, cvImg.rows,
                     static_cast<int>(cvImg.step),
                     QImage::Format_RGB888);

        return QPixmap::fromImage(image.rgbSwapped());
    }
    case CV_8UC1: // 8-bit, 1 channel
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        QImage image(cvImg.data,
                     cvImg.cols, cvImg.rows,
                     static_cast<int>(cvImg.step),
                     QImage::Format_Grayscale8);
#else
        static QVector<QRgb> sColorTable;
        // only create our color table the first time
        if (sColorTable.isEmpty())
        {
            sColorTable.resize(256);

            for (int i = 0; i < 256; ++i)
            {
                sColorTable[i] = qRgb(i, i, i);
            }
        }
        QImage image(cvImg.data,
                     cvImg.cols, cvImg.rows,
                     static_cast<int>(cvImg.step),
                     QImage::Format_Indexed8);
        image.setColorTable(sColorTable);
#endif
        return QPixmap::fromImage(image);
    }
    default:
        qDebug() << "ASM::cvMatToQPixmap() - cv::Mat image type not handled in switch:" << cvImg.type();
        break;
    }
    return QPixmap();
}

HandleWorker::~HandleWorker()
{
    delete MtcnnD;
    // delete CenterE;
}
