#include <iostream>
#include <fstream>
#include <omp.h>
#include <time.h>
#include <stdlib.h>
#include <stdexcept>
#include "searchTool.h"

using namespace std;
typedef unsigned char uint8_t;
typedef unsigned int uint;

void readJegouHeader(const char *path, uint &n, uint &d);
uint8_t *readBatchJegou(const char *path, uint start_pos, uint num);
double getMeanValue(const char *path, int num = 0);
vector<vector<float>> getFeature128FromSIFT1B(const char *path, uint start_pos, uint num, float mean = 26.4);
vector<vector<float>> getFeature64FromSIFT1B(const char *path, uint start_pos, uint num, float mean = 26.4);
int main(int argc, char **argv)
{
    if (argc < 4)
    {
        cout << "Usage:" << endl
             << "addCodesToRedis [bvecs file] [add n*100000 feature to redis] [redis db number]" << endl;
        cout << "example: \n addCodesToRedis bigann_base.bvecs 10000 2." << endl;
        return -1;
    }
    
    char *bvecs = argv[1];
    int num = atoi(argv[2]);
    int dbIndex = atoi(argv[3]);
    int batch = 10000;
    uint n1 = 0;
    uint d1 = 0;
    readJegouHeader(bvecs, n1, d1);
    cout << "path:" << bvecs << "| num: " << num * batch << " | dbindex: " << dbIndex << endl;
    //getMeanValue(bvecs,5000000);
    // 添加64维的特征向量
    const int dim = 64;
    SearchTool search(dim, "redispassword");
    if (!search.initSearchTool(100, 50, 0.7,dbIndex, HASH_SCAN_METHOD_MIH))
    {
        cout << "searchTool init fail." << endl;
        return -1;
    }
    cout << "select db " << dbIndex << " : " << (search.selectDb(dbIndex) ? "true" : "false") << endl;
    cout << "searchTool init ok." << endl;
    PersonInfo person = {"0", "none", "none", "2017-06-14"};
    search.addPerson(person);

    int length = batch;

    clock_t start, end;
    start = clock();
    // #pragma omp parallel for
    for (int i = 0; i < num; ++i)
    {
        cout << "process " << i * batch << endl;
        if (i * batch + batch > n1)
            length = n1 - i;
        vector<vector<float>> features;
        if(dim==128)
            features = getFeature128FromSIFT1B(bvecs, i * batch, length);
        else features = getFeature64FromSIFT1B(bvecs, i * batch, length);
        //#pragma omp parallel for
        for (uint j = 0; j < features.size(); ++j)
        {
            FaceImageInfo images;
            images.id = person.id;
            images.path = "none";
            images.feature = features[j];
            if (!search.addFace(images))
                cout << "addFace fail." << endl;
        }
    }
    end = clock();
    cout << "time used: " << (end - start) / CLOCKS_PER_SEC << " seconds" << endl;
    cout << "add " << num << " faces to id 0" << endl;
    return 0;
}

double getMeanValue(const char *path, uint num)
{
    uint n1 = 0;
    uint d1 = 0;
    readJegouHeader(path, n1, d1);

    cout << "header dim " << d1 << endl;
    cout << "header num " << n1 << endl;
    cout << "filesize   " << (sizeof(uint8_t) * n1 * d1) / 100000.0 << " mb " << endl;
    uint featureUse = n1;
    if (num > 0 && num <= n1)
        featureUse = num;
    long long sum = 0;
    int batch = 1000000;

    clock_t start = clock();
#pragma omp parallel for reduction(+ : sum)
    for (uint i = 0; i < featureUse; i += batch)
    {
        uint end = (i + batch > n1) ? n1 : i + batch;
        uint length = end - i;
        uint8_t *data = readBatchJegou(path, i, length);
        uint batchLen = length * d1;
        cout << "process " << i << endl;
#pragma omp parallel for reduction(+ : sum)
        for (uint j = 0; j < batchLen; ++j)
        {
            // cout << (int)data[j] << " ";
            sum += (int)data[j];
        }
        delete[] data;
    }
    clock_t end = clock();
    cout << "time used: " << (end - start) / CLOCKS_PER_SEC << " seconds" << endl;
    cout << "sum: " << sum << " avg: " << sum / (128.0 * featureUse) << endl;
    return sum / (128.0 * featureUse);
}

void readJegouHeader(const char *path, uint &n, uint &d)
{
    // rewrite of http://corpus-texmex.irisa.fr/fvecs_read.m
    // read little endian

    // read binary from file
    std::ifstream in(path, std::ios_base::in | std::ios_base::binary);
    std::string pp = path;
    if (!in.good())
    {
        in.close();
        throw std::runtime_error("Dataset file " + pp + " does not exists");
    }

    // // get dimension of vector
    int dimension = 0;
    in.read(reinterpret_cast<char *>(&dimension), sizeof(int));

    uint vecsizeof = 4 + dimension;

    // // get size of file
    in.seekg(0, std::ios::beg);
    std::streampos fsize = in.tellg();
    in.seekg(0, std::ios::end);
    fsize = in.tellg() - fsize;

    uint number_of_vectors = fsize / vecsizeof;
    in.seekg(0, std::ios::beg);

    n = number_of_vectors;
    d = dimension;
}

