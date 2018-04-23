#ifndef BLACKLIST_DATABASE_H
#define BLACKLIST_DATABASE_H
#include <QTime>
#include "global.h"
#include "center_face.h"
#include "search_tool.h"
class BlackListDataBase : public QObject
{
    Q_OBJECT
public:
    explicit BlackListDataBase(int dim, const char *passwd);
    ~BlackListDataBase();
    bool InitBlackList(int topK, int minDis, float threshold, int dbIndex, int queryMethod);
    PersonInfo QPersonToPerson(QPersonInfo &qinfo);
    QPersonInfo PersonToQPerson(PersonInfo &info);
    FaceImageInfo QFaceToFace(QFaceImageInfo &qfaceInfo);
    QFaceImageInfo FaceToQFace(FaceImageInfo &faceInfo);
    SearchPersonInfo QueryPersonByFeature(std::vector<float>);
    size_t PersonCount();

private:
    SearchTool *search_tool;

signals:
    void SendPersonInfoSig(QPersonInfo &qinfo);
    void SendQueryResultSig(QPersonInfo &qinfo);
    void SendFacesUrl(std::vector<std::string>);

public slots:
    bool QueryPersonSlot(QString &qid);
    bool QueryPersonSlot(std::vector<float> feature, std::string &id, std::string &path, float sim);
    bool AddPersonSlot(QPersonInfo &qinfo);
    bool DeletePersonSlot(QString &qid);
    bool UpdatePersonInfoSlot(QPersonInfo &qinfo);

    bool AddFaceSlot(QFaceImageInfo &qfaceInfo);
    bool AddFaceSlot(std::list<QFaceImageInfo> &qfaceInfoList);
    bool GetFacesSlot(QString &qid);

    bool AddPersonAndFaceSlot(QPersonInfo qinfo, std::vector<QFaceImageInfo> qfaceInfoVec);
};

#endif // BLACKLIST_DATABASE_H
