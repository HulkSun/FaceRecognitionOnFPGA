#ifndef SEARCH_TOOL_H
#define SEARCH_TOOL_H

#include <string>
#include <list>
#include <vector>

using std::string;
 
#define SEARCH_TOOL_VERSION_MAJOR    0
#define SEARCH_TOOL_VERSION_MINOR    2
#define SEARCH_TOOL_VERSION_RELEASE  2
#ifndef MIH_H
#define HASH_SCAN_METHOD_LINEAR 0
#define HASH_SCAN_METHOD_MIH 1
#endif
class MIH;
class RedisManager;
struct PersonInfo
{
    string id;
    string name;
    string sex;
    string timestamp;
};

struct FaceImageInfo
{
    string id;
    std::vector<float> feature;
    string path;
};

class SearchTool
{
  private:
    //bool addFeature(string hashkey, std::vector<float> feature);
    bool deleteFace(const string &id); // delete id's all face
    bool deleteFeature(string &hashkey, string &facekey);

    SearchTool(const SearchTool &s) : bits(s.bits) {}
    SearchTool &operator=(const SearchTool &);

    // hash key : count person_0:id id_name:name id_sex id_timestamp:timestamp...
    const string person_info_key;
    // hash key : id_face_count:face_counts id_face_url_0:path id_face_hashkey_0:hashkey...
    const string face_info_key;
    // set key : id
    const string hash_info_key;
    string password;
    const int bits;
    int K; //top key
    int B_over_8;
    int m;//mih m segment
    RedisManager *redis;
    MIH *mih_query;

  public:
    explicit SearchTool(int b, const char *passwd="", const char *host = "127.0.0.1", int port = 6379);
    ~SearchTool();
    string serialize(std::vector<float> &);
    string featureToBinaryString(std::vector<float> &);
    string featureToHashcode(std::vector<float> &);
    std::vector<float> deserialize(string &value);
    string binaryStringToCode(string &binString);
    string codeToBinaryString(string &binString);

    string getFeatureKey(string id, string &count);
    string getFaceUrlKey(string id, string &count);
    string getFaceHashKey(string id, string &count);
    string getFeatureKey(string id, int &count);
    string getFaceUrlKey(string id, int &count);
    string getFaceHashKey(string id, int &count);
    string getPersonNameKey(string id);

    // interface 
    bool initSearchTool(int topk = 100, int minDis = 80, float threshold = 0.75,int dbIndex=0,int queryMethod=0);
    bool selectDb(int dbIndex = 0);
    bool addPerson(PersonInfo &info);
    bool deletePerson(const string id);
    bool updatePersonInfo(PersonInfo &info);
    std::vector<string> queryPerson(const string &id);
    bool queryPerson(std::vector<float> feature, string &id, string &path,float &similarity);
    std::list<PersonInfo> getAllPersonInfo();
    string getFace(const string &key);
    std::vector<string> getFaces(const string &id);
    bool addFace(FaceImageInfo &info);
    bool addFace(std::list<FaceImageInfo> &infoList);
    size_t personCount();
    void setThreshold(float threshold);
    float getThreshold();
};

#endif //SEARCH_TOOL_H
