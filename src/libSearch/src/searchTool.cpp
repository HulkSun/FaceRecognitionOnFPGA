#include "searchTool.h"
#include "MIH.h"
#include "redisManager.h"
#include "linscan.h"
#include <stdexcept>
#include <bitset>
#include <cstdlib>
#include <sstream>
#include "boost/serialization/serialization.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include "boost/serialization/vector.hpp"
// #include <iostream>//only for debug
using namespace std;

#define PERSON_INFO_KEY "person_info_hash"
#define FACE_INFO_KEY "face_info_hash"
#define HASHCODE_SET_KEY "hash_codes_set"
#define FACE_FEATURE_KEY_MID_STRING "_face_feature_"
#define FACE_PATH_KEY_MID_STRING "_face_url_"
#define FACE_HASH_KEY_MID_STRING "_face_hashkey_"
#define COUNT_KEY "count"
#ifndef BITSET_LENGTH
#define BITSET_LENGTH 64
#endif

SearchTool::SearchTool(int b, const char *passwd, const char *host, int port)
    : person_info_key(PERSON_INFO_KEY), face_info_key(FACE_INFO_KEY),
      hash_info_key(HASHCODE_SET_KEY), bits(b), mih_query(NULL)
{
    redis = new RedisManager(host, port);
    if (passwd)
        password = string(passwd);
}

bool SearchTool::initSearchTool(int topk, int minDis, float threshold, int dbIndex, int queryMethod)
{
    K = topk;
    B_over_8 = bits / 8;
    if (redis == NULL)
        return false;
    bool success = redis->initRedisConnect(1000);

    if (!success)
        return false;
    if (!password.empty())
        success = redis->authPasswd(password.c_str());

    if (mih_query == NULL)
        mih_query = new MIH(bits, topk, minDis, threshold, queryMethod);
    selectDb(dbIndex);
    // append all hash code to MIH
    vector<string> codes = redis->set_getAll(HASHCODE_SET_KEY);
    if (!codes.empty())
    {
        if (codes[0].size() != (UINT32)B_over_8)
            throw runtime_error("hash codes size mismatch.");
        success = mih_query->appendCodes(codes);
    }
    return success;
}
bool SearchTool::selectDb(int dbIndex)
{
    return redis->selectDb(dbIndex);
}

SearchTool::~SearchTool()
{
    if (mih_query)
        delete mih_query;
    if (redis)
        delete redis;
}

// convert float[] to 01 hash codes string
string SearchTool::featureToBinaryString(vector<float> &feature)
{
    string res;
    for (auto i : feature)
        res += i > 0 ? "1" : "0";
    return res;
}

string SearchTool::featureToHashcode(vector<float> &feature)
{
    vector<ullong> res = mih_query->toHash(feature);
    char *p = (char *)&res[0];
    string hashcode(p, B_over_8);
    return hashcode;
}

// bin string to code
string SearchTool::binaryStringToCode(string &binString)
{
    int n = bits / BITSET_LENGTH;
    ullong *p = new ullong[n];
    for (int i = 0; i < n; ++i)
    {
        bitset<BITSET_LENGTH> bitvec(binString.substr(i * BITSET_LENGTH, BITSET_LENGTH));
        p[i] = bitvec.to_ullong();
    }
    string res((char *)p, B_over_8);
    delete[] p;

    return res;
}

// code to bin string
string SearchTool::codeToBinaryString(string &code)
{
    int n = bits / BITSET_LENGTH;
    string res;
    ullong *p = (ullong *)&code[0];
    for (int i = 0; i < n; ++i)
    {
        bitset<BITSET_LENGTH> bitvec(p[i]);
        res += bitvec.to_string();
    }
    return res;
}

// convert float[] to string
string SearchTool::serialize(vector<float> &feature)
{
    stringstream ss;
    boost::archive::binary_oarchive oa(ss);
    oa << feature;
    return ss.str();
}

// convert string to float[]
inline vector<float> SearchTool::deserialize(string &value)
{
    vector<float> res;
    stringstream ss(value);
    boost::archive::binary_iarchive ia(ss);
    ia >> res;
    return res;
}

inline string SearchTool::getFeatureKey(string id, string &count)
{
    return id + FACE_FEATURE_KEY_MID_STRING + count;
}

inline string SearchTool::getFaceUrlKey(string id, string &count)
{
    return id + FACE_PATH_KEY_MID_STRING + count;
}

inline string SearchTool::getFaceHashKey(string id, string &count)
{
    return id + FACE_HASH_KEY_MID_STRING + count;
}

