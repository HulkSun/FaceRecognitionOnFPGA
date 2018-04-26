#include "track_widget.h"
extern BlackListDataBase *blacklist_database;

TrackWidget::TrackWidget(QWidget *parent)
    : QWidget(parent)
{
    video_src_combox = new QComboBox();
    for (int i = 0; i < VideoNum; ++i) {
        video_src_combox->addItem(Cameras[i].name);
    }
    video_src_combox->setFixedWidth(120);
    video_src_combox->setFixedHeight(30);
    video_src_combox->setStyleSheet("QComboBox{"
                                    "background: rgb(29,33,42);"
                                    "color: rgb(255,255,255);"
                                    "border: 1px solid rgb(43,49,61);"
                                    "border-radius: 4px;"
                                    "padding: 0px 10px 0px 10px;}"                // 上 右 下 左
                                    "QComboBox::drop-down{"
                                    "subcontrol-origin: padding;"
                                    "subcontrol-position: top right;"
                                    "width: 20px;"
                                    "padding-right: 2px;"
                                    "border-left-width: none;"
                                    "border-top-right-radius: 3px;"
                                    "border-bottom-right-radius: 3px;}"
                                    "QComboBox::down-arrow{"
                                    "image: url(:/button/combobox_arrow_down);}"
                                    );
    QListView *list_view = new QListView(video_src_combox);
    list_view->setStyleSheet("QListView::item{"
                             "background-color: rgb(29,33,42);"
                             "color: rgb(255,255,255);"
                             "padding: 0px 10px 0px 10px;"
                             "border-bottom:3px solid rgb(38,42,53);"
                             "height: 27px;}"
                             "QListView::item:selected{"
                             "background-color: rgb(66,169,235);}");
    video_src_combox->setView(list_view);
    url_lineedit = new QLineEdit();
    url_lineedit->setFixedHeight(30);
    url_lineedit->setTextMargins(8, 0, 8, 0);
    url_lineedit->setPlaceholderText("请选择视频来源");
    url_lineedit->setStyleSheet("background-color: rgb(29,33,41); color: rgb(255,255,255); border: 1px solid rgb(43,49,61); border-radius: 4px");   //  border: 1px solid rgb(66,169,235)
    if (!Cameras.empty()) url_lineedit->setText(Cameras[0].url);
    else url_lineedit->setText(QString("No camera with configure!"));
    start_btn = new QPushButton("打开");
    start_btn->setCursor(Qt::PointingHandCursor);
    start_btn->setFixedWidth(100);
    start_btn->setStyleSheet("QPushButton{background-color: rgb(66,169,235); color: rgb(255,255,255); border: 0px; border-radius: 4px; padding: 2px 4px;}"
                             "QPushButton:hover{background-color: rgb(60,195,245);}"
                             "QPushButton:pressed{background-color: rgb(9,140,188);}");
    choose_video_btn = new QPushButton("选择视频路径");
    choose_video_btn->setCursor(Qt::PointingHandCursor);
    choose_video_btn->setFixedWidth(120);
    choose_video_btn->setStyleSheet("QPushButton{background-color: rgb(66,169,235); color: rgb(255,255,255); border: 2px; border-radius: 4px; padding: 2px 4px;}"
                                    "QPushButton:hover{background-color: rgb(60,195,245);}"
                                    "QPushButton:pressed{background-color: rgb(9,140,188);}");
    choose_video_btn->hide();

    video_label = new QLabel();
    video_label->resize(QSize(LABELWIDTH, LABELHEIGHT));
    QPixmap pixmap1(":/skin/video_bg");
    pixmap1 = pixmap1.scaled(video_label->size());
    video_label->setPixmap(pixmap1);

    QFont font;
    font.setPointSize(13);
    QLabel* suspect_picture_label = new QLabel();
    suspect_picture_label->setPixmap(QPixmap(":/icon/suspect_logo").scaled(15,15));
    suspect_picture_label->setStyleSheet("background-color:rgb(23,26,33);border:0px;");
    // QLabel* suspect_word_label = new QLabel("嫌疑人报警");
    QLabel* suspect_word_label = new QLabel("识别结果");
    suspect_word_label->setStyleSheet("background-color:rgb(23,26,33); color:rgb(255,255,255);border:0px;");
    QHBoxLayout* tmp_suspect_layout = new QHBoxLayout();
    tmp_suspect_layout->addStretch();
    tmp_suspect_layout->addWidget(suspect_picture_label);
    tmp_suspect_layout->addSpacing(5);
    tmp_suspect_layout->addWidget(suspect_word_label);
    tmp_suspect_layout->addStretch();
    tmp_suspect_layout->setMargin(0);
    tmp_suspect_layout->setContentsMargins(0, 0, 0, 0);
    QLabel* suspect_label = new QLabel();
    suspect_label->setLayout(tmp_suspect_layout);
    suspect_label->setFixedHeight(35);
    suspect_label->setStyleSheet("background-color: rgb(23,26,33); color:rgb(255,255,255); border:1px solid rgb(66,169,235)");

    QLabel* dispatch_label = new QLabel("布控人数：");
    font.setPointSize(15);
    dispatch_label->setFont(font);
    dispatch_label->setStyleSheet("color:rgb(223,12,6);");
    dispatch_num_label = new QLabel();
    dispatch_num_label->setFont(font);
//    dispatch_num_label->setText(QString("%1人").arg(blacklist_database->PersonCount()));
    dispatch_num_label->setText(QString("%1人").arg(0));
    dispatch_num_label->setStyleSheet("color:rgb(223,12,6);");
    QLabel* query_result_label = new QLabel("查询结果：");
    query_result_label->setStyleSheet("color:rgb(255,255,255);");
    query_result_num_label = new QLabel();
    QSqlQuery query;
    query.exec("SELECT * FROM HistoryInfo");
    query_result_num_label->setText(QString("%1对").arg(query.size()));
    query_result_num_label->setStyleSheet("color:rgb(222,220,71);");
    QLabel* check_match_label = new QLabel("复核匹配：");
    check_match_label->setStyleSheet("color:rgb(255,255,255);");
    check_match_num_label = new QLabel();
    check_match_num_label->setText(QString("%1对").arg(0));
    check_match_num_label->setStyleSheet("color:rgb(222,220,71);");
    history_search_btn = new QPushButton("历史记录搜索");
    history_search_btn->setFixedWidth(180);
    history_search_btn->setStyleSheet("QPushButton{background-color:rgb(60,195,245); color:rgb(255,255,255);}");

    face_info_table = new QTableWidget();
    face_info_table->setColumnCount(6);
    face_info_table->setColumnWidth(0, 100);
    face_info_table->setColumnWidth(1, 100);
    face_info_table->setColumnWidth(2, 80);
    face_info_table->setColumnWidth(3, 70);
    face_info_table->setColumnWidth(4, 100);
    face_info_table->setColumnWidth(5, 100);
    face_info_table->setFixedWidth(560);
    face_info_table->setFixedHeight(750);
    face_info_table->horizontalHeader()->setStretchLastSection(true);  // 设置充满表宽度
    face_info_table->horizontalHeader()->setFixedHeight(30);  // 设置表头高度
    face_info_table->horizontalHeader()->setStyleSheet("QHeaderView::section{background:rgb(47, 53, 67); color: rgb(255, 255, 255)}; border: 2px");  // 设置表头背景色
    face_info_table->verticalHeader()->setDefaultSectionSize(FaceInfoTableRowHeight);
    face_info_table->setFrameShape(QFrame::NoFrame);        // StyledPanel
    face_info_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    face_info_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    face_info_table->verticalHeader()->setVisible(false);
    face_info_table->setShowGrid(false);
    face_info_table->verticalScrollBar()->setStyleSheet("QScrollBar:vertical{background:rgb(32,36,45); padding-top:20px; padding-bottom:20px; padding-left:1px; padding-right:1px; border:1px solid rgb(38,42,53); width: 16px;}"       // 整体设置
                                                        "QScrollBar::handle:vertical{background:rgb(47,53,66); border-radius: 6px; border: none;}"        // 设置滚动条
                                                        "QScrollBar::hadnle:vertical:hover{background:rgb(255,255,255); border-radius: 6px; border: none;}"       // 设置鼠标放到滚动条上的状态
                                                        "QScrollBar::add-line:vertical{background:url(:/scrollbar/arrow_down) center on-repeat;}"        // 设置右箭头
                                                        "QScrollBar::sub-line:vertical{background:url(:/scrollbar/arrow_up) center no-repeat;}");        // 设置左箭头
//    QPalette palette = face_info_table->palette();
//    palette.setBrush(QPalette::Base, QBrush(QColor(38,42,53)));
//    face_info_table->setPalette(palette);
    face_info_table->setStyleSheet("QWidget{background-color: rgb(32,36,45);}"          // 设置表格背景色
                                   "QTableWidget::item{border: 1px solid rgb(38, 42, 53)} ");       // 设置网格线宽度及颜色
    QStringList header;
//    header << tr("摄像头") << tr("属性") << tr("识别信息") << tr("快照") << tr("相似度") << tr("时间");
    header << tr("快照") << tr("识别信息") << tr("属性") << tr("相似度") << tr("时间") << tr("摄像头");
    face_info_table->setHorizontalHeaderLabels(header);
	
    custom_plot = new QCustomPlot();
    InitTimeTakeBar();

    playId = 0;
    stopFlag = false;

    QHBoxLayout *url_layout = new QHBoxLayout();
    url_layout->addSpacing(20);
    url_layout->addWidget(video_src_combox);
    url_layout->addSpacing(0);
    url_layout->addWidget(url_lineedit);
    url_layout->addSpacing(15);
    url_layout->addWidget(choose_video_btn);
    url_layout->addSpacing(7);
    // url_layout->addWidget(start_btn);
    url_layout->addSpacing(20);
    url_layout->setMargin(0);

    QVBoxLayout *video_layout = new QVBoxLayout();
    video_layout->addWidget(video_label);
    video_layout->setMargin(0);

    QVBoxLayout *left_layout = new QVBoxLayout();
    left_layout->addSpacing(30);
    // left_layout->addLayout(url_layout);
    left_layout->addSpacing(20);
    left_layout->addLayout(video_layout);
    left_layout->addSpacing(20);

    QHBoxLayout* suspect_layout = new QHBoxLayout();
    suspect_layout->addStretch();
    suspect_layout->addWidget(suspect_label);
    suspect_layout->addStretch();

    QHBoxLayout* dispatch_layout = new QHBoxLayout();
    dispatch_layout->addStretch();
    // dispatch_layout->addWidget(dispatch_label);
    // dispatch_layout->addWidget(dispatch_num_label);
    dispatch_layout->addStretch();

    QHBoxLayout* query_result_layout = new QHBoxLayout();
    query_result_layout->addSpacing(5);
    // query_result_layout->addWidget(query_result_label);
    // query_result_layout->addWidget(query_result_num_label);
    query_result_layout->addStretch();
    // query_result_layout->addWidget(check_match_label);
    // query_result_layout->addWidget(check_match_num_label);
    query_result_layout->addStretch();
    // query_result_layout->addWidget(history_search_btn);
    query_result_layout->addSpacing(5);

    QVBoxLayout *right_layout = new QVBoxLayout();
    right_layout->addSpacing(2);
    right_layout->addLayout(suspect_layout);
    right_layout->addSpacing(2);
    right_layout->addLayout(dispatch_layout);
    right_layout->addSpacing(2);
    right_layout->addLayout(query_result_layout);
    right_layout->addSpacing(2);
    right_layout->addWidget(face_info_table);
    right_layout->addSpacing(20);
//    right_layout->addWidget(custom_plot);
//    right_layout->addSpacing(20);

    QHBoxLayout *main_layout = new QHBoxLayout();
    main_layout->addStretch();
    main_layout->addLayout(left_layout);
    main_layout->addStretch();
    main_layout->addSpacing(20);
    main_layout->addLayout(right_layout);
    main_layout->addStretch();

    setLayout(main_layout);
    connect(video_src_combox, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeLineEditText(int)));
    connect(start_btn, SIGNAL(clicked(bool)), this, SLOT(StartPlayVideo(bool)));
    connect(choose_video_btn, SIGNAL(clicked(bool)), this, SLOT(ChooseVideoPath(bool)));
}

void TrackWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(rect(), QPixmap(":/skin/inside_bg"));
}

void TrackWidget::ChangeLineEditText(QString text)
{
    url_lineedit->clear();
    if(text == QString("本地视频")){
        choose_video_btn->show();
        url_lineedit->setText("../videoData/test20170523.mp4");
    }else if(text == QString("摄像头1")){
        choose_video_btn->hide();
        if (Cameras.size() >= 1) {
            url_lineedit->setText(Cameras[0].url);  //海康模板: rtsp://admin:admin@192.168.1.9:554/cam/realmonitor?channel=1&subtype=0
        } else {
            qDebug() << "error config in \"config.ini\"";
        }
//        if (playFlag_vec[0]) start_btn->setText(tr("关闭"));
//        else start_btn->setText(tr("打开"));
    } else if (text == QString("摄像头2")) {
        choose_video_btn->hide();
        if (Cameras.size() >= 2) {
            url_lineedit->setText(Cameras[1].url);
        } else {
            qDebug() << "error config in \"config.ini\"";
        }
//        if (playFlag_vec[1]) start_btn->setText(tr("关闭"));
//        else start_btn->setText(tr("打开"));
    } else {
        choose_video_btn->hide();
        url_lineedit->setPlaceholderText("请选择视频来源");
    }
}

