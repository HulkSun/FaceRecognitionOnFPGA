#ifndef REDIS_MANAGER_H
#define REDIS_MANAGER_H

#include <hiredis/hiredis.h>
#include <string>
#include <vector>
#include <map>
#include "SpinLock.h"

class RedisManager
{

  public:
    explicit RedisManager(const char *h, int p);
    ~RedisManager();
    //redist operate
    bool initRedisConnect(int ms = 1000);
    bool selectDb(int index);
    void bgsave();
    void customCMD(const char *cmd);
    bool authPasswd(const char *passwd);

    // set operate
    int set_adds(const char *keyOfset, const char *items);
    int set_dels(const char *keyOfset, const char *items);
    bool set_IsMember(const char *keyOfset, const char *item);
    unsigned set_size(const char *keyOfset);
    std::vector<std::string> set_getAll(const char *keyOfset);
    // hash operate
    bool hash_set(const char *hashkey, const char *subkey, const char *subitem);
    bool hash_sets(const char *hashkey, const char *sub_key_values);
    bool hash_add(const char *hashkey, const char *subkey, const char *subitem);
    // bool hash_adds(const char* hashkey,const char* sub_key_values);
    bool hash_del(const char *hashkey, const char *subkey);
    bool hash_dels(const char *hashkey, const char *sub_key_values);
    bool hash_exist(const char *hashkey, const char *subkey);
    size_t hash_len(const char *hashkey);
    int hash_increase(const char *hashkey, const char *subkey, int n);
    std::string hash_get(const char *hashkey, const char *subkey);
    std::vector<std::string> hash_gets(const char *hashkey, const char *subkeys);
    std::vector<std::string> hash_getAllKyes(const char *hashkey);
    std::vector<std::pair<std::string, std::string>> hash_getAll(const char *hashkey);
    std::map<std::string, std::string> hash_getAllOrdered(const char *hashkey);

    // set operate binary safe api
    int set_add(const char *setKey,size_t len1, const char *item,size_t len2);
    int set_del(const char *setKey,size_t len1, const char *item,size_t len2);
    bool set_IsMember(const char *setKey,size_t len1, const char *item,size_t len2);
    // hash operate binary safe api
    bool hash_set(const char *hashkey, size_t len1, const char *subkey, size_t len2, const char *subitem, size_t len3);
    bool hash_add(const char *hashkey, size_t len1, const char *subkey, size_t len2, const char *subitem, size_t len3);
    bool hash_del(const char *hashkey, size_t len1, const char *subkey, size_t len2);
    std::string hash_get(const char *hashkey, size_t len1, const char *subkey, size_t len2);
    size_t hash_len(const char *hashkey, size_t len1);
    std::vector<std::pair<std::string, std::string>> hash_getAll(const char *hashkey, size_t len1);

  private:
    RedisManager(const RedisManager &) {}
    RedisManager &operator=(const RedisManager &);
    redisContext *pRedisCtx;
    std::string hostname;
    int port;
    dev::SpinLock mtx;
};

#endif //REDIS_MANAGER_H
