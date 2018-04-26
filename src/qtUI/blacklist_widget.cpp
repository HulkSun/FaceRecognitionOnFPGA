#include "blacklist_widget.h"

BlackListWidget::BlackListWidget(QWidget *parent)
    : QWidget(parent)
{
    /* blacklist left config */
    query_button = new QPushButton();
    query_button->setFixedWidth(50);
    query_button->setCursor(Qt::PointingHandCursor);
    query_button->setToolTip(tr("搜索"));
//    query_button->setFlat(true);            // 实现按钮透明
    query_button->setStyleSheet("QPushButton{background:url(:/button/button_search) center no-repeat; border: none;}");
    query_edit = new QLineEdit();
    query_edit->setFixedWidth(530);
    query_edit->setPlaceholderText(tr("请输入要搜索的id"));
    query_edit->setStyleSheet("background-color: rgb(29,33,41); color: rgb(255,255,255); height: 35px; border: 1px solid rgb(66,169,235); border-radius: 6px;");
    QMargins margins = query_edit->textMargins();
    query_edit->setTextMargins(margins.left(), margins.top(), query_button->width(), margins.bottom());
    QHBoxLayout *query_edit_layout = new QHBoxLayout();
    query_edit_layout->addStretch();
    query_edit_layout->addWidget(query_button);
    query_edit_layout->setSpacing(0);
    query_edit_layout->setContentsMargins(0, 0, 5, 0);
    query_edit->setTextMargins(8, 0, 0, 0);
    query_edit->setLayout(query_edit_layout);

    add_button = new QPushButton(tr("+ 添加"));
    add_button->setCursor(Qt::PointingHandCursor);
    add_button->setFixedWidth(120);
    add_button->setFixedHeight(30);
    add_button->setStyleSheet("QPushButton{background-color: rgb(66,169,235); color: rgb(255,255,255); border: 2px; border-radius: 4px; padding: 2px 4px;}"
                              "QPushButton:hover{background-color: rgb(60,195,245);}"
                              "QPushButton:pressed{background-color: rgb(9,140,188);}");
    return_button = new QPushButton(tr("<< 返回"));
    return_button->setCursor(Qt::PointingHandCursor);
    return_button->setFixedWidth(120);
    return_button->setStyleSheet("QPushButton{background-color: rgb(66,169,235); color: rgb(255,255,255); border: 2px; border-radius: 4px; padding: 2px 4px;}"
                                 "QPushButton:hover{background-color: rgb(60,195,245);}"
                                 "QPushButton:pressed{background-color: rgb(9,140,188);}");
//    return_button->setStyleSheet("border:none");   // 隐藏边框线
    return_button->setFlat(true);
    return_button->hide();
    blacklist_table = new QTableWidget();
    blacklist_table->setColumnCount(5);
    blacklist_table->setColumnWidth(0, 200);
    blacklist_table->setColumnWidth(1, 200);
    blacklist_table->setColumnWidth(2, 300);
    blacklist_table->setColumnWidth(3, 300);
    blacklist_table->setColumnWidth(4, 200);
    blacklist_table->setFixedWidth(1250);
    blacklist_table->horizontalHeader()->setStretchLastSection(true);
    blacklist_table->horizontalHeader()->setFixedHeight(30);
    blacklist_table->horizontalHeader()->setStyleSheet("QHeaderView::section{background:rgb(47,53,67); color: rgb(255,255,255)}");
    blacklist_table->verticalHeader()->setVisible(false);
    blacklist_table->verticalHeader()->setDefaultSectionSize(40);           // 设置行高
    blacklist_table->setMouseTracking(true);
    blacklist_table->viewport()->setMouseTracking(true);
    blacklist_table->setFrameShape(QFrame::NoFrame);            // StyledPanel
    blacklist_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    blacklist_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    blacklist_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    blacklist_table->setShowGrid(false);         // true
    blacklist_table->verticalScrollBar()->setStyleSheet("QScrollBar:vertical{background:rgb(32,36,45); padding-top:20px; padding-bottom:20px; padding-left:1px; padding-right:1px; border:1px solid rgb(38,42,53); width: 16px;}"       // 整体设置
                                                        "QScrollBar::handle:vertical{background:rgb(47,53,66); border-radius: 6px; border: none;}"        // 设置滚动条
                                                        "QScrollBar::hadnle:vertical:hover{background:rgb(255,255,255); border-radius: 6px; border: none;}"       // 设置鼠标放到滚动条上的状态
                                                        "QScrollBar::add-line:vertical{background:url(:/scrollbar/arrow_down) center on-repeat;}"        // 设置右箭头
                                                        "QScrollBar::sub-line:vertical{background:url(:/scrollbar/arrow_up) center no-repeat;}");        // 设置左箭头
    blacklist_table->setStyleSheet("selection-background-color: rgb(0, 120, 215)");         // 设置选中的颜色
    blacklist_table->setStyleSheet("QWidget{background-color: rgb(32,37,45)}"           // 设置表格背景色
                                   "QTableWidget::item{border-top: 1px solid rgb(38, 42, 53); border-bottom: 1px solid rgb(38, 42, 53)}");          // 设置网格线宽度及颜色
    QStringList header;
    header << tr("姓名") << tr("性别") << tr("身份证号") << tr("最后修改时间") << tr("操作");
    blacklist_table->setHorizontalHeaderLabels(header);

    query_result_table = new QTableWidget();
    query_result_table->setColumnCount(5);
    query_result_table->setColumnWidth(0, 200);
    query_result_table->setColumnWidth(1, 200);
    query_result_table->setColumnWidth(2, 300);
    query_result_table->setColumnWidth(3, 300);
    query_result_table->setColumnWidth(4, 200);
    query_result_table->setFixedWidth(1250);
    query_result_table->horizontalHeader()->setStretchLastSection(true);
    query_result_table->horizontalHeader()->setFixedHeight(30);
    query_result_table->horizontalHeader()->setStyleSheet("QHeaderView::section{background:rgb(47,53,67); color: rgb(255,255,255)}");
    query_result_table->verticalHeader()->setVisible(false);
    query_result_table->verticalHeader()->setDefaultSectionSize(40);           // 设置行高
    query_result_table->setFrameShape(QFrame::NoFrame);         // StyledPanel
    query_result_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    query_result_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    query_result_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    query_result_table->setShowGrid(false);         // true
    query_result_table->verticalScrollBar()->setStyleSheet("QScrollBar:vertical{background:rgb(32,36,45); padding-top:20px; padding-bottom:20px; padding-left:1px; padding-right:1px; border:1px solid rgb(38,42,53); width: 16px;}"       // 整体设置
                                                        "QScrollBar::handle:vertical{background:rgb(47,53,66); border-radius: 6px; border: none;}"        // 设置滚动条
                                                        "QScrollBar::hadnle:vertical:hover{background:rgb(255,255,255); border-radius: 6px; border: none;}"       // 设置鼠标放到滚动条上的状态
                                                        "QScrollBar::add-line:vertical{background:url(:/scrollbar/arrow_down) center on-repeat;}"        // 设置右箭头
                                                        "QScrollBar::sub-line:vertical{background:url(:/scrollbar/arrow_up) center no-repeat;}");        // 设置左箭头
    query_result_table->setStyleSheet("selection-background-color: rgb(0, 120, 215)");
    query_result_table->setStyleSheet("QWidget{background-color: rgb(32,37,45)}"
                                      "QTableWidget::item{border-top: 1px solid rgb(38, 42, 53); border-bottom: 1px solid rgb(38, 42, 53)}");          // 设置表格背景色
    query_result_table->setHorizontalHeaderLabels(header);

    stacked_table = new QStackedWidget();
    stacked_table->addWidget(blacklist_table);
    stacked_table->addWidget(query_result_table);

    QHBoxLayout *function_layout = new QHBoxLayout();
    function_layout->addSpacing(10);
    function_layout->addWidget(add_button);
    function_layout->addWidget(return_button);
    function_layout->addStretch();
    // function_layout->addWidget(query_edit);
    QVBoxLayout *left_layout = new QVBoxLayout();
    left_layout->addSpacing(20);
    left_layout->addLayout(function_layout);
    left_layout->addSpacing(20);
    left_layout->addWidget(stacked_table);
    left_layout->addSpacing(20);
    /* end */

    /* blacklist right config */
    picture_label = new MyQLabel();
    picture_label->setFixedSize(250, 250);
    QPixmap pixmap(":/skin/picture_bg");
    picture_label->setPixmap(pixmap.scaled(QSize(picture_label->size().width()-50, picture_label->size().height()-50)));
    picture_label->setAlignment(Qt::AlignCenter);
//    picture_label->setStyleSheet("QLabel{border: 2px solid rgb(66, 169, 235)}");
    name_label = new QLabel(tr("姓       名 : "));
    name_label->setFixedWidth(75);
    name_label->setStyleSheet("color: rgb(255, 255, 255)");
    sex_label = new QLabel(tr("性       别 : "));
    sex_label->setFixedWidth(75);
    sex_label->setStyleSheet("color: rgb(255, 255, 255)");
    id_label = new QLabel(tr("身份证号 : "));
    id_label->setFixedWidth(75);
    id_label->setStyleSheet("color: rgb(255, 255, 255)");

    name_edit = new QLineEdit();
    name_edit->setFixedHeight(30);
    name_edit->setTextMargins(8, 0, 0, 0);
    name_edit->setStyleSheet("background-color: rgb(32,37,45); color: rgb(255,255,255); border: 1px solid rgb(46,52,64);");
    id_edit = new QLineEdit();
    id_edit->setFixedHeight(30);
    id_edit->setTextMargins(8, 0, 0, 0);
    id_edit->setStyleSheet("background-color: rgb(32,37,45); color: rgb(255,255,255); border: 1px solid rgb(46,52,64);");

    male_radio = new QRadioButton("男");
    male_radio->setStyleSheet("QRadioButton{spacing: 5px;font-size: 14px;color: rgb(255,255,255);}"         // 设置文本样式
                                     "QRaidoButton::indicator{width: 22px;height: 22px;}"           // 设置 indicator 子组件的大小
                                     "QRadioButton::indicator:unchecked{image: url(:/button/radio_unchecked);}"     // 设置按钮未选中时的状态
                                     "QRadioButton::indicator:checked{image: url(:/button/radio_checked);}");       // 设置按钮选中时的状态
    female_radio = new QRadioButton("女");
    female_radio->setStyleSheet("QRadioButton{spacing: 5px;font-size: 14px;color: rgb(255,255,255);}"
                                     "QRaidoButton::indicator{width: 22px;height: 22px;}"
                                     "QRadioButton::indicator:unchecked{image: url(:/button/radio_unchecked);}"
                                     "QRadioButton::indicator:checked{image: url(:/button/radio_checked);}");

    upload_button = new QPushButton(tr("+ 添加照片"));
    upload_button->setCursor(Qt::PointingHandCursor);
    upload_button->setFixedWidth(120);
    upload_button->setStyleSheet("QPushButton{background-color: rgb(38,42,53); color: rgb(255,255,255); border: 2px solid rgb(66,169,235); border-radius: 4px; padding: 2px 4px;}");
    modify_button = new QPushButton(tr("修改"));
    modify_button->setCursor(Qt::PointingHandCursor);
    modify_button->setFixedWidth(80);
    modify_button->setStyleSheet("QPushButton{background-color: rgb(66,169,235); color: rgb(255,255,255); border: 2px; border-radius: 4px; padding: 2px 4px;}"
                                 "QPushButton:hover{background-color: rgb(60,195,245);}"
                                 "QPushButton:pressed{background-color: rgb(9,140,188);}");
    save_button = new QPushButton(tr("保存"));
    save_button->setCursor(Qt::PointingHandCursor);
    save_button->setFixedWidth(80);
    save_button->setStyleSheet("QPushButton{background-color: rgb(66,169,235); color: rgb(255,255,255); border: 2px; border-radius: 4px; padding: 2px 4px;}"
                               "QPushButton:hover{background-color: rgb(60,195,245);}"
                               "QPushButton:pressed{background-color: rgb(9,140,188);}");
    cancel_button = new QPushButton(tr("取消"));
    cancel_button->setCursor(Qt::PointingHandCursor);
    cancel_button->setFixedWidth(80);
    cancel_button->setStyleSheet("QPushButton{background-color: rgb(66,169,235); color: rgb(255,255,255); border: 2px; border-radius: 4px; padding: 2px 4px;}"
                                 "QPushButton:hover{background-color: rgb(60,195,245);}"
                                 "QPushButton:pressed{background-color: rgb(9,140,188);}");

    picture_table = new QTableWidget();
    picture_table->setRowCount(1);   // 设置行数
    picture_table->setColumnCount(4);
    picture_table->setRowHeight(0, 75);
    picture_table->setFixedHeight(94);
    picture_table->horizontalHeader()->setVisible(false);   // 隐藏行表头
    picture_table->verticalHeader()->setVisible(false);   // 隐藏列表头
    picture_table->setSelectionBehavior(QAbstractItemView::SelectItems);
    picture_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    picture_table->setFrameShape(QFrame::StyledPanel);
    picture_table->setFocusPolicy(Qt::NoFocus);
    picture_table->setShowGrid(false);
    picture_table->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal{background:rgb(32,36,45); padding-top:1px; padding-bottom:1px; padding-left:20px; padding-right:20px; border:1px solid rgb(38,42,53); height: 16px;}"       // 整体设置
                                                        "QScrollBar::handle:horizontal{background:rgb(47,53,66); border-radius: 6px; border: 1px solid rgb(32,36,45);}"        // 设置滚动条
                                                        "QScrollBar::hadnle:horizontal:hover{background:rgb(255,255,255); border-radius: 6px; border: none;}"       // 设置鼠标放到滚动条上的状态
                                                        "QScrollBar::add-line:horizontal{background:url(:/scrollbar/arrow_right) center on-repeat;}"        // 设置右箭头
                                                        "QScrollBar::sub-line:horizontal{background:url(:/scrollbar/arrow_left) center no-repeat;}");        // 设置左箭头
    picture_table->setStyleSheet("QWidget{background-color: rgb(32,37,45)}"
                                 "QTableWidget::item{border-left: 2px solid rgb(38, 42, 53); border-right: 2px solid rgb(38, 42, 53);"
                                 "border-top: 3px solid rgb(32,37,45); border-bottom: 2px solid rgb(32,37,45);}");

    QHBoxLayout *picture_layout = new QHBoxLayout();
    picture_layout->addSpacing(100);
    picture_layout->addWidget(picture_label);
    picture_layout->addSpacing(100);
    QHBoxLayout *picture_table_layout = new QHBoxLayout();
    picture_table_layout->addSpacing(20);
    picture_table_layout->addWidget(picture_table);
    picture_table_layout->addSpacing(20);
    QHBoxLayout *name_layout = new QHBoxLayout();
    name_layout->addSpacing(20);
    name_layout->addWidget(name_label);
    name_layout->addSpacing(10);
    name_layout->addWidget(name_edit);
    name_layout->addSpacing(20);
    QHBoxLayout *sex_layout = new QHBoxLayout();
    sex_layout->addSpacing(20);
    sex_layout->addWidget(sex_label);
    sex_layout->addSpacing(20);
    sex_layout->addWidget(male_radio);
    sex_layout->addSpacing(40);
    sex_layout->addWidget(female_radio);
    sex_layout->addStretch();
    QHBoxLayout *id_layout = new QHBoxLayout();
    id_layout->addSpacing(20);
    id_layout->addWidget(id_label);
    id_layout->addSpacing(10);
    id_layout->addWidget(id_edit);
    id_layout->addSpacing(20);
    QHBoxLayout *upload_layout = new QHBoxLayout();