void TrackWidget::ChangeLineEditText(int index)
{
    url_lineedit->setText(Cameras[index].url);
    playId = index;
}

void TrackWidget::ChooseVideoPath(bool)
{
    QString videoPath = QFileDialog::getOpenFileName(this, tr("Open Video"),
                                                     "..",
                                                     tr("Video Files(*.mp4 *.MP4 *.avi)"));
    if(videoPath.isNull()) return;
    url_lineedit->setText(videoPath);
}

void TrackWidget::StartPlayVideo(bool)
{
    if(start_btn->text() == tr("打开")){
//        stopFlag = false;
        if (video_src_combox->currentText() == QString("本地视频")) {
            start_btn->setText(tr("关闭"));
            emit StartPlayLocalVideoSig(url_lineedit->text());
        }else if (video_src_combox->currentText() == QString("摄像头1")) {
            start_btn->setText(tr("关闭"));
//            playFlag_vec[0] = true;
            emit StartPlayCameraSig(0, url_lineedit->text());
//            emit StartPlayCameraSig();
        } else if (video_src_combox->currentText() == QString("摄像头2")) {
            start_btn->setText(tr("关闭"));
//            playFlag_vec[1] = true;
            emit StartPlayCameraSig(1, url_lineedit->text());
        }
    }else{
//        stopFlag = true;
        start_btn->setText(tr("打开"));
        if (video_src_combox->currentText() == QString("摄像头1")) {
//            playFlag_vec[0] = false;
            emit StopPlaySig(0);
        } else if (video_src_combox->currentText() == QString("摄像头2")) {
//            playFlag_vec[1] = false;
            emit StopPlaySig(1);
        }
    }
}

