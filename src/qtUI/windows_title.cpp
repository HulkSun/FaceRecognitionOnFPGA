#include "windows_title.h"

WindowsTitle::WindowsTitle(QWidget *parent)
    : QWidget(parent)
{
    // logo_label = new QLabel(tr("基于FPGA的人脸识别系统"));
    // logo_label->setPixmap(QPixmap(":/icon/logo").scaled(35, 35));
    title_label = new QLabel(tr("基于FPGA的人脸识别系统"));
    title_label->setStyleSheet("color: rgb(255,255,255); font: bold 22px;");
    title_label->setFixedHeight(40);

    min_btn = new QPushButton();
    min_btn->setFixedSize(35, 35);
    min_btn->setToolTip(tr("最小化"));
    min_btn->setFlat(true);
    min_btn->setIcon(QIcon(QPixmap(":/button/button_min").scaled(35, 35)));
    max_btn = new QPushButton();
    max_btn->setFixedSize(35, 35);
    max_btn->setToolTip(tr("最大化"));
    max_btn->setFlat(true);
    max_btn->setIcon(QIcon(QPixmap(":/button/button_max").scaled(35, 35)));
    close_btn = new QPushButton();
    close_btn->setFixedSize(35, 35);
    close_btn->setToolTip(tr("关闭"));
    close_btn->setFlat(true);
    close_btn->setIcon(QIcon(QPixmap(":/button/button_close").scaled(35, 35)));

    QHBoxLayout *main_layout = new QHBoxLayout();
    main_layout->addStretch();
    // main_layout->addWidget(logo_label, 0, Qt::AlignVCenter);
    main_layout->addSpacing(20);
    main_layout->addWidget(title_label, 0, Qt::AlignVCenter);
    main_layout->addStretch();
    main_layout->addWidget(min_btn, 0, Qt::AlignVCenter);
    main_layout->addSpacing(5);
    main_layout->addWidget(max_btn, 0, Qt::AlignVCenter);
    main_layout->addSpacing(5);
    main_layout->addWidget(close_btn, 0, Qt::AlignVCenter);
    main_layout->addSpacing(5);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 5);

    this->setLayout(main_layout);
    this->setFixedHeight(50);
    is_move = false;

    connect(min_btn, SIGNAL(clicked()), this, SLOT(MinWindowsSlot()));
    connect(max_btn, SIGNAL(clicked()), this, SLOT(MaxWindowsSlot()));
    connect(close_btn, SIGNAL(clicked()), this, SLOT(CloseWindowsSlot()));
}

void WindowsTitle::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(rect(), QPixmap(":/skin/title_bg"));
    painter.setPen(QPen(QColor(33, 62, 84), 2));
    painter.drawLine(QPointF(0, this->height()), QPointF(this->width(), this->height()));
}

void WindowsTitle::mousePressEvent(QMouseEvent *event)
{
    press_point = event->pos();
    is_move = true;
}

void WindowsTitle::mouseMoveEvent(QMouseEvent *event)
{
    if((event->buttons() == Qt::LeftButton) && is_move){
        parent_widget = this->parentWidget();
        QPoint parent_point = parent_widget->pos();
        parent_point.setX(parent_point.x() + event->x() - press_point.x());
        parent_point.setY(parent_point.y() + event->y() - press_point.y());
        parent_widget->move(parent_point);
    }
}

void WindowsTitle::mouseReleaseEvent(QMouseEvent *)
{
    if(is_move)
        is_move = false;
}

void WindowsTitle::mouseDoubleClickEvent(QMouseEvent *)
{
    MaxWindowsSlot();
}

void WindowsTitle::MinWindowsSlot()
{
    parent_widget = this->parentWidget();
    emit MinWindowsSig();
}

void WindowsTitle::MaxWindowsSlot()
{
    parent_widget = this->parentWidget();
    parent_widget->setGeometry(QApplication::desktop()->availableGeometry());
}

void WindowsTitle::CloseWindowsSlot()
{
    parent_widget = this->parentWidget();
    parent_widget->close();
}

WindowsTitle::~WindowsTitle()
{
}
