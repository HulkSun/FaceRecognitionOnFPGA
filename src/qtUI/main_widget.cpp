#include "main_widget.h"

extern BlackListDataBase *blacklist_database;
extern MTCNN *MTCNNDetector;
#ifndef USE_FPGA
extern CenterFace *CenterExtractor;
#endif // !USE_FPGA
extern cv::PCA pca;
extern QMutex qmtx;

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
{
    qRegisterMetaType<QString>("QString&");
    qRegisterMetaType<QVector<double>>("QVector<double>");
    qRegisterMetaType<QPixmap>("QPixmap&");
    qRegisterMetaType<QPersonInfo>("QPersonInfo");
    qRegisterMetaType<QPersonInfo>("QPersonInfo&");
    qRegisterMetaType<QFaceImageInfo>("QFaceImageInfo");
    qRegisterMetaType<QFaceImageInfo>("QFaceImageInfo&");
    qRegisterMetaType<ShowPersonInfo>("ShowPersonInfo");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<std::vector<float>>("std::vector<float>");
    qRegisterMetaType<std::vector<std::vector<float>>>("std::vector<std::vector<float> >");
    qRegisterMetaType<std::vector<FaceInfo>>("std::vector<FaceInfo>");
    qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
    qRegisterMetaType<std::vector<QFaceImageInfo>>("std::vector<QFaceImageInfo>");
    qRegisterMetaType<std::vector<QFaceImageInfo>>("std::vector<QFaceImageInfo&>");
    qRegisterMetaType<std::vector<SearchPersonInfo>>("std::vector<SearchPersonInfo>");

    this->setWindowIcon(QIcon(":/icon/logo"));
    this->setWindowTitle("基于FPGA的人脸识别系统");
    this->setMinimumSize(900, 600);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint); // 设置无边框
    QDesktopWidget *desktop = QApplication::desktop();
    this->setFixedSize(desktop->width(), desktop->height());

    MTCNNDetector = new MTCNN(strModelDir);
#ifndef USE_FPGA
    CenterExtractor = new CenterFace(strModelDir);