void TrackWidget::InitPlayerSlot(int , bool flag)
{
    if(flag){
//        stopFlag = true;
//        playFlag_vec[userId] = false;
        QPixmap pixmap(":/skin/video_bg");
        pixmap = pixmap.scaled(QSize(LABELWIDTH, LABELHEIGHT));
//        video_label_vec[userId]->setPixmap(pixmap);
//        video_label_vec[userId]->setAlignment(Qt::AlignCenter);
        video_label->setPixmap(pixmap);
        video_label->setAlignment(Qt::AlignCenter);
        start_btn->setText(tr("打开"));
    }
}

void TrackWidget::UpdateImageSlot(int userId, QPixmap &pixmap)
{
//    if(stopFlag) return;
//    if (!playFlag_vec[userId]) return;
//    qDebug() << "debug: " << userId << playId;
    if (userId != playId) return;
    if(pixmap.isNull()) return;
    if(pixmap.height() > LABELHEIGHT || pixmap.width() > LABELWIDTH){
        float h_zoom = (LABELHEIGHT * 1.0) / pixmap.height();
        float w_zoom = (LABELWIDTH * 1.0) / pixmap.width();
        float zoom_radio = h_zoom > w_zoom ? w_zoom : h_zoom;
        pixmap = pixmap.scaled(QSize(zoom_radio * pixmap.width(), zoom_radio * pixmap.height()));
    }
//    video_label_vec[userId]->setPixmap(pixmap);
//    video_label_vec[userId]->setAlignment(Qt::AlignCenter);
    video_label->setPixmap(pixmap);
    video_label->setAlignment(Qt::AlignCenter);
}