//    upload_layout->addSpacing(10);
    upload_layout->addStretch();
    upload_layout->addWidget(upload_button);
    upload_layout->addStretch();
    QHBoxLayout *confirm_layout = new QHBoxLayout();
    confirm_layout->addStretch();
    confirm_layout->addWidget(modify_button);
    confirm_layout->addStretch();
    confirm_layout->addWidget(save_button);
    confirm_layout->addStretch();
    confirm_layout->addWidget(cancel_button);
    confirm_layout->addStretch();

    QVBoxLayout *person_layout = new QVBoxLayout();
    person_layout->addStretch(3);
    person_layout->addLayout(picture_layout);
    person_layout->addStretch(2);
    person_layout->addLayout(picture_table_layout);
    person_layout->addStretch(2);
    person_layout->addLayout(name_layout);
    person_layout->addStretch(1);
    person_layout->addLayout(sex_layout);
    person_layout->addStretch(1);
    person_layout->addLayout(id_layout);
    person_layout->addStretch(1);
    person_layout->addLayout(upload_layout);
    person_layout->addStretch(2);
    person_layout->addLayout(confirm_layout);
    person_layout->addStretch(3);

    person_group = new QGroupBox();
    person_group->setTitle(tr("  添加人脸数据"));
    person_group->setStyleSheet("QGroupBox{border: none; color: rgb(66,169,235); font: bold 18px;}"
                                "QGroupBox::title{subcontrol-origin:margin; subcontrol-position:top left; padding:4 0px; border-left: 4px solid rgb(66,169,235);}");
    person_group->setLayout(person_layout);

    QVBoxLayout *right_layout = new QVBoxLayout();
    right_layout->addSpacing(20);
    right_layout->addWidget(person_group);
    right_layout->addSpacing(20);
    /* end */

    QHBoxLayout *main_layout = new QHBoxLayout();
    main_layout->addStretch();
    main_layout->addLayout(left_layout);
    main_layout->addStretch();
    main_layout->addSpacing(50);
    main_layout->addLayout(right_layout);
    main_layout->addStretch();
    this->setLayout(main_layout);

    connect(query_button, SIGNAL(clicked()), this, SLOT(SearchPersonSlot()));
    connect(add_button, SIGNAL(clicked()), this, SLOT(AddNewPersonSlot()));
    connect(return_button, SIGNAL(clicked()), this, SLOT(ReturnTableSlot()));
    connect(blacklist_table, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(CheckPersonSlot(QTableWidgetItem*)));
    connect(blacklist_table, SIGNAL(entered(QModelIndex)), this, SLOT(ShowItemSlot(QModelIndex)));
    connect(query_result_table, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(CheckQueryPersonSlot(QTableWidgetItem*)));

    connect(upload_button, SIGNAL(clicked()), this, SLOT(UploadPictureSlot()));
    connect(modify_button, SIGNAL(clicked()), this, SLOT(ModifyInfoSlot()));
    connect(save_button, SIGNAL(clicked()), this, SLOT(SaveSlot()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(CancelSlot()));

    IsReadOnly(true);
    current_row = 0;
    operate_type = -1;
}

void BlackListWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(rect(), QPixmap(":/skin/inside_bg"));
}

/**************************************************************/
/*                               public function                             */
/**************************************************************/
bool BlackListWidget::QueryPersonById(QString &qid)
{
    add_button->hide();
    return_button->show();
    query_result_table->clearContents();
    query_result_table->setRowCount(0);
    stacked_table->setCurrentWidget(query_result_table);
    if(!qid.isEmpty()){
        emit QueryPersonSig(qid);
    }
    return true;
}

bool BlackListWidget::AddNewPerson(int row)
{
    int rowCount = blacklist_table->rowCount();
    if(row < 0 || row > rowCount) return false;
    blacklist_table->clearSelection();
    blacklist_table->insertRow(row);
    QTableWidgetSelectionRange range(row, 0, row, blacklist_table->columnCount()-1);
    blacklist_table->setRangeSelected(range, true);

    IsReadOnly(false);
    InitInfoGroup();
    modify_button->setEnabled(false);
    return true;
}

bool BlackListWidget::ShowExistPerson(int row, QString name, QString sex, QString id, QString changeTime)
{
    int rowCount = blacklist_table->rowCount();
    if(row < 0 || row > rowCount) return false;
    blacklist_table->insertRow(row);
    blacklist_table->setItem(row, 0, new QTableWidgetItem(name));
    blacklist_table->item(row, 0)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 0)->setTextColor(QColor(255, 255, 255));
    blacklist_table->item(row, 0)->setTextAlignment(Qt::AlignCenter);
    blacklist_table->setItem(row, 1, new QTableWidgetItem(sex));
    blacklist_table->item(row, 1)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 1)->setTextColor(QColor(255, 255, 255));
    blacklist_table->item(row, 1)->setTextAlignment(Qt::AlignCenter);
    blacklist_table->setItem(row, 2, new QTableWidgetItem(id));
    blacklist_table->item(row, 2)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 2)->setTextColor(QColor(255, 255, 255));
    blacklist_table->item(row, 2)->setTextAlignment(Qt::AlignCenter);
    blacklist_table->setItem(row, 3, new QTableWidgetItem(changeTime));
    blacklist_table->item(row, 3)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 3)->setTextColor(QColor(255, 255, 255));
    blacklist_table->item(row, 3)->setTextAlignment(Qt::AlignCenter);
    blacklist_table->setItem(row, 4, new QTableWidgetItem(tr("删除")));
    blacklist_table->item(row, 4)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 4)->setTextColor(QColor(66, 169, 235));
    blacklist_table->item(row, 4)->setTextAlignment(Qt::AlignCenter);

    QTableWidgetSelectionRange range(0, 0, 0, blacklist_table->columnCount()-1);
    blacklist_table->setRangeSelected(range, true);
    CheckPerson(0);
    return true;
}

