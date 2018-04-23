#include "MIH.h"

#include <bitset>
#include <list>
#include <map>

#include <cstdlib>
#include <cstring>
#include <cmath>

#include "types.h"
#include "linscan.h"
#include "mihasher.h"
// #include <iostream>//only for debug
using namespace std;
MIH::MIH(int b, int k, int d, float t, int method) : bits(b), K(k), B_over_8(bits / 8), threshold(t),mihHasher(NULL), linscan(NULL),codes_db(NULL)
{
    queryMehtod = method;
    if(method==HASH_SCAN_METHOD_MIH)
    {
        //1亿 64bit m=2, 千万 64bit m=3，百万 64bit m=4
        mihHasher = new mihasher(b, 2);
        mihHasher->setK(k);
        initMIH_result();
    }   
    else
        linscan = new Linscan(b, k, d, 0);

}

void MIH::initMIH_result()
{//初始化保存查询结果的变量

}
void MIH::destory()
{
    if (linscan != NULL)
    {
        delete linscan;
        linscan = NULL;
    }
    if (mihHasher != NULL)
    {
        delete mihHasher;
        mihHasher = NULL;
    }
    if (codes_db != NULL)
    {
        delete codes_db;
        codes_db = NULL;
    }
}

//linscan add codes
bool MIH::appendCode(string &code, int day)
{
    // only appendcode with linscan
    if(queryMehtod==HASH_SCAN_METHOD_MIH) return true;
    UINT8 *u_hash = (UINT8 *)malloc(sizeof(UINT8) * B_over_8);
    if (u_hash == NULL)
        return false;
    memcpy(u_hash, code.data(), code.size());
    linscan->add_codes(u_hash, day);

    return true;
}
bool MIH::appendCodes(vector<string> &codes)
{
    if(queryMehtod==HASH_SCAN_METHOD_MIH)
    {
        static bool is_first=true;
        if(is_first)
        {
            codes_db=new UINT8[codes.size()*bits];
            UINT8 * p=codes_db;
            for (auto code : codes)
            {
                memcpy(p, code.data(), code.size());
                p+=code.size();
            }
            mihHasher->populate(codes_db, codes.size(), B_over_8);
            is_first=false;
        }
    }
    else
    {
        for (auto code : codes)
        {  
            if (!appendCode(code))
                return false;
        }
    }
    return true;
}

vector<string> MIH::getCandidates_linscan(vector<ullong> query_hash, bool isadd, int day)
{
    vector<string> result;
    //std::ofstream out("linscan_interface.log");
    UINT8 *u_hash = (UINT8 *)malloc(sizeof(UINT8) * B_over_8);

    if (u_hash == NULL)
        return result;

    memcpy(u_hash, &query_hash[0], query_hash.size() * 8);

    UINT8 **candies = (UINT8 **)malloc(sizeof(UINT8 *) * K);

    if (candies == NULL)
        return result;

    linscan->query(u_hash, candies, day);
    if (isadd)
        linscan->add_codes(u_hash, day);
    else
        free(u_hash);

    for (int i = 0; i < K && candies[i] != NULL; i++)
    {
        result.push_back(string((char *)candies[i], B_over_8));
    }
    free(candies);

    //out.close();
    return result;
}
vector<string> MIH::getCandidates_linscan(vector<float> &feature, bool isadd, int day)
{
    return getCandidates_linscan(toHash(feature), isadd, day);
}
vector<string> MIH::getCandidates(vector<float> &feature)
{
    if(queryMehtod==HASH_SCAN_METHOD_MIH)
    {
        // clock_t start=clock();
        vector<string> result;
        result.reserve(K);
        vector<ullong> code=toHash(feature);
        qstat stats;
        //使用mih查询
        UINT32 _codes_seq[K];
        UINT32 nres[bits+1];
        // clock_t end1=clock();
        // cout<<"DEBUG::start mih query"<<endl;
        mihHasher->batchquery(_codes_seq, nres, &stats, (UINT8*)code.data(), 1, B_over_8);
        // cout<<"DEBUG::mih query finished"<<endl;
        // clock_t end2=clock();
        UINT8 *p = codes_db;
        for (int i = 0; i < K && p != NULL; i++)
        {
            p=codes_db+(_codes_seq[i]-1)*B_over_8;
            result.push_back(string((char *)p, B_over_8));
        }
        // clock_t end3=clock();
        // cout<<"DEBUG:: init "<<end1-start<<" batchquery "<<end2-end1<<" result "<<end3-end2<<endl;
        // cout<<"DEBUG::return query result"<<endl;
        return result;
    }
    else
        return getCandidates_linscan(feature);
}

