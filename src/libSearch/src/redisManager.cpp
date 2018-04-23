#include "redisManager.h"
#include <iostream>
#include <utility>

using namespace std;

RedisManager::RedisManager(const char *h = "127.0.0.1", int p = 6379)
    : pRedisCtx(NULL), hostname(h), port(p)
{
}

RedisManager::~RedisManager()
{
    // Disconnects and frees the context
    if (pRedisCtx)
        redisFree(pRedisCtx);
}

bool RedisManager::initRedisConnect(int ms)
{
    struct timeval timeout = {0, ms};
    mtx.lock();
    if (pRedisCtx)
        redisFree(pRedisCtx);

    pRedisCtx = (redisContext *)redisConnectWithTimeout(hostname.c_str(), port, timeout);
    if ((pRedisCtx == NULL) || (pRedisCtx->err))
    {
        if (pRedisCtx)
        {
            cout << "Connection error:" << pRedisCtx->errstr << endl;
        }
        else
        {
            cout << "Connection error: can't allocate redis context" << endl;
        }
        return false;
    }
    mtx.unlock();
    return true;
}

bool RedisManager::selectDb(int index)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "select %d", index);
    mtx.unlock();
    bool success = false;
    if (replay)
    {
        if (string("OK") == replay->str)
            success = true;
    }
    freeReplyObject(replay);
    return success;
}
void RedisManager::customCMD(const char *cmd)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, cmd);
    mtx.unlock();
    freeReplyObject(replay);
}

bool RedisManager::authPasswd(const char *passwd)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "auth %s", passwd);
    mtx.unlock();
    bool success = false;
    if (replay)
    {
        if (string("OK") == replay->str)
            success = true;
    }
    freeReplyObject(replay);
    return success;
}

void RedisManager::bgsave()
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "bgsave");
    mtx.unlock();
    freeReplyObject(replay);
}
// set oprations
int RedisManager::set_adds(const char *keyOfset, const char *items)
{
    string cmd("sadd ");
    cmd += keyOfset + string(" ") + string(items);
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, cmd.c_str());
    mtx.unlock();
    unsigned set_size = 0;
    if (replay)
    {
        set_size = replay->integer;
    }
    freeReplyObject(replay);
    return set_size;
}

int RedisManager::set_dels(const char *keyOfset, const char *items)
{
    string cmd("srem ");
    cmd += keyOfset + string(" ") + string(items);
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, cmd.c_str());
    mtx.unlock();
    unsigned set_size = 0;
    if (replay)
    {
        set_size = replay->integer;
    }
    freeReplyObject(replay);
    return set_size;
}

bool RedisManager::set_IsMember(const char *keyOfset, const char *item)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "sismember %s %s", keyOfset, item);
    mtx.unlock();
    bool success = false;
    if (replay)
    {
        if (replay->integer == 1)
            success = true;
    }
    freeReplyObject(replay);
    return success;
}

unsigned RedisManager::set_size(const char *keyOfset)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "scard %s", keyOfset);
    mtx.unlock();
    unsigned set_size = 0;
    if (replay)
    {
        set_size = replay->integer;
    }
    freeReplyObject(replay);
    return set_size;
}

vector<string> RedisManager::set_getAll(const char *keyOfset)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "smembers %s", keyOfset);
    mtx.unlock();
    vector<string> res;
    if (replay)
    {
        if (replay->type == REDIS_REPLY_ARRAY)
            for (unsigned i = 0; i < replay->elements; ++i)
            {
                res.push_back(string(replay->element[i]->str, replay->element[i]->len));
            }
    }
    freeReplyObject(replay);
    return res;
}

// hash set oprations
bool RedisManager::hash_set(const char *hashkey, const char *subkey, const char *subitem)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hset %s %s %s", hashkey, subkey, subitem);
    mtx.unlock();
    bool success = false;

    if (replay)
    {
        // if(replay->integer == 1) //create
        success = true;
        if (replay->type == REDIS_REPLY_ERROR)
        {
            cout << replay->str << endl;
            success = false;
        }
    }
    freeReplyObject(replay);
    return success;
}

