#ifndef MIH_H
#define MIN_H

#include <vector>
#include <string>
#include <list>
#include "types.h"
using std::vector;
using std::string;
using std::list;
class Linscan;
class mihasher;
#define BITSET_LENGTH 64
#define HASH_SCAN_METHOD_LINEAR 0
#define HASH_SCAN_METHOD_MIH 1

typedef unsigned long long ullong;

class MIH
{
  public:
    explicit MIH(int b, int k, int d,float t, int method = HASH_SCAN_METHOD_LINEAR);
    
    ~MIH() { destory(); }
    void initLinscan(int b, int k, int d, int n = 0);
    bool appendCode(string& hash, int day = 0);
    bool appendCodes(vector<string> &codes);
    vector<string> getCandidates_linscan(vector<ullong> query_hash, bool isadd = false, int day = 0);
    vector<string> getCandidates_linscan(vector<float> &feature, bool isadd = false, int day = 0);
    vector<string> getCandidates(vector<float> &feature);
    void destory();
    

    // list<vector<ullong> > toHash(list<vector<float> > features);
    vector<ullong> toHash(vector<float> &feature);
    vector<ullong> toHash(string &code);
    std::pair<float,int> getSamePersons(list<vector<float>> features, vector<string> ids, vector<float> queryFeature);
    float cosineSimilarity(vector<float> vectorA, vector<float> vectorB);
    void setThreshold(float t)
    {
        threshold = t;
    }
    float getThreshold()
    {
        return threshold;
    }

  private:
    MIH(const MIH &mih) : bits(mih.bits) {}
    MIH &operator=(const MIH &) ;
    void initMIH_result();
    void rotateNewDayCodes();
    const int bits;
    int K;
    unsigned int B_over_8;
    int queryMehtod;
    float threshold;
    Linscan *linscan;
    mihasher *mihHasher;
    UINT8 *codes_db;
};

#endif //MIH_H