void MIH::rotateNewDayCodes()
{
    linscan->move_codes(1, 0, linscan->codes_size() - 1);
    linscan->clear_codes(0);
}

// list<vector<ullong> > MIH::toHash(list<vector<float> > features)
// {
//     List<string> result = new ArrayList<string>();
//         for(int i = 0; i < features.size(); i++){
//             float[] feature = features.get(i);
//             string hash = toHash(feature);
//             result.add(hash);
//         }
//         return result;
// }

vector<ullong> MIH::toHash(vector<float> &feature)
{
    vector<ullong> res;
    if (feature.empty())
        return res;
    if (feature.size() != (size_t)bits)
        throw runtime_error("MIH::toHash Invalid parameter.");

    bitset<BITSET_LENGTH> bitvec;
    int j = 0;
    for (size_t i = 0; i < feature.size(); ++i)
    {
        bitvec[j] = feature[i] > 0 ? 1 : 0;
        ++j;
        if (j == BITSET_LENGTH)
        {
            res.push_back(bitvec.to_ullong());
            j = 0;
        }
    }
    return res;
}

vector<ullong> MIH::toHash(string &code)
{
    vector<ullong> res;
    if (code.empty())
        return res;
    if (code.size() != (size_t)B_over_8)
        throw runtime_error("MIH::toHash Invalid parameter.");
    size_t j = 0;
    for (int i = 0; i < bits; i += BITSET_LENGTH)
    {
        bitset<BITSET_LENGTH> bitvec(code.substr(j, BITSET_LENGTH));
        j += BITSET_LENGTH;
        res.push_back(bitvec.to_ullong());
    }

    return res;
}

// pair<float,int> int==-1 not find
pair<float, int> MIH::getSamePersons(list<vector<float>> features, vector<string> ids, vector<float> queryFeature)
{
    if (features.size() <= 0 || ids.size() <= 0 || features.size() != ids.size())
        return pair<float, int>(0, -1);

    // vote scores
    map<string, float> scoreMap;
    map<string, pair<float, int>> bestMatch;
    auto it = features.begin();
    for (size_t i = 0; i < features.size(); i++)
    {
        float res = cosineSimilarity(*it, queryFeature) - threshold;
        ++it;
        if (res > 0)
        {
            scoreMap[ids[i]] += 0.7f * res;
        }
        else
        {
            scoreMap[ids[i]] += 0.3f * res;
        }
        if (res > bestMatch[ids[i]].first)
        {
            bestMatch[ids[i]].first = res;
            bestMatch[ids[i]].second = i;
        }
    }
    float maxSimilarity = 0.0f;
    string id;
    for (auto entry : scoreMap)
    {
        if (entry.second > maxSimilarity)
        {
            id = entry.first;
            maxSimilarity = entry.second;
        }
    }
    if (!id.empty())
    {
        bestMatch[id].first += threshold;
        return bestMatch[id];
    }
    return pair<float, int>(0, -1);
}

float MIH::cosineSimilarity(vector<float> vectorA, vector<float> vectorB)
{
    float dotProduct = 0.0f;
    float normA = 0.0f;
    float normB = 0.0f;
    for (unsigned int i = 0; i < vectorA.size(); i++)
    {
        dotProduct += vectorA[i] * vectorB[i];
        normA += vectorA[i] * vectorA[i];
        normB += vectorB[i] * vectorB[i];
    }
    return (float)(0.5 + 0.5 * (dotProduct / (sqrt(normA) * sqrt(normB))));
}