void TrackWidget::UpdateImageSlot(int userId, QImage image)
{
    if(image.isNull()) return;
    QPixmap pixmap = QPixmap::fromImage(image);
    UpdateImageSlot(userId, pixmap);
}

void TrackWidget::ShowInTableSlot(int userId, const QString &name, const QString& sex, const QString &imagePath, const QPixmap &shotFace, const float cosSim, int preFaceNum)
{
    int tmpCount = 0;
    for (int i = 0; i < face_info_table->rowCount(); ++i) {
        if (tmpCount == preFaceNum) break;
        if (Cameras[userId].name == face_info_table->item(i, 5)->text()) {
            ++tmpCount;
            if (name == face_info_table->item(i, 2)->text().mid(QString("  ").length(), name.length())) return;
        }
    }
//    for(int i = 0; i <= preFaceNum; ++i){
//        if(face_info_table->rowCount() > i && name == face_info_table->item(i, 2)->text().mid(QString("  ").length(), name.length())) return;
//    }
    int rowHeight = FaceInfoTableRowHeight;

//    QTableWidgetItem *item0 = new QTableWidgetItem();
//    item0->setData(Qt::DecorationRole, shotFace.scaled(rowHeight-20, rowHeight-20));

//    QTableWidgetItem *item1 = new QTableWidgetItem();
//    item1->setData(Qt::DecorationRole, QPixmap(imagePath).scaled(rowHeight-20, rowHeight-20));

    QLabel* item0 = new QLabel();
    item0->setPixmap(shotFace.scaled(rowHeight-20, rowHeight-20));
    QHBoxLayout* item0_layout = new QHBoxLayout();
    item0_layout->addWidget(item0);
    item0_layout->setAlignment(item0, Qt::AlignCenter);
    QWidget* item0_widget = new QWidget();
    item0_widget->setLayout(item0_layout);

    QLabel* item1 = new QLabel();
    item1->setPixmap(QPixmap(imagePath).scaled(rowHeight-20, rowHeight-20));
    QHBoxLayout* item1_layout = new QHBoxLayout();
    item1_layout->addWidget(item1);
    item1_layout->setAlignment(item1, Qt::AlignCenter);
    QWidget* item1_widget = new QWidget();
    item1_widget->setLayout(item1_layout);

    QTableWidgetItem *item2 = new QTableWidgetItem();
    item2->setText(QString("  %1\n  %2").arg(name).arg(sex));
    item2->setTextColor(QColor(255, 255, 255));
//    item2->setTextAlignment(Qt::AlignCenter);

    QTableWidgetItem *item3 = new QTableWidgetItem();
    item3->setText(QString::number(cosSim*100, 'f', 1) + QString("%"));
    item3->setTextColor(QColor(255, 255, 255));
    item3->setTextAlignment(Qt::AlignCenter);

    QTableWidgetItem *item4 = new QTableWidgetItem();
    item4->setText(QString("%1\n%2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")).arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    item4->setTextColor(QColor(255, 255, 255));
    item4->setTextAlignment(Qt::AlignCenter);

    QTableWidgetItem* item5 = new QTableWidgetItem();
    item5->setText(Cameras.at(userId).name);
    item5->setTextColor(QColor(255, 255, 255));
    item5->setTextAlignment(Qt::AlignCenter);

    int rowCount = face_info_table->rowCount();
    if(rowCount >= MAXROWCOUNT) {
        face_info_table->removeRow(rowCount - 1);
    }
    face_info_table->insertRow(0);
    face_info_table->setRowHeight(0, rowHeight);

//    face_info_table->setItem(0, 0, item0);
//    face_info_table->setItem(0, 1, item1);
    face_info_table->setCellWidget(0, 0, item0_widget);
    face_info_table->setCellWidget(0, 1, item1_widget);
    face_info_table->setItem(0, 2, item2);
    face_info_table->setItem(0, 3, item3);
    face_info_table->setItem(0, 4, item4);
    face_info_table->setItem(0, 5, item5);
}

