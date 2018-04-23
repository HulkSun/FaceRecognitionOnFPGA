#ifndef BLACKLIST_WIDGET_H
#define BLACKLIST_WIDGET_H

#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QStackedWidget>
#include <QMessageBox>
#include <QRadioButton>
#include <QToolTip>
#include "mtcnn.h"
#include "center_face.h"
#include "my_qlabel.h"
#include "global.h"

#define Add 0
#define Delete 1
#define Update 2
#define Query 3

extern MTCNN *MTCNNDetector;
extern CenterFace *CenterExtractor;
extern cv::PCA pca;                 // pca

class BlackListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BlackListWidget(QWidget *parent = 0);
    ~BlackListWidget();
    void GetDataBase();

    bool QueryPersonById(QString &id);
    bool AddNewPerson(int row);   // 添加一个新人
    bool ShowExistPerson(int row, QString name, QString sex, QString id, QString changeTime);   // 显示一个数据库已有人的信息在table中
    bool DeleteOnePerson(int row);   // 删除row行对应人的信息
    bool CheckPerson(int row);
    bool UploadImage();
    bool IsReadOnly(bool);
    bool SaveAdd(int row);
    bool SaveModify(int row);
    bool CancelModify(int row);

    bool InitInfoGroup();

protected:
  void paintEvent(QPaintEvent *);

private:
    /* blacklist config widget */
    QLineEdit *query_edit;
    QPushButton *query_button;
    QPushButton *add_button;
//    QPushButton *delete_button;
    QPushButton *return_button;
    QTableWidget *blacklist_table;
    QTableWidget *query_result_table;
    QStackedWidget *stacked_table;

    /* blacklist person widget */
    MyQLabel *picture_label;
    QLabel *name_label;
    QLabel *sex_label;
    QLabel *id_label;
    QLineEdit *name_edit;
    QLineEdit *id_edit;
//    QComboBox *sex_box;
    QRadioButton *male_radio;
    QRadioButton *female_radio;
    QPushButton *upload_button;
    QPushButton *modify_button;
    QPushButton *save_button;
    QPushButton *cancel_button;
    QTableWidget *picture_table;
    QGroupBox *person_group;
    int current_row;
    int operate_type;
signals:
    void AddPersonSig(QPersonInfo &qinfo);
    void DeletePersonSig(QString &qid);
    void UpdatePersonSig(QPersonInfo &qinfo);
    void QueryPersonSig(QString &qid);
    void AddFaceSig(QFaceImageInfo &qfaceInfo);
    void GetFacesSig(QString &qid);
    void AddPersonAndFaceSig(QPersonInfo qinfo, std::vector<QFaceImageInfo> qfaceInfoVec);

private slots:
    void SearchPersonSlot();
    void AddNewPersonSlot();
    void DeletePersonSlot();
    void ReturnTableSlot();
    void CheckPersonSlot(QTableWidgetItem*);
    void CheckQueryPersonSlot(QTableWidgetItem*);

    void UploadPictureSlot();
    void ModifyInfoSlot();
    void SaveSlot();
    void CancelSlot();

    void ShowPersonSlot(QPersonInfo &qinfo);
    void GetQueryResultSlot(QPersonInfo &qinfo);
    void ShowFacesSlot(std::vector<std::string>);

    void ShowItemSlot(QModelIndex);
};

#endif // BLACKLIST_WIDGET_H