bool RedisManager::hash_sets(const char *hashkey, const char *sub_key_values)
{
    string cmd("hmset ");
    cmd += hashkey + string(" ") + string(sub_key_values);
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, cmd.c_str());
    mtx.unlock();
    bool success = false;
    if (replay)
    {
        if (string("OK") == replay->str)
            success = true;
    }
    freeReplyObject(replay);
    return success;
}

bool RedisManager::hash_add(const char *hashkey, const char *subkey, const char *subitem)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hsetnx %s %s %s", hashkey, subkey, subitem);
    mtx.unlock();
    bool success = false;
    if (replay)
    {
        if (replay->integer == 1)
            success = true;
    }
    freeReplyObject(replay);
    return success;
}

bool RedisManager::hash_dels(const char *hashkey, const char *subkeys)
{
    string cmd("hdel ");
    cmd += hashkey + string(" ") + string(subkeys);
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, cmd.c_str());
    mtx.unlock();
    bool success = false;
    if (replay)
    {
        if (replay->integer >= 0)
            success = true;
    }
    freeReplyObject(replay);
    return success;
}

bool RedisManager::hash_del(const char *hashkey, const char *subkey)
{
    return hash_dels(hashkey, subkey);
}

bool RedisManager::hash_exist(const char *hashkey, const char *subkey)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hexists %s %s", hashkey, subkey);
    mtx.unlock();
    bool success = false;
    if (replay)
    {
        if (replay->integer == 1)
            success = true;
    }
    freeReplyObject(replay);
    return success;
}

size_t RedisManager::hash_len(const char *hashkey)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hlen %s ", hashkey);
    mtx.unlock();
    size_t len = 0;
    if (replay)
    {
        if (replay->type == REDIS_REPLY_INTEGER)
            len = replay->integer;
    }
    freeReplyObject(replay);
    return len;
}
int RedisManager::hash_increase(const char *hashkey, const char *subkey, int n)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hincrby %s %s %d", hashkey, subkey, n);
    mtx.unlock();
    int value = 0;
    if (replay)
    {
        if (replay->type == REDIS_REPLY_INTEGER)
            value = replay->integer;
    }
    freeReplyObject(replay);
    return value;
}

string RedisManager::hash_get(const char *hashkey, const char *subkey)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hget %s %s", hashkey, subkey);
    mtx.unlock();
    string res;
    if (replay)
    {
        if (replay->type == REDIS_REPLY_STRING)
        {
            res = replay->str;
        }
    }
    freeReplyObject(replay);
    return res;
}

vector<string> RedisManager::hash_gets(const char *hashkey, const char *subkeys)
{
    string cmd("hmget ");
    cmd += hashkey + string(" ") + string(subkeys);
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, cmd.c_str());
    mtx.unlock();
    vector<string> res;
    bool hasValue = false;
    if (replay)
    {
        if (replay->type == REDIS_REPLY_ARRAY)
            for (unsigned i = 0; i < replay->elements; ++i)
            {
                if (replay->element[i]->type == REDIS_REPLY_STRING)
                {
                    hasValue = true;
                    res.push_back(string(replay->element[i]->str));
                }
                else
                    res.push_back(string(""));
            }
    }
    if (!hasValue)
        res.clear();
    freeReplyObject(replay);
    return res;
}

vector<string> RedisManager::hash_getAllKyes(const char *hashkey)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hkeys %s", hashkey);
    mtx.unlock();
    vector<string> res;
    if (replay)
    {
        if (replay->type == REDIS_REPLY_ARRAY)
            for (unsigned i = 0; i < replay->elements; ++i)
            {
                res.push_back(replay->element[i]->str);
            }
    }
    freeReplyObject(replay);
    return res;
}

vector<pair<string, string>> RedisManager::hash_getAll(const char *hashkey)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hgetall %s", hashkey);
    mtx.unlock();
    vector<pair<string, string>> res;
    if (replay)
    {
        if (replay->type == REDIS_REPLY_ARRAY)
            for (unsigned i = 0; i < replay->elements; i += 2)
            {
                res.push_back(make_pair(replay->element[i]->str, string(replay->element[i + 1]->str, replay->element[i + 1]->len)));
            }
    }
    freeReplyObject(replay);
    return res;
}