#endif // !USE_FPGA

    blacklist_database = new BlackListDataBase(DataDim, "");
    blacklist_thread = new QThread();
    blacklist_database->moveToThread(blacklist_thread);
    blacklist_thread->start();

    stacked_widget = new QStackedWidget(this);
    windows_title = new WindowsTitle(this);
    title_widget = new TitleWidget(this);
    track_widget = new TrackWidget(this);
    blacklist_widget = new BlackListWidget(this);
    stacked_widget->addWidget(track_widget);
    stacked_widget->addWidget(blacklist_widget);

    for (int i = 0; i < VideoNum; ++i)
    {
        QThread *handle_thread = new QThread();
        HandleWorker *handle_worker = new HandleWorker(i);
        handle_worker->moveToThread(handle_thread);
        handle_thread->start();
        handle_thread_vec.push_back(handle_thread);

        connect(track_widget, SIGNAL(StartPlayLocalVideoSig(QString)), handle_worker, SLOT(StartPlayLocalVideoSlot(QString)));
        connect(track_widget, SIGNAL(StartPlayCameraSig(int, QString)), handle_worker, SLOT(StartPlayCameraSlot(int, QString)));
        connect(track_widget, SIGNAL(StopPlaySig(int)), handle_worker, SLOT(StopPlaySlot(int)), Qt::DirectConnection);

        connect(handle_worker, SIGNAL(InitPlayerSig(int, bool)), track_widget, SLOT(InitPlayerSlot(int, bool)));
        connect(handle_worker, SIGNAL(ShowInTableSig(int, QString, QString, QString, QPixmap, float, int)),
                track_widget, SLOT(ShowInTableSlot(int, QString, QString, QString, QPixmap, float, int)));
        //        connect(handle_worker, SIGNAL(ShowInTableSig(ShowPersonInfo,int)), track_widget, SLOT(ShowInTableSlot(ShowPersonInfo,int)));
        connect(handle_worker, SIGNAL(UpdateImageSig(int, QPixmap &)), track_widget, SLOT(UpdateImageSlot(int, QPixmap &)));
        connect(handle_worker, SIGNAL(SendImageSig(int, QImage)), track_widget, SLOT(UpdateImageSlot(int, QImage)));
        connect(handle_worker, SIGNAL(SendTimeTakeSig(int, QVector<double>)), track_widget, SLOT(ShowTimeTakeBarSlot(int, QVector<double>)));

        connect(handle_thread, &QThread::finished, handle_worker, &QObject::deleteLater);
        connect(this, SIGNAL(StartPlayCameraSig(int, QString)), handle_worker, SLOT(StartPlayCameraSlot(int, QString)));
    }

    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(Qt::white));
    track_widget->setPalette(palette);
    track_widget->setAutoFillBackground(true);
    blacklist_widget->setPalette(palette);
    blacklist_widget->setAutoFillBackground(true);

    QVBoxLayout *function_layout = new QVBoxLayout();
    function_layout->addWidget(title_widget);
    function_layout->addWidget(stacked_widget);
    function_layout->addSpacing(50);
    function_layout->setMargin(0);

    QHBoxLayout *down_layout = new QHBoxLayout();
    down_layout->addSpacing(50);
    down_layout->addLayout(function_layout);
    down_layout->addSpacing(50);
    down_layout->setMargin(0);

    QVBoxLayout *main_layout = new QVBoxLayout();
    main_layout->addWidget(windows_title);
    main_layout->addLayout(down_layout);
    main_layout->setSpacing(0);
    main_layout->setMargin(0);
    this->setLayout(main_layout);

    TrainPCA("../data/PCATrainData");

    connect(windows_title, SIGNAL(MinWindowsSig()), this, SLOT(MinWindowsSlot()));
    connect(title_widget, SIGNAL(turnPage(int)), this, SLOT(turnPage(int)));

    connect(blacklist_database, SIGNAL(SendPersonInfoSig(QPersonInfo &)), blacklist_widget, SLOT(ShowPersonSlot(QPersonInfo &)));
    connect(blacklist_database, SIGNAL(SendQueryResultSig(QPersonInfo &)), blacklist_widget, SLOT(GetQueryResultSlot(QPersonInfo &)));
    connect(blacklist_database, SIGNAL(SendFacesUrl(std::vector<std::string>)), blacklist_widget, SLOT(ShowFacesSlot(std::vector<std::string>)));

    connect(blacklist_widget, SIGNAL(AddPersonSig(QPersonInfo &)), blacklist_database, SLOT(AddPersonSlot(QPersonInfo &)), Qt::DirectConnection);
    connect(blacklist_widget, SIGNAL(DeletePersonSig(QString &)), blacklist_database, SLOT(DeletePersonSlot(QString &)));
    connect(blacklist_widget, SIGNAL(UpdatePersonSig(QPersonInfo &)), blacklist_database, SLOT(UpdatePersonInfoSlot(QPersonInfo &)));
    connect(blacklist_widget, SIGNAL(QueryPersonSig(QString &)), blacklist_database, SLOT(QueryPersonSlot(QString &)));
    connect(blacklist_widget, SIGNAL(AddFaceSig(QFaceImageInfo &)), blacklist_database, SLOT(AddFaceSlot(QFaceImageInfo &)));
    connect(blacklist_widget, SIGNAL(GetFacesSig(QString &)), blacklist_database, SLOT(GetFacesSlot(QString &)));
    connect(blacklist_widget, SIGNAL(AddPersonAndFaceSig(QPersonInfo, std::vector<QFaceImageInfo>)), blacklist_database, SLOT(AddPersonAndFaceSlot(QPersonInfo, std::vector<QFaceImageInfo>)));

    connect(blacklist_thread, &QThread::finished, blacklist_database, &QObject::deleteLater);

    for (int i = 0; i < VideoNum; ++i)
    {
        emit StartPlayCameraSig(i, Cameras[i].url);
    }
    blacklist_database->InitBlackList(100, DataDim / 2, 0.7, 1, 0);
    //        AddFaceDataSet("/home/lchy/dataset/facesData");
}

void MainWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(rect(), QPixmap(":/skin/outside_bg"));
}

bool MainWidget::TrainPCA(QString trainDir)
{
    qDebug() << "Training PCA model...";
    QTime tim;
    tim.start();
    std::vector<std::vector<float>> features;
    QDir *dir = new QDir(trainDir);
    int imgCount = 0;

    if (!dir->exists())
    {
        qDebug() << "PCA训练文件夹不存在";
        return false;
    }

    dir->setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList fileInfoList = dir->entryInfoList();

    for (int i = 0; i != fileInfoList.count(); ++i)
    {
        QFileInfo curFile = fileInfoList[i];
        cv::Mat img = cv::imread(curFile.filePath().toStdString());

        if (img.empty())
            continue;

#ifdef USE_FPGA
        std::vector<float> feature = FPGAExtractor.extractFeature(img);
#else
        std::vector<float> feature = CenterExtractor->ExtractFeature(img);
#endif // USE_FPGA
        features.push_back(feature);
        ++imgCount;
    }

    cv::Mat dataSet(imgCount, features.at(0).size(), CV_32FC1);

    for (int i = 0; i != dataSet.rows; ++i)
        for (int j = 0; j != dataSet.cols; ++j)
            dataSet.at<float>(i, j) = features[i][j];

    pca(dataSet, cv::Mat(), CV_PCA_DATA_AS_ROW, DataDim);
    qDebug() << "Total images : " << imgCount << endl;
    qDebug() << "PCA model training done! It take" << tim.elapsed() / 1000.0 << "s.";

    return true;
}