bool BlackListWidget::DeleteOnePerson(int row)
{
    int rowCount = blacklist_table->rowCount();
    if(row >= 0 && row <= rowCount){
        QString qid = blacklist_table->item(row, 2)->text();
        blacklist_table->removeRow(row);
        emit DeletePersonSig(qid);
        // 清空该id的人脸照片
        QString idPath = "./facesData/" + qid;
        QDir idDir(idPath);
        if (!idDir.exists()) return true;
        QFileInfoList fileList = idDir.entryInfoList(QDir::Dirs | QDir::Files);
        for (int i = 0; i < fileList.count(); ++i) {
            if (fileList[i].fileName() == QString(".") || fileList[i].fileName() == QString("..")) {continue;}
            QFileInfo curFile = fileList[i];
            if (curFile.isFile()) {
                QFile fileTemp(curFile.filePath());
                fileTemp.remove();
                fileList.removeAt(i);
            }
        }
        idDir.remove(".");
    }
    return true;
}

bool BlackListWidget::CheckPerson(int row)
{
    row = row >= 0 ? row : 0;
    if(row >= blacklist_table->rowCount())
        return false;
    name_edit->setText(blacklist_table->item(row, 0)->text());
    id_edit->setText(blacklist_table->item(row, 2)->text());
    QString sex = blacklist_table->item(row, 1)->text();
    if(sex == QString("男")) male_radio->setChecked(true);
    else if(sex == QString("女")) female_radio->setChecked(true);
    IsReadOnly(true);
    modify_button->setEnabled(true);
    QString qid = id_edit->text();
    emit GetFacesSig(qid);
    return true;
}