inline string SearchTool::getFeatureKey(string id, int &count)
{
    return id + FACE_FEATURE_KEY_MID_STRING + to_string(count);
}

inline string SearchTool::getFaceUrlKey(string id, int &count)
{
    return id + FACE_PATH_KEY_MID_STRING + to_string(count);
}

inline string SearchTool::getFaceHashKey(string id, int &count)
{
    return id + FACE_HASH_KEY_MID_STRING + to_string(count);
}

inline string SearchTool::getPersonNameKey(string id)
{
    return id + "_name";
}

bool SearchTool::addPerson(PersonInfo &info)
{
    string key = info.id + "_id";
    if (!redis->hash_add(person_info_key.c_str(), key.c_str(), info.id.c_str()))
        return false;
    key = info.id + "_name";
    if (!redis->hash_add(person_info_key.c_str(), key.c_str(), info.name.c_str()))
        return false;
    key = info.id + "_sex";
    if (!redis->hash_add(person_info_key.c_str(), key.c_str(), info.sex.c_str()))
        return false;
    key = info.id + "_timestamp";
    if (!redis->hash_add(person_info_key.c_str(), key.c_str(), info.timestamp.c_str()))
        return false;
    return true;
}

bool SearchTool::deleteFeature(string &hashkey, string &facekey)
{
    // return redis->hash_del(hashkey.c_str(), facekey.c_str());
    return redis->hash_del(hashkey.c_str(), B_over_8, facekey.c_str(), facekey.size());
}

bool SearchTool::deleteFace(const string &id)
{
    string count_key = id + "_face_count";
    string count = redis->hash_get(face_info_key.c_str(), count_key.c_str());
    if (count.empty())
        return true;
    int face_count = stoi(count, nullptr, 10);
    // int face_count = atoi(count.c_str());
    if (face_count < 0)
    {
        redis->hash_increase(face_info_key.c_str(), count_key.c_str(), -face_count);
        return true;
    }

    string feature_hashcode_key, featureKey;
    stringstream ss;
    for (int i = 0; i < face_count; ++i)
    {
        feature_hashcode_key = redis->hash_get(face_info_key.c_str(), getFaceHashKey(id, i).c_str());
        featureKey = getFeatureKey(id, i);
        ss.clear();
        ss << getFaceUrlKey(id, i) << " " << getFaceHashKey(id, i);
        // delete from face_info
        redis->hash_dels(face_info_key.c_str(), ss.str().c_str());
        // delete hashkey
        deleteFeature(feature_hashcode_key, featureKey);
    }
    redis->hash_dels(face_info_key.c_str(), count_key.c_str());
    return true;
}

bool SearchTool::deletePerson(const string id)
{
    stringstream ss;
    ss << id << "_id " << id << "_name " << id << "_sex " << id << "_timestamp ";
    bool success = redis->hash_dels(person_info_key.c_str(), ss.str().c_str());
    // redis->hash_increase(person_info_key.c_str(), COUNT_KEY, -1);
    success = success && deleteFace(id);
    return success;
}

bool SearchTool::updatePersonInfo(PersonInfo &info)
{
    string key = info.id + "_id";
    if (!redis->hash_exist(person_info_key.c_str(), key.c_str()))
        return false;
    key = info.id + "_name " + info.name + " " + info.id + "_sex " + info.sex + " " + info.id + "_timestamp " + info.timestamp;
    if (!redis->hash_sets(person_info_key.c_str(), key.c_str()))
        return false;
    return true;
}

// 根据id查询
vector<string> SearchTool::queryPerson(const string &id)
{
    vector<string> res;
    if (id.empty())
        return res;
    string key = id + "_name " + id + "_sex " + id + "_timestamp";
    return redis->hash_gets(person_info_key.c_str(), key.c_str());
}

bool SearchTool::queryPerson(vector<float> feature, string &id, string &path, float &similarity)
{
    // clock_t start=clock();
    vector<string> hash_keys = mih_query->getCandidates(feature);
    if (hash_keys.empty())
        return false;
    
    vector<string> ids, face_key;
    list<vector<float>> features;
    size_t size = hash_keys.size() * 2;
    ids.reserve(size);
    face_key.reserve(size);
    // clock_t end1=clock();
    // #pragma omp parallel for
    for (uint i = 0; i < hash_keys.size(); ++i)
    {
        if (!hash_keys[i].empty())
        {
            vector<pair<string, string>> candidates = redis->hash_getAll(hash_keys[i].c_str(), B_over_8);
            for (auto key_value : candidates)
            {
                // #pragma omp critical
                {
                    face_key.push_back(key_value.first);
                    ids.push_back(key_value.first.substr(0, key_value.first.find("_")));
                    features.push_back(move(deserialize(key_value.second)));
                }
            }
        }
    }
    // clock_t end2=clock();
    auto res = mih_query->getSamePersons(features, ids, feature);
    // clock_t end3=clock();
    // cout<<"DEBUG:: "<<end1-start<<" "<<end2-end1<<" "<<end3-end2<<endl;
    if (res.second == -1)
        return false;
    
    id = ids[res.second];
    size_t index = face_key[res.second].find_last_of('_') + 1;
    string face_count = face_key[res.second].substr(index, face_key[res.second].size() - index);
    path = getFace(getFaceUrlKey(id, face_count));
    similarity = res.first;
    return true;
}

