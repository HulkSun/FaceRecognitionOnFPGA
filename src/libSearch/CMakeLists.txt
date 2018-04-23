add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
#SET (CMAKE_CXX_COMPILER "g++")
SET (CMAKE_BUILD_TYPE "Release")

# Initialize CXXFLAGS.
set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} -Wall -fopenmp -std=c++11 -Wreorder -Wno-sign-compare -Wno-reorder")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# source code 
SET (HASH_SCAN_SRC lib/hash_scan/MIH.cpp lib/hash_scan/linscan.cpp lib/hash_scan/mihasher.cpp lib/hash_scan/sparse_hashtable.cpp lib/hash_scan/bucket_group.cpp lib/hash_scan/array32.cpp )
SET (HASH_SCAN_DIR lib/hash_scan)
SET (HIREDIS_SRC lib/hiredis/read.c lib/hiredis/sds.c lib/hiredis/net.c lib/hiredis/dict.c lib/hiredis/hiredis.c)
SET (HIREDIS_DIR lib/hiredis)
#AUX_SOURCE_DIRECTORY(${HIREDIS_DIR} HIREDIS_SRC)

INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/lib ${HASH_SCAN_DIR} ${HIREDIS_DIR})

# library config
ADD_LIBRARY(searchtool SHARED src/searchTool.cpp src/redisManager.cpp ${HASH_SCAN_SRC} ${HIREDIS_SRC})
target_link_libraries(searchtool boost_serialization)
#ADD_LIBRARY(searchtool_static STATIC src/searchTool.cpp src/redisManager.cpp ${HASH_SCAN_SRC} ${HIREDIS_SRC})