bool BlackListWidget::UploadImage()
{
    QDir *dir = new QDir();
    QString facesDir = "./facesData";
    if(!dir->exists(facesDir)) dir->mkdir(facesDir);
    if(id_edit->text().isEmpty()){
        QMessageBox::warning(this, tr("提示"), tr("错误！人员 id 为空，请先填写 id ，再上传照片"));
        return false;
    }
    QString idDir = facesDir + "/" + id_edit->text();
    if(!dir->exists(idDir)){ // 没有文件夹就新建
        dir->mkdir(idDir);
    }else{ // 有则清空该文件夹
        QDir dir(idDir);
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        QFileInfoList fileInfoList = dir.entryInfoList();
        for(int i = 0; i != fileInfoList.count(); ++i){
            QFileInfo curFile = fileInfoList[i];
            if(curFile.isFile()){
                QFile fileTemp(curFile.filePath());
                fileTemp.remove();
            }
        }
    }
    QStringList files = QFileDialog::getOpenFileNames(this, tr("QFileDialog::getOpenFileNames()"), "../facesData", tr("Images(*.jpg *.png *.bmp *.tif)"), 0, 0);
    double threshold[3] = {0.8, 0.9, 0.9};
    double factor = 0.5;
    int minSize = 80;
    for(int i = 0; i != files.count(); ++i){
        QString destPath = idDir + "/" + id_edit->text() + "_" + QString::number(i, 10) + ".jpg";
        if(QFile::exists(destPath)) QFile::remove(destPath);
        // 用 mtcnn 自动提取图片的人脸照片
        cv::Mat image = cv::imread(files[i].toStdString().c_str());

        std::vector<FaceInfo> faceInfo;
        MTCNNDetector->Detect(image, faceInfo, minSize, threshold, factor);
        int numFace = faceInfo.size();
        if(numFace != 1){
            qDebug() << "错误！"<<files[i]<<"含有多张人脸，无法确定哪一张是"<<name_edit->text()<<"的脸！"<<"或者该图片中没有人脸！";
            continue;
        }
        float x = faceInfo[0].bbox.x1;
        float y = faceInfo[0].bbox.y1;
        float h = faceInfo[0].bbox.x2 - faceInfo[0].bbox.x1 + 1;
        float w = faceInfo[0].bbox.y2 - faceInfo[0].bbox.y1 + 1;
        if(x < 0) x = 0;
        if(y < 0) y = 0;
        if(x + h > image.rows) h = image.rows - x - 1;
        if(y + w > image.cols) w = image.cols - y - 1;
        cv::Mat face = MTCNNDetector->Align(image, faceInfo[0].facePts);
        if(face.empty()){
            qDebug() << files[i] << "人脸矫正失败！";
            image(cv::Rect(y, x, w, h)).copyTo(face);
        }

        cv::imwrite(destPath.toStdString().c_str(), face); // face
        // end
        QPixmap pixmap;
        pixmap.load(destPath);
        if(i == 0){
            picture_label->setPixmap(pixmap.scaled(QSize(picture_label->size().width()-50, picture_label->size().height()-50)));
            picture_label->setAlignment(Qt::AlignCenter);
        }else{
            QLabel *label = new QLabel;
            label->setPixmap(pixmap.scaled(QSize(70, 70)));
            label->setAlignment(Qt::AlignCenter);
            if(i <= picture_table->columnCount()){
                picture_table->setCellWidget(0, i - 1, label);
            }else{
                picture_table->insertColumn(i - 1);
                picture_table->setCellWidget(0, i - 1, label);
            }
        }
    }
    return true;
}