list<PersonInfo> SearchTool::getAllPersonInfo()
{
    // vector<pair<string, string>> res = redis->hash_getAll(person_info_key.c_str());
    map<string, string> res = redis->hash_getAllOrdered(person_info_key.c_str());
    list<PersonInfo> result_list;
    if (res.size() < 2)
        return result_list;
    PersonInfo info;
    // size_t nums = personCount();
    // for (size_t j = 0; j < nums; ++j)
    // {
    //     info.id = res[j * 4].second;
    //     info.name = res[j * 4 + 1].second;
    //     info.sex = res[j * 4 + 2].second;
    //     info.timestamp = res[j * 4 + 3].second;
    // }

    auto it = res.begin();
    for (size_t j = 0; j < res.size(); j+=4)
    {
        info.id = it->second;
        ++it;
        info.name = it->second;
        ++it;        
        info.sex = it->second;
        ++it;
        info.timestamp = it->second;
        ++it;
        result_list.push_back(info);
    }

    return result_list;
}

string SearchTool::getFace(const string &key)
{
    return redis->hash_get(face_info_key.c_str(), key.c_str());
}

vector<string> SearchTool::getFaces(const string &id)
{
    vector<string> res;
    if (id.empty())
        return res;
    string key = id + "_face_count";
    string value = redis->hash_get(face_info_key.c_str(), key.c_str());
    if (value.empty())
        return res;
    // int count = atoi(value.c_str());
    int count = stoi(value, nullptr, 10);
    key.clear();
    stringstream ss;
    for (int i = 0; i < count; ++i)
    {
        ss << getFaceUrlKey(id, i) << " ";
    }
    return redis->hash_gets(face_info_key.c_str(), ss.str().c_str());
}

bool SearchTool::addFace(FaceImageInfo &info)
{
    if (info.feature.empty())
        return false;
    // if id dosen't exist return false
    if (!redis->hash_exist(person_info_key.c_str(), getPersonNameKey(info.id).c_str()))
        return false;

    string hash_key = featureToHashcode(info.feature);
    if (!redis->set_IsMember(hash_info_key.c_str(), hash_info_key.size(), hash_key.c_str(), B_over_8))
    {
        redis->set_add(hash_info_key.c_str(), hash_info_key.size(), hash_key.c_str(), B_over_8);
        mih_query->appendCode(hash_key);
    }

    string count_key = info.id + "_face_count";
    string count = redis->hash_get(face_info_key.c_str(), count_key.c_str());
    if (count.empty())
    {
        count = "0";
        redis->hash_set(face_info_key.c_str(), count_key.c_str(), count.c_str());
    }

    // add to face_info
    string key = getFaceUrlKey(info.id, count);
    if (!redis->hash_set(face_info_key.c_str(), key.c_str(), info.path.c_str()))
        return false;

    key = getFaceHashKey(info.id, count);
    if (!redis->hash_set(face_info_key.c_str(), face_info_key.size(), key.c_str(), key.size(), hash_key.c_str(), B_over_8))
        return false;

    redis->hash_increase(face_info_key.c_str(), count_key.c_str(), 1);
    // add to hash_key
    string feat = serialize(info.feature);
    key = getFeatureKey(info.id, count);
    redis->hash_set(hash_key.c_str(), B_over_8, key.c_str(), key.size(), feat.c_str(), feat.size());

    return true;
}

size_t SearchTool::personCount()
{
    return (redis->hash_len(person_info_key.c_str())) / 4;
}
bool SearchTool::addFace(list<FaceImageInfo> &infoList)
{
    for (auto info : infoList)
        if (!addFace(info))
            return false;
    return true;
}

void SearchTool::setThreshold(float threshold)
{
    mih_query->setThreshold(threshold);
}
float SearchTool::getThreshold()
{
    return mih_query->getThreshold();
}
