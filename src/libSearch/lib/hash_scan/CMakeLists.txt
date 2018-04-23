cmake_minimum_required(VERSION 2.6)
project (hashscan)

find_package(JNI)
if (JNI_FOUND)
	message (STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
	message (STATUS "JNI_LIBRARIES=${JNI_LIBRARIES}")
endif()

find_package(Threads)

set (CMAKE_BUILD_TYPE "Release")
set (CMAKE_CXX_FLAGS "-DSINGLE_CORE -O3 -fopenmp")
#set (CMAKE_CXX_FLAGS "-DSINGLE_CORE -O2 -fopenmp -DNDEBUG")
# -Wall -march=native -mtune=native")

include_directories (${PROJECT_SOURCE_DIR} ${JNI_INCLUDE_DIRS})

add_library (hashscan SHARED linscan_interface.cpp linscan.cpp)
target_link_libraries (hashscan ${JNI_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