void TrackWidget::InitTimeTakeBar()
{
    // set background color:
    custom_plot->setBackground(QBrush(QColor(38, 42, 53)));

    // create empty bar chart objects:
    detectBar = new QCPBars(custom_plot->xAxis, custom_plot->yAxis);
    alignBar = new QCPBars(custom_plot->xAxis, custom_plot->yAxis);
    extractBar = new QCPBars(custom_plot->xAxis, custom_plot->yAxis);
    queryBar = new QCPBars(custom_plot->xAxis, custom_plot->yAxis);
    detectBar->setAntialiased(false);
    alignBar->setAntialiased(false);
    extractBar->setAntialiased(false);
    queryBar->setAntialiased(false);
    detectBar->setStackingGap(0);
    alignBar->setStackingGap(0);
    extractBar->setStackingGap(0);
    queryBar->setStackingGap(0);

    // set names and colors:
    detectBar->setName("人脸检测");
    detectBar->setPen(QPen(QColor(62, 123, 240).lighter(170)));
    detectBar->setBrush(QColor(62, 123, 240));
    alignBar->setName("人脸对齐");
    alignBar->setPen(QPen(QColor(69, 179, 132).lighter(150)));
    alignBar->setBrush(QColor(69, 179, 132));
    extractBar->setName("特征提取");
    extractBar->setPen(QPen(QColor(240, 126, 89).lighter(130)));
    extractBar->setBrush(QColor(240, 126, 89));
    queryBar->setName("特征查询");
    queryBar->setPen(QPen(QColor(251, 66, 98).lighter(110)));
    queryBar->setBrush(QColor(251, 66, 98));

    // stack bars on top of each other:
    alignBar->moveAbove(detectBar);
    extractBar->moveAbove(alignBar);
    queryBar->moveAbove(extractBar);

    // prepare x axis:
//    custom_plot->xAxis->setRange(0, 8);
    custom_plot->xAxis->setBasePen(QPen(Qt::white));
    custom_plot->xAxis->setTickPen(QPen(Qt::white));
    custom_plot->xAxis->setTickLabelColor(Qt::white);
    custom_plot->xAxis->setLabelColor(Qt::white);
    custom_plot->xAxis->grid()->setVisible(true);
    custom_plot->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));

    // prepare y axis:
    custom_plot->yAxis->setRange(0, 100);
    custom_plot->yAxis->setPadding(5); // a bit more space to the left border
    custom_plot->yAxis->setLabel("耗时（ms）");
    custom_plot->yAxis->setBasePen(QPen(Qt::white));
    custom_plot->yAxis->setTickPen(QPen(Qt::white));
    custom_plot->yAxis->setTickLabelColor(Qt::white);
    custom_plot->yAxis->setLabelColor(Qt::white);