bool BlackListWidget::IsReadOnly(bool flag)
{ // flag == true : read only; else can read and write
    name_edit->setReadOnly(flag);
    id_edit->setReadOnly(flag);
    male_radio->setEnabled(!flag);
    female_radio->setEnabled(!flag);
    upload_button->setEnabled(!flag);
    return true;
}

bool BlackListWidget::SaveAdd(int row)
{
    QPersonInfo qinfo;
    qinfo.name = name_edit->text();
    if(male_radio->isChecked()) qinfo.sex = QString("男");
    else if(female_radio->isChecked()) qinfo.sex = QString("女");
    qinfo.id = id_edit->text();
    qinfo.timestamp = QDateTime::currentDateTime();
    blacklist_table->setItem(row, 0, new QTableWidgetItem(qinfo.name));
    blacklist_table->item(row, 0)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 0)->setTextColor(QColor(255, 255, 255));
    blacklist_table->item(row, 0)->setTextAlignment(Qt::AlignCenter);
    blacklist_table->setItem(row, 1, new QTableWidgetItem(qinfo.sex));
    blacklist_table->item(row, 1)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 1)->setTextColor(QColor(255, 255, 255));
    blacklist_table->item(row, 1)->setTextAlignment(Qt::AlignCenter);
    blacklist_table->setItem(row, 2, new QTableWidgetItem(qinfo.id));
    blacklist_table->item(row, 2)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 2)->setTextColor(QColor(255, 255, 255));
    blacklist_table->item(row, 2)->setTextAlignment(Qt::AlignCenter);
    blacklist_table->setItem(row, 3, new QTableWidgetItem(qinfo.timestamp.toString("yyyy-MM-dd@hh:mm:ss")));
    blacklist_table->item(row, 3)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 3)->setTextColor(QColor(255, 255, 255));
    blacklist_table->item(row, 3)->setTextAlignment(Qt::AlignCenter);
    blacklist_table->setItem(row, 4, new QTableWidgetItem(tr("删除")));
    blacklist_table->item(row, 4)->setBackgroundColor(QColor(34, 50, 64));
    blacklist_table->item(row, 4)->setTextColor(QColor(66, 169, 235));
    blacklist_table->item(row, 4)->setTextAlignment(Qt::AlignCenter);

    QString dirPath = "./facesData/" + qinfo.id;
    QDir dir(dirPath);
    if(!dir.exists()){
        qDebug() << "错误！保存时找不到该人员人脸所在文件夹！";
        emit AddPersonSig(qinfo);
        return false;
    }
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList fileInfoList = dir.entryInfoList();
    if(fileInfoList.count() <= 0){
        qDebug() << "数据库中无该人员照片！";
        emit AddPersonSig(qinfo);
        return false;
    }
    std::vector<QFaceImageInfo> qfaceInfoVec;
    for(int i = 0; i != fileInfoList.count(); ++i){
        QString imagePath = dirPath + "/" + fileInfoList[i].fileName();
        cv::Mat img = cv::imread(imagePath.toStdString());
        QFaceImageInfo qfaceInfo;
        qfaceInfo.id = id_edit->text();
        qfaceInfo.path = imagePath;
        std::vector<float> feature1024 = CenterExtractor->ExtractFeature(img);
        cv::Mat dataMat = cv::Mat(feature1024);
        cv::Mat dst = pca.project(dataMat.t());
        qfaceInfo.feature.assign((float*)dst.datastart, (float*)dst.dataend);
        qfaceInfoVec.push_back(qfaceInfo);
    }
    emit AddPersonAndFaceSig(qinfo, qfaceInfoVec);
    IsReadOnly(true);
    return true;
}

bool BlackListWidget::SaveModify(int row)
{
    QPersonInfo qinfo;
    qinfo.name = name_edit->text();
    if(male_radio->isChecked()) qinfo.sex = QString("男");
    else if(female_radio->isChecked()) qinfo.sex = QString("女");
    qinfo.id = id_edit->text();
    qinfo.timestamp = QDateTime::currentDateTime();
    blacklist_table->setItem(row, 0, new QTableWidgetItem(qinfo.name));
    blacklist_table->item(row, 0)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 0)->setTextColor(QColor(255, 255, 255));
    blacklist_table->item(row, 0)->setTextAlignment(Qt::AlignCenter);
    blacklist_table->setItem(row, 1, new QTableWidgetItem(qinfo.sex));
    blacklist_table->item(row, 1)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 1)->setTextColor(QColor(255, 255, 255));
    blacklist_table->item(row, 1)->setTextAlignment(Qt::AlignCenter);
    blacklist_table->setItem(row, 2, new QTableWidgetItem(qinfo.id));
    blacklist_table->item(row, 2)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 2)->setTextColor(QColor(255, 255, 255));
    blacklist_table->item(row, 2)->setTextAlignment(Qt::AlignCenter);
    blacklist_table->setItem(row, 3, new QTableWidgetItem(qinfo.timestamp.toString("yyyy-MM-dd@hh:mm:ss")));
    blacklist_table->item(row, 3)->setBackgroundColor(QColor(32, 36, 45));
    blacklist_table->item(row, 3)->setTextColor(QColor(255, 255, 255));
    blacklist_table->item(row, 3)->setTextAlignment(Qt::AlignCenter);
    blacklist_table->setItem(row, 4, new QTableWidgetItem(tr("删除")));
    blacklist_table->item(row, 4)->setBackgroundColor(QColor(34, 50, 64));
    blacklist_table->item(row, 4)->setTextColor(QColor(66, 169, 235));
    blacklist_table->item(row, 4)->setTextAlignment(Qt::AlignCenter);

    IsReadOnly(true);
    //Todo: 写入到数据库中
    emit UpdatePersonSig(qinfo);
    return true;
}

