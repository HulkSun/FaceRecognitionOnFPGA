#include "title_widget.h"

TitleWidget::TitleWidget(QWidget *parent)
    : QWidget(parent)
{
    QFont font;
    font.setPointSize(12);
    track_button = new QPushButton(tr("实时监控"));
    track_button->setCursor(Qt::PointingHandCursor);
    track_button->setStyleSheet("background-color: rgb(38,42,53); color: rgb(66,169,235); font: bold 20px; border-top:3px solid rgb(66,169,235);"); //border-style: insert
    track_button->setFixedSize(150, 40);
//    track_button->setStyleSheet("background-color: rgb(255, 255, 255); border-style: inset");
    track_button->setFont(font);
    blacklist_button = new QPushButton(tr("人脸库管理"));
    blacklist_button->setCursor(Qt::PointingHandCursor);
    blacklist_button->setFixedSize(150, 40);
//    blacklist_button->setStyleSheet("background-color: rgb(225, 225, 225)");
    blacklist_button->setStyleSheet("background-color: rgb(47,53,67); color: rgb(102,102,102); font: bold 20px; border-top:3px solid rgb(47,53,67);");
    blacklist_button->setFont(font);

    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->addWidget(track_button, 0, Qt::AlignBottom);
    button_layout->addWidget(blacklist_button, 0, Qt::AlignBottom);
    button_layout->addStretch();
    button_layout->setSpacing(0);

    QVBoxLayout *main_layout = new QVBoxLayout();
    main_layout->addSpacing(30);
    main_layout->addLayout(button_layout);
    main_layout->setSpacing(0);
    main_layout->setMargin(0);
    this->setLayout(main_layout);

    QSignalMapper *signal_mapper = new QSignalMapper(this);
    signal_mapper->setMapping(track_button, QString::number(0, 10));
    signal_mapper->setMapping(blacklist_button, QString::number(1, 10));

    connect(track_button, SIGNAL(clicked()), signal_mapper, SLOT(map()));
    connect(blacklist_button, SIGNAL(clicked()), signal_mapper, SLOT(map()));
    connect(signal_mapper, SIGNAL(mapped(QString)), this, SLOT(turnPage(QString)));
}

void TitleWidget::turnPage(QString current_page)
{
    bool ok;
    int current_index = current_page.toInt(&ok, 10);
    switch(current_index){
    case 0:
        track_button->setStyleSheet("background-color: rgb(38,42,53); color: rgb(66,169,235); font: bold 20px; border-top:3px solid rgb(66,169,235);");
        blacklist_button->setStyleSheet("background-color: rgb(47,53,67); color: rgb(102,102,102); font: bold 20px; border-top:3px solid rgb(47,53,67);");
//        track_button->setStyleSheet("background-color: rgb(255, 255, 255); border-style: inset");
//        blacklist_button->setStyleSheet("background-color: rgb(225, 225, 225)");
        break;
    case 1:
        track_button->setStyleSheet("background-color: rgb(47,53,67); color: rgb(102,102,102); font: bold 20px; border-top:3px solid rgb(47,53,67);");
        blacklist_button->setStyleSheet("background-color: rgb(38,42,53); color: rgb(66,169,235); font: bold 20px; border-top:3px solid rgb(66,169,235);");
//        track_button->setStyleSheet("background-color: rgb(225, 225, 225)");
//        blacklist_button->setStyleSheet("background-color: rgb(255, 255, 255); border-style: inset");
        break;
    default:
        break;
    }
    emit turnPage(current_index);
}

TitleWidget::~TitleWidget()
{

}