std::map<std::string, std::string> RedisManager::hash_getAllOrdered(const char *hashkey)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hgetall %s", hashkey);
    mtx.unlock();
    std::map<std::string, std::string> res;
    if (replay)
    {
        if (replay->type == REDIS_REPLY_ARRAY)
            for (unsigned i = 0; i < replay->elements; i += 2)
            {
                res[string(replay->element[i]->str)]=string(replay->element[i + 1]->str, replay->element[i + 1]->len);
            }
    }
    freeReplyObject(replay);
    return res;
}

// binary safe api

//set operate binary safe api
int RedisManager::set_add(const char *setKey, size_t len1, const char *item, size_t len2)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "sadd %b %b", setKey, len1, item, len2);
    mtx.unlock();
    unsigned set_size = 0;
    if (replay)
    {
        set_size = replay->integer;
    }
    freeReplyObject(replay);
    return set_size;
}

int RedisManager::set_del(const char *setKey, size_t len1, const char *item, size_t len2)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "srem %b %b", setKey, len1, item, len2);
    mtx.unlock();
    unsigned set_size = 0;
    if (replay)
    {
        set_size = replay->integer;
    }
    freeReplyObject(replay);
    return set_size;
}

bool RedisManager::set_IsMember(const char *setKey, size_t len1, const char *item, size_t len2)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "sismember %b %b", setKey, len1, item, len2);
    mtx.unlock();
    bool success = false;
    if (replay)
    {
        if (replay->integer == 1)
            success = true;
    }
    freeReplyObject(replay);
    return success;
}
//hash operate binary safe api
bool RedisManager::hash_set(const char *hashkey, size_t len1, const char *subkey, size_t len2, const char *subitem, size_t len3)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hset %b %b %b", hashkey, len1, subkey, len2, subitem, len3);
    mtx.unlock();
    bool success = false;
    if (replay)
    {
        // if(replay->integer == 1) //create
        success = true;
        if (replay->type == REDIS_REPLY_ERROR)
        {
            cout << replay->str << endl;
            success = false;
        }
    }
    freeReplyObject(replay);
    return success;
}
bool RedisManager::hash_add(const char *hashkey, size_t len1, const char *subkey, size_t len2, const char *subitem, size_t len3)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hsetnx %b %b %b", hashkey, len1, subkey, len2, subitem, len3);
    mtx.unlock();
    bool success = false;
    if (replay)
    {
        if (replay->integer == 1)
            success = true;
    }
    freeReplyObject(replay);
    return success;
}
string RedisManager::hash_get(const char *hashkey, size_t len1, const char *subkey, size_t len2)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hget %b %b", hashkey, len1, subkey, len2);
    mtx.unlock();
    string res;
    if (replay)
    {
        if (replay->type == REDIS_REPLY_STRING)
        {
            res = replay->str;
        }
    }
    freeReplyObject(replay);
    return res;
}
bool RedisManager::hash_del(const char *hashkey, size_t len1, const char *subkey, size_t len2)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hdel %b %b", hashkey, len1, subkey, len2);
    mtx.unlock();
    bool success = false;
    if (replay)
    {
        if (replay->integer >= 0)
            success = true;
    }
    freeReplyObject(replay);
    return success;
}
size_t RedisManager::hash_len(const char *hashkey, size_t len1)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hlen %b ", hashkey, len1);
    mtx.unlock();
    size_t len = 0;
    if (replay)
    {
        if (replay->type == REDIS_REPLY_INTEGER)
            len = replay->integer;
    }
    freeReplyObject(replay);
    return len;
}

vector<pair<string, string>> RedisManager::hash_getAll(const char *hashkey, size_t len1)
{
    mtx.lock();
    redisReply *replay = (redisReply *)redisCommand(pRedisCtx, "hgetall %b", hashkey, len1);
    mtx.unlock();
    vector<pair<string, string>> res;
    if (replay)
    {
        if (replay->type == REDIS_REPLY_ARRAY)
            for (unsigned i = 0; i < replay->elements; i += 2)
            {
                res.push_back(make_pair(replay->element[i]->str, string(replay->element[i + 1]->str, replay->element[i + 1]->len)));
            }
    }
    freeReplyObject(replay);
    return res;
}