bool BlackListWidget::CancelModify(int row)
{
    CheckPerson(row);
    return true;
}

bool BlackListWidget::InitInfoGroup()
{
    QPixmap pixmap(":/skin/picture_bg");
    picture_label->setPixmap(pixmap.scaled(QSize(picture_label->size().width()-50, picture_label->size().height()-50)));
    picture_label->setAlignment(Qt::AlignCenter);
    picture_table->clear();
    name_edit->clear();
    id_edit->clear();
    male_radio->setChecked(true);
    female_radio->setChecked(false);
    upload_button->setEnabled(true);
    return true;
}

/**************************************************************/
/*                               slot function                                 */
/**************************************************************/
void BlackListWidget::SearchPersonSlot()
{
    operate_type = Query;
    // QString id = query_edit->text();
    QString id = "";
    QueryPersonById(id);
}

void BlackListWidget::AddNewPersonSlot()
{
    operate_type = Add;
    bool flag = false;
    current_row = blacklist_table->rowCount();
    for(int i = 0; i != blacklist_table->columnCount(); ++i){
        if(blacklist_table->item(current_row-1, i) != NULL){
            flag = true;
            break;
        }
    }
    if(current_row == 0 || flag){
        AddNewPerson(current_row);
    }else{
        current_row -= 1;
        QTableWidgetSelectionRange range(current_row, 0, current_row, blacklist_table->columnCount()-1);
        blacklist_table->setRangeSelected(range, true);
    }
    blacklist_table->verticalScrollBar()->setSliderPosition(blacklist_table->verticalScrollBar()->maximum());
}

void BlackListWidget::DeletePersonSlot()
{
    operate_type = Delete;
    QList<QTableWidgetItem*> items = blacklist_table->selectedItems();
    int count = items.count();
    for(int i = 0; i != count; ++i){
        if(i == 0) current_row = blacklist_table->row(items.at(i)) - 1;
        DeleteOnePerson(blacklist_table->row(items.at(i)));
    }
    current_row = current_row >= 0 ? current_row : 0;
    if(current_row < blacklist_table->rowCount()){
        QTableWidgetSelectionRange range(current_row, 0, current_row, blacklist_table->columnCount()-1);
        blacklist_table->setRangeSelected(range, true);
        CheckPerson(current_row);
    }else{
        InitInfoGroup();
    }
}

void BlackListWidget::ReturnTableSlot()
{
    add_button->show();
//    delete_button->show();
    return_button->hide();
    stacked_table->setCurrentWidget(blacklist_table);
    // query_edit->clear();
    // query_edit->setPlaceholderText(tr("请输入要查询的id"));
    current_row = blacklist_table->currentRow();
    CheckPerson(current_row);
}