uint8_t *readBatchJegou(const char *path, uint start_pos, uint num)
{
    // rewrite of http://corpus-texmex.irisa.fr/fvecs_read.m
    // read little endian

    // read binary from file
    std::ifstream in(path, std::ios_base::in | std::ios_base::binary);
    std::string pp = path;
    if (!in.good())
    {
        in.close();
        throw std::runtime_error("Dataset file " + pp + " does not exists");
    }

    // // get dimension of vector
    int dimension = 0;
    in.read(reinterpret_cast<char *>(&dimension), sizeof(int));

    // // get size of file
    in.seekg(0, std::ios::beg);
    std::streampos fsize = in.tellg();
    in.seekg(0, std::ios::end);
    fsize = in.tellg() - fsize;

    in.seekg(0, std::ios::beg);

    unsigned char temp[sizeof(uint8_t)];

    uint8_t *ptr2 = new uint8_t[num * dimension];
    uint pos = 0;
    for (uint j = start_pos; j < start_pos + num; ++j)
    {
        in.seekg((j) * sizeof(uint8_t) * 132 + sizeof(uint8_t) * 4);
        // adjust offset
        //in.read(reinterpret_cast<char *>(temp), sizeof(uint8_t));
        // read each coordinate
        for (int i = 0; i < dimension; ++i)
        {
            in.read(reinterpret_cast<char *>(temp), sizeof(uint8_t));
            ptr2[pos] = reinterpret_cast<uint8_t &>(temp);
            pos++;
        }
    }

    in.close();
    return ptr2;
}

vector<vector<float>> getFeature64FromSIFT1B(const char *path, uint start_pos, uint num, float mean)
{
    // rewrite of http://corpus-texmex.irisa.fr/fvecs_read.m
    // read little endian

    // read binary from file
    std::ifstream in(path, std::ios_base::in | std::ios_base::binary);
    std::string pp = path;
    if (!in.good())
    {
        in.close();
        throw std::runtime_error("Dataset file " + pp + " does not exists");
    }

    // // get dimension of vector
    int dimension = 0;
    in.read(reinterpret_cast<char *>(&dimension), sizeof(int));

    // // get size of file
    in.seekg(0, std::ios::beg);
    std::streampos fsize = in.tellg();
    in.seekg(0, std::ios::end);
    fsize = in.tellg() - fsize;

    in.seekg(0, std::ios::beg);

    unsigned char temp[sizeof(uint8_t)];
    vector<vector<float>> result(num*2, vector<float>(64, 0));
    uint row = 0;
    for (uint j = start_pos; j < start_pos + num; ++j)
    {
        in.seekg((j) * sizeof(uint8_t) * 132 + sizeof(uint8_t) * 4);
        // adjust offset read each coordinate
        for (int i = 0; i < 64; ++i)
        {
            in.read(reinterpret_cast<char *>(temp), sizeof(uint8_t));
            result[row][i] = reinterpret_cast<uint8_t &>(temp) - mean;
        }
        ++row;
        for (int i = 0; i < 64; ++i)
        {
            in.read(reinterpret_cast<char *>(temp), sizeof(uint8_t));
            result[row][i] = reinterpret_cast<uint8_t &>(temp) - mean;
        }
        ++row;
    }

    in.close();
    return result;
}

vector<vector<float>> getFeature128FromSIFT1B(const char *path, uint start_pos, uint num, float mean)
{
    // rewrite of http://corpus-texmex.irisa.fr/fvecs_read.m
    // read little endian

    // read binary from file
    std::ifstream in(path, std::ios_base::in | std::ios_base::binary);
    std::string pp = path;
    if (!in.good())
    {
        in.close();
        throw std::runtime_error("Dataset file " + pp + " does not exists");
    }

    // // get dimension of vector
    int dimension = 0;
    in.read(reinterpret_cast<char *>(&dimension), sizeof(int));

    // // get size of file
    in.seekg(0, std::ios::beg);
    std::streampos fsize = in.tellg();
    in.seekg(0, std::ios::end);
    fsize = in.tellg() - fsize;

    in.seekg(0, std::ios::beg);

    unsigned char temp[sizeof(uint8_t)];
    vector<vector<float>> result(num, vector<float>(dimension, 0));
    uint row = 0;
    for (uint j = start_pos; j < start_pos + num; ++j)
    {
        in.seekg((j) * sizeof(uint8_t) * 132 + sizeof(uint8_t) * 4);
        // adjust offset read each coordinate
        for (int i = 0; i < dimension; ++i)
        {
            in.read(reinterpret_cast<char *>(temp), sizeof(uint8_t));
            result[row][i] = reinterpret_cast<uint8_t &>(temp) - mean;
        }
        ++row;
    }

    in.close();
    return result;
}
