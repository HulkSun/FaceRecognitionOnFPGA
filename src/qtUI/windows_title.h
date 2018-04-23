#ifndef WINDOWS_TITLE_H
#define WINDOWS_TITLE_H

#include <QMouseEvent>
#include "global.h"
class WindowsTitle : public QWidget
{
    Q_OBJECT

public:
    explicit WindowsTitle(QWidget *parent = 0);
    ~WindowsTitle();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);

private:
    QLabel *logo_label;
    QLabel *title_label;
    QPushButton *min_btn;
    QPushButton *max_btn;
    QPushButton *close_btn;
    QWidget *parent_widget;

    QPoint press_point;
    bool is_move;

signals:
    void MinWindowsSig();
//    void MaxWindows();
//    void CloseWindows();

private slots:
    void MinWindowsSlot();
    void MaxWindowsSlot();
    void CloseWindowsSlot();
};

#endif