void BlackListWidget::CheckPersonSlot(QTableWidgetItem *item)
{
    QString qid = blacklist_table->item(item->row(),2)->text();
    QString qname = blacklist_table->item(item->row(),0)->text();
    if(item->column() != blacklist_table->columnCount()-1){
        current_row = blacklist_table->currentRow();
        CheckPerson(item->row());
    }else{
        QString delete_info_str = tr("确认删除该用户吗？\n 身份证号: ") + qid + tr("\n 姓        名: ") + qname;
        delete_info_str.split("\n");
        QMessageBox::StandardButton delete_reply = QMessageBox::information(this, tr("删除提示！"), delete_info_str,
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(delete_reply == QMessageBox::Yes){
            if(item->row() == 0){
                if(blacklist_table->rowCount() == 1)
                    InitInfoGroup();
                else{
                    current_row = item->row() + 1;
                    QTableWidgetSelectionRange range(current_row, 0, current_row, blacklist_table->columnCount()-1);
                    blacklist_table->setRangeSelected(range, true);
                    CheckPerson(current_row);
                }
            }else{
                current_row = item->row()-1;
                QTableWidgetSelectionRange range(current_row, 0, current_row, blacklist_table->columnCount()-1);
                blacklist_table->setRangeSelected(range, true);
                CheckPerson(current_row);
            }
            DeleteOnePerson(item->row());
        }
    }
}

void BlackListWidget::CheckQueryPersonSlot(QTableWidgetItem *item)
{
    QString qid = query_result_table->item(item->row(),2)->text();
    QString qname = query_result_table->item(item->row(),0)->text();
    if(item->column() == query_result_table->columnCount() - 1){
        QString delete_info_str = tr("确认删除该用户吗？\n 身份证号: ") + qid + tr("\n 姓        名: ") + qname;
        delete_info_str.split("\n");
        QMessageBox::StandardButton delete_reply = QMessageBox::information(this, tr("删除提示！"), delete_info_str,
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(delete_reply == QMessageBox::Yes){
            query_result_table->removeRow(item->row());
            InitInfoGroup();
            int i = 0;
            for(; i != blacklist_table->rowCount(); ++i){
                if(blacklist_table->item(i, 2)->text() == qid)
                    break;
            }
            DeleteOnePerson(i);
        }
    }
}

void BlackListWidget::UploadPictureSlot()
{
    UploadImage();
}

void BlackListWidget::ModifyInfoSlot()
{
    operate_type = Update;
    IsReadOnly(false);
}

void BlackListWidget::SaveSlot()
{
    modify_button->setEnabled(true);
    if(operate_type == Add)  SaveAdd(current_row);
    if(operate_type == Update) SaveModify(current_row);
}

void BlackListWidget::CancelSlot()
{
    if (operate_type == Update) {
        current_row = blacklist_table->currentRow();
        CancelModify(current_row);
    } else if (operate_type == Add) {
        int curRow = blacklist_table->rowCount() - 1;
        blacklist_table->removeRow(curRow);
        current_row = curRow - 1;
        QTableWidgetSelectionRange range(current_row, 0, current_row, blacklist_table->columnCount()-1);
        blacklist_table->setRangeSelected(range, true);
        CheckPerson(current_row);
    }
}

void BlackListWidget::ShowPersonSlot(QPersonInfo &qinfo)
{
    int rowCount = blacklist_table->rowCount();
    ShowExistPerson(rowCount, qinfo.name, qinfo.sex, qinfo.id, qinfo.timestamp.toString("yyyy-MM-dd@hh:mm:ss"));
}

void BlackListWidget::GetQueryResultSlot(QPersonInfo &qinfo)
{
    int rowCount = query_result_table->rowCount();
    query_result_table->insertRow(rowCount);
    query_result_table->setItem(rowCount, 0, new QTableWidgetItem(qinfo.name));
    query_result_table->item(rowCount, 0)->setBackgroundColor(QColor(32, 36, 45));
    query_result_table->item(rowCount, 0)->setTextColor(QColor(255, 255, 255));
    query_result_table->item(rowCount, 0)->setTextAlignment(Qt::AlignCenter);
    query_result_table->setItem(rowCount, 1, new QTableWidgetItem(qinfo.sex));
    query_result_table->item(rowCount, 1)->setBackgroundColor(QColor(32, 36, 45));
    query_result_table->item(rowCount, 1)->setTextColor(QColor(255, 255, 255));
    query_result_table->item(rowCount, 1)->setTextAlignment(Qt::AlignCenter);
    query_result_table->setItem(rowCount, 2, new QTableWidgetItem(qinfo.id));
    query_result_table->item(rowCount, 2)->setBackgroundColor(QColor(32, 36, 45));
    query_result_table->item(rowCount, 2)->setTextColor(QColor(255, 255, 255));
    query_result_table->item(rowCount, 2)->setTextAlignment(Qt::AlignCenter);
    query_result_table->setItem(rowCount, 3, new QTableWidgetItem(qinfo.timestamp.toString("yyyy-MM-dd@hh:mm:ss")));
    query_result_table->item(rowCount, 3)->setBackgroundColor(QColor(32, 36, 45));
    query_result_table->item(rowCount, 3)->setTextColor(QColor(255, 255, 255));
    query_result_table->item(rowCount, 3)->setTextAlignment(Qt::AlignCenter);
    query_result_table->setItem(rowCount, 4, new QTableWidgetItem(tr("删除")));
    query_result_table->item(rowCount, 4)->setBackgroundColor(QColor(32, 36, 45));
    query_result_table->item(rowCount, 4)->setTextColor(QColor(66, 169, 235));
    query_result_table->item(rowCount, 4)->setTextAlignment(Qt::AlignCenter);

    name_edit->setText(qinfo.name);
    id_edit->setText(qinfo.id);
    QString sex = qinfo.sex;
    if(sex == QString("男")) male_radio->setChecked(true);
    else if(sex == QString("女")) female_radio->setChecked(true);
    IsReadOnly(true);
    modify_button->setEnabled(true);
    emit GetFacesSig(qinfo.id);
}

void BlackListWidget::ShowFacesSlot(std::vector<std::string> facesUrl)
{
    QPixmap pixmap(":/skin/picture_bg");
    picture_label->setPixmap(pixmap.scaled(QSize(picture_label->size().width()-50, picture_label->size().height()-50)));
    picture_label->setAlignment(Qt::AlignCenter);
    picture_table->clear();
    if(facesUrl.size() == 0){
//        qDebug() << "数据库中不存在该人员的人脸信息！";
        return;
    }
    for(size_t i = 0; i != facesUrl.size(); ++i){
        QPixmap pixmap;
        if(!pixmap.load(QString::fromStdString(facesUrl[i]))){
            qDebug() << "错误！" << QString::fromStdString(facesUrl[i]) << "下没有该人脸照片";
            continue;
        }
        if(i == 0){
            picture_label->setPixmap(pixmap.scaled(QSize(picture_label->size().width()-50, picture_label->size().height()-50)));
            picture_label->setAlignment(Qt::AlignCenter);
        }else{
            QLabel *label =new QLabel();
            label->setPixmap(pixmap.scaled(QSize(70, 70)));
            label->setAlignment(Qt::AlignHCenter);
            if(i <= (size_t)picture_table->columnCount()){
                picture_table->setCellWidget(0, i-1, label);
            }else{
                picture_table->insertColumn(i - 1);
                picture_table->setCellWidget(0, i - 1, label);
            }
        }
    }
}

void BlackListWidget::ShowItemSlot(QModelIndex index)
{
    if(index.isValid())
        return;
    QTableWidgetItem *item = blacklist_table->item(index.row(), index.column());
    item->setBackgroundColor(QColor(29, 33, 42));
    QToolTip::showText(QCursor::pos(), item->text());
}

BlackListWidget::~BlackListWidget()
{

}