bool MainWidget::AddFaceDataSet(QString _dataSetDir)
{
    qDebug() << "DataSet is being injected...";
    QTime tim;
    tim.start();
    double threshold[3] = {0.8, 0.9, 0.9};
    double factor = 0.5;
    int minSize = 80;

    QString facesDir = "./facesData";
    QDir *storeDir = new QDir();
    if (!storeDir->exists(facesDir))
        storeDir->mkdir(facesDir);

    QDir *dataDir = new QDir(_dataSetDir);
    if (!dataDir->exists())
        return false;
    dataDir->setFilter(QDir::Dirs);
    QFileInfoList subFileList = dataDir->entryInfoList();
    for (int id = 0; id < subFileList.count(); ++id)
    {
        if (subFileList[id].fileName() == QString(".") || subFileList[id].fileName() == QString(".."))
            continue;
        QPersonInfo qinfo;
        qinfo.id = QString::number(id - 1, 10);
        qinfo.name = subFileList[id].fileName();
        qinfo.sex = QString("男");
        qinfo.timestamp = QDateTime::currentDateTime();
        std::vector<QFaceImageInfo> qfaceInfoVec;

        QString idDir = facesDir + "/" + qinfo.id;
        if (!storeDir->exists(idDir))
            storeDir->mkdir(idDir);

        QDir subDataDir(subFileList[id].filePath());
        QStringList filter;
        filter << "*.tif"
               << "*.jpg"
               << "*.png"
               << "*.bmp";
        subDataDir.setNameFilters(filter);
        QFileInfoList sub2FileList = subDataDir.entryInfoList();
        qDebug() << "------------------- " << subFileList[id].filePath() << " -------------------";
        for (int k = 0; k < sub2FileList.count(); ++k)
        {
            if (sub2FileList[k].fileName() == QString(".") || sub2FileList[k].fileName() == QString(".."))
                continue;
            QString imgName = sub2FileList[k].fileName();
            qDebug() << imgName;
            if (imgName.left(1) == QString("F"))
                qinfo.sex = QString("女");
            QString destPath = idDir + "/" + QString::number(id, 10) + "_" + QString::number(k, 10) + ".jpg";
            cv::Mat img = cv::imread(sub2FileList[k].filePath().toStdString().c_str());
            std::vector<FaceInfo> faceInfo;
            MTCNNDetector->Detect(img, faceInfo, minSize, threshold, factor);
            if (faceInfo.size() != 1)
                continue;
            float x = faceInfo[0].bbox.x1;
            float y = faceInfo[0].bbox.y1;
            float h = faceInfo[0].bbox.x2 - faceInfo[0].bbox.x1 + 1;
            float w = faceInfo[0].bbox.y2 - faceInfo[0].bbox.y1 + 1;
            if (x < 0)
                x = 0;
            if (y < 0)
                y = 0;
            if (x + h > img.rows)
                h = img.rows - x - 1;
            if (y + w > img.cols)
                w = img.cols - y - 1;
            cv::Mat face = MTCNNDetector->Align(img, faceInfo[0].facePts);
            if (face.empty())
            {
                img(cv::Rect(y, x, w, h)).copyTo(face);
            }

            QFaceImageInfo qfaceInfo;
            qfaceInfo.id = qinfo.id;
            qfaceInfo.path = destPath;

#ifdef USE_FPGA
            //use FPGA
            std::vector<float> feature = FPGAExtractor.extractFeature(face);
            cv::Mat dataMat = cv::Mat(feature);
#else
            //use center face
            std::vector<float> feature1024 = CenterExtractor->ExtractFeature(face);
            cv::Mat dataMat = cv::Mat(feature1024);

#endif // USE_FPGA
            cv::Mat dst = pca.project(dataMat.t());
            qfaceInfo.feature.assign((float *)dst.datastart, (float *)dst.dataend);
            qfaceInfoVec.push_back(qfaceInfo);
            cv::imwrite(destPath.toStdString().c_str(), face);
        }
        blacklist_database->AddPersonAndFaceSlot(qinfo, qfaceInfoVec);
    }
    qDebug() << "DataSet Injection Done! It takes " << tim.elapsed() / 1000.0 << "s.";
    return true;
}

void MainWidget::turnPage(int current_page)
{
    switch (current_page)
    {
    case 0:
        stacked_widget->setCurrentWidget(track_widget);
        break;
    case 1:
        stacked_widget->setCurrentWidget(blacklist_widget);
        break;
    default:
        break;
    }
}

void MainWidget::MinWindowsSlot()
{
    this->showMinimized();
}

MainWidget::~MainWidget()
{
    delete MTCNNDetector;
#ifndef USE_FPGA
    delete CenterExtractor;
#endif // !USE_FPGA
    delete blacklist_database;
}
