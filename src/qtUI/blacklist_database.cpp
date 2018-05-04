#include "blacklist_database.h"
#include <iostream>

BlackListDataBase::BlackListDataBase(int dim, const char *passwd)
{
    search_tool = new SearchTool(dim, passwd);
}

bool BlackListDataBase::InitBlackList(int topK, int minDis, float threshold, int dbIndex, int queryMethod)
{
    QTime tim;
    tim.start();
    if (!search_tool->initSearchTool(topK, minDis, threshold, dbIndex, queryMethod))
    {
        qDebug() << "search tool init fail";
        return false;
    }
    std::list<PersonInfo> allPersonInfo = search_tool->getAllPersonInfo();
    allPersonInfo.sort([](const PersonInfo &p1, const PersonInfo &p2) -> bool { return atoi(p1.id.c_str()) < atoi(p2.id.c_str()); });
    for (std::list<PersonInfo>::iterator it = allPersonInfo.begin(); it != allPersonInfo.end(); ++it)
    {
        PersonInfo info = *it;
        QPersonInfo qinfo = PersonToQPerson(info);
        emit SendPersonInfoSig(qinfo);
    }
    qDebug() << "getAllPersonInfo takes:" << tim.elapsed() / 1000.0 << "s.";
    return true;
}

PersonInfo BlackListDataBase::QPersonToPerson(QPersonInfo &qinfo)
{
    PersonInfo info;
    info.id = qinfo.id.toStdString();
    info.name = qinfo.name.toStdString();
    info.sex = qinfo.sex.toStdString();
    info.timestamp = qinfo.timestamp.toString("yyyy-MM-dd@hh:mm:ss").toStdString();
    return info;
}

QPersonInfo BlackListDataBase::PersonToQPerson(PersonInfo &info)
{
    QPersonInfo qinfo;
    qinfo.id = QString::fromStdString(info.id);
    qinfo.name = QString::fromStdString(info.name);
    qinfo.sex = QString::fromStdString(info.sex);
    qinfo.timestamp = QDateTime::fromString(QString::fromStdString(info.timestamp), "yyyy-MM-dd@hh:mm:ss");
    return qinfo;
}

FaceImageInfo BlackListDataBase::QFaceToFace(QFaceImageInfo &qfaceInfo)
{
    FaceImageInfo faceInfo;
    faceInfo.id = qfaceInfo.id.toStdString();
    faceInfo.path = qfaceInfo.path.toStdString();
    faceInfo.feature = qfaceInfo.feature;
    return faceInfo;
}

QFaceImageInfo BlackListDataBase::FaceToQFace(FaceImageInfo &faceInfo)
{
    QFaceImageInfo qfaceInfo;
    qfaceInfo.id = QString::fromStdString(faceInfo.id);
    qfaceInfo.path = QString::fromStdString(faceInfo.path);
    qfaceInfo.feature = faceInfo.feature;
    return qfaceInfo;
}

/* slot function */
bool BlackListDataBase::QueryPersonSlot(QString &qid)
{
    std::string id = qid.toStdString();
    std::vector<std::string> queryResult = search_tool->queryPerson(id);
    if (queryResult.size() == 0)
    {
        qDebug() << "查询失败，或数据库中没有 id =" << qid << "的人员！";
        return false;
    }
    QPersonInfo qinfo;
    for (size_t i = 0; i < queryResult.size();)
    {
        qinfo.id = qid;
        qinfo.name = QString::fromStdString(queryResult[i++]);
        qinfo.sex = QString::fromStdString(queryResult[i++]);
        qinfo.timestamp = QDateTime::fromString(QString::fromStdString(queryResult[i++]), "yyyy-MM-dd@hh:mm:ss");
        emit SendQueryResultSig(qinfo);
    }
    return true;
}

bool BlackListDataBase::QueryPersonSlot(std::vector<float> feature, std::string &id, std::string &path, float sim)
{
    if (!search_tool->queryPerson(feature, id, path, sim))
    {
        qDebug() << "特征查询失败！";
        return false;
    }
    return true;
}

bool BlackListDataBase::AddPersonSlot(QPersonInfo &qinfo)
{
    PersonInfo info = QPersonToPerson(qinfo);
    if (!search_tool->addPerson(info))
    {
        qDebug() << "添加人员信息失败！";
        return false;
    }
    return true;
}

bool BlackListDataBase::DeletePersonSlot(QString &qid)
{
    const std::string id = qid.toStdString();
    if (!search_tool->deletePerson(id))
    {
        qDebug() << "删除人员信息失败！";
        return false;
    }
    return true;
}

bool BlackListDataBase::UpdatePersonInfoSlot(QPersonInfo &qinfo)
{
    PersonInfo info = QPersonToPerson(qinfo);
    if (!search_tool->updatePersonInfo(info))
    {
        qDebug() << "更新人员信息失败！";
        return false;
    }
    return true;
}

bool BlackListDataBase::AddFaceSlot(QFaceImageInfo &qfaceInfo)
{
    FaceImageInfo faceInfo = QFaceToFace(qfaceInfo);
    if (!search_tool->addFace(faceInfo))
    {
        qDebug() << "id =" << qfaceInfo.id << "添加照片" << qfaceInfo.path << "失败！";
        return false;
    }
    return true;
}

bool BlackListDataBase::AddFaceSlot(std::list<QFaceImageInfo> &qfaceInfoList)
{
    std::list<FaceImageInfo> faceInfoList;
    FaceImageInfo faceInfo;
    for (std::list<QFaceImageInfo>::iterator it = qfaceInfoList.begin(); it != qfaceInfoList.end(); ++it)
    {
        QFaceImageInfo qfaceInfo = *it;
        faceInfo = QFaceToFace(qfaceInfo);
        faceInfoList.push_back(faceInfo);
    }
    if (!search_tool->addFace(faceInfoList))
    {
        qDebug() << "添加照片信息失败！";
        return false;
    }
    return true;
}

bool BlackListDataBase::GetFacesSlot(QString &qid)
{
    const std::string id = qid.toStdString();
    std::vector<std::string> facesUrl = search_tool->getFaces(id);
    emit SendFacesUrl(facesUrl);
    return true;
}

bool BlackListDataBase::AddPersonAndFaceSlot(QPersonInfo qinfo, std::vector<QFaceImageInfo> qfaceInfoVec)
{
    if (!AddPersonSlot(qinfo))
        return false;
    for (size_t i = 0; i != qfaceInfoVec.size(); ++i)
    {
        QFaceImageInfo qfaceInfo = qfaceInfoVec[i];
        if (!AddFaceSlot(qfaceInfo))
            return false;
    }
    return true;
}

/* 直接调用，查询单个人脸的信息 */
SearchPersonInfo BlackListDataBase::QueryPersonByFeature(std::vector<float> feature)
{
    SearchPersonInfo personInfo;
    std::string id, path;
    float sim;
    if (!search_tool->queryPerson(feature, id, path, sim))
    {
        personInfo.id = "";
        personInfo.name = ""; // 查询失败
        personInfo.sex = "";
        personInfo.path = "";
        personInfo.sim = -1;
        return personInfo;
    }
    std::vector<std::string> queryResult = search_tool->queryPerson(id); // name sex timestamp
    personInfo.id = id;
    personInfo.name = queryResult[0];
    personInfo.sex = queryResult[1];
    personInfo.path = path;
    personInfo.sim = sim;
    return personInfo;
}

size_t BlackListDataBase::PersonCount()
{
    return search_tool->personCount();
}

BlackListDataBase::~BlackListDataBase()
{
}
