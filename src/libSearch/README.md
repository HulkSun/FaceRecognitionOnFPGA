# search tools

## Install Redis
```bash
$ sudo apt-get update
$ sudo apt-get install redis-server
$ sudo apt-get install redis-tools
# start redis
$ redis-server

# command line
$ redis-cli
```

## include
1. redisManager : c++ interface to operate redis
2. lib/hash_scan : mih
3. lib/hiredis: official c interface  
`github:https://github.com/redis/hiredis.git`

## Redis keys

- person_info_hash(Hash)

|key|value|
|:--|:--|
|id_id|person's id|
|id_name|person's name|
|id_sex|person's sex|
|id_timestamp|time of last modify|

- face_info_hash(Hash)

|key|value|
|:--|:--|
|id_face_count|how many face photo|
|id_face_url_0|path of face|
|id_face_hashkey_0|hashkey of face|

- hash_codes_set(set)
All hash codes.

- hashcode(Hash)

|key|value|
|:--|:--|
|id_face_feature_0|float feature|

## Compile

```bash
$ git submodule update --init
$ mkdir build
$ cd build
$ cmake ..
$ make

```