//    custom_plot->yAxis->grid()->setSubGridVisible(true);
    custom_plot->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));

    // setup legend:
    custom_plot->legend->setVisible(true);
    custom_plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignHCenter);  // Qt::AlignHCenter
    custom_plot->legend->setBrush(QColor(255, 255, 255, 100));
    custom_plot->legend->setBorderPen(Qt::NoPen);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    custom_plot->legend->setFont(legendFont);
    custom_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void TrackWidget::ShowTimeTakeBarSlot(int, QVector<double> timeTake)
{
    static int count = 0;
    static QVector<double> ticks;
    static QVector<QString> labels;
    ticks.append(count);
    labels.append(QTime::currentTime().toString("hh:mm:ss.zzz"));
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);
    custom_plot->xAxis->setTicker(textTicker);
    custom_plot->xAxis->setTickLabelRotation(60);
    // add data:
    static QVector<double> detectData, alignData, extractData, queryData;
    detectData.append(timeTake[0]);
    alignData.append(timeTake[1]);
    extractData.append(timeTake[2]);
    queryData.append(timeTake[3]);
    detectBar->setData(ticks, detectData);
    alignBar->setData(ticks, alignData);
    extractBar->setData(ticks, extractData);
    queryBar->setData(ticks, queryData);
    // make key axis range scroll with the data (at a constant range size if 8):
    custom_plot->xAxis->setRange(count, 8, Qt::AlignRight);
    custom_plot->replot();
    count++;
}

QPixmap TrackWidget::MatToQPixmap(cv::Mat &cvImg)
{
    switch ( cvImg.type() )
    {
    case CV_8UC4: // 8-bit, 4 channel
    {
        QImage image( cvImg.data,
                      cvImg.cols, cvImg.rows,
                      static_cast<int>(cvImg.step),
                      QImage::Format_ARGB32 );

        return QPixmap::fromImage(image);
    }
    case CV_8UC3: // 8-bit, 3 channel
    {
        QImage image( cvImg.data,
                      cvImg.cols, cvImg.rows,
                      static_cast<int>(cvImg.step),
                      QImage::Format_RGB888 );

        return QPixmap::fromImage(image.rgbSwapped());
    }
    case CV_8UC1: // 8-bit, 1 channel
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        QImage image( cvImg.data,
                      cvImg.cols, cvImg.rows,
                      static_cast<int>(cvImg.step),
                      QImage::Format_Grayscale8 );
#else
        static QVector<QRgb>  sColorTable;
        // only create our color table the first time
        if ( sColorTable.isEmpty() )
        {
            sColorTable.resize( 256 );

            for ( int i = 0; i < 256; ++i )
            {
                sColorTable[i] = qRgb( i, i, i );
            }
        }
        QImage image( cvImg.data,
                      cvImg.cols, cvImg.rows,
                      static_cast<int>(cvImg.step),
                      QImage::Format_Indexed8 );
        image.setColorTable( sColorTable );
#endif
        return QPixmap::fromImage(image);
    }
    default:
        qDebug() << "ASM::cvMatToQPixmap() - cv::Mat image type not handled in switch:" << cvImg.type();
        break;
    }
    return QPixmap();
}

void TrackWidget::closeEvent(QCloseEvent *event)
{
    switch(QMessageBox::information(this, tr("关闭提示"), tr("确认关闭？"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
    {
    case QMessageBox::Yes:
        qApp->quit();
        break;
    case QMessageBox::No:
        event->ignore();
        break;
    case QMessageBox::Cancel:
        event->ignore();
        break;
    default:
        event->ignore();
        break;
    }
}

TrackWidget::~TrackWidget()
{
}
