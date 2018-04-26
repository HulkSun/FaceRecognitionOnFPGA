find_package(Qt5 COMPONENTS Core Gui Sql Widgets PrintSupport REQUIRED)
find_package(OpenCV REQUIRED)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS -std=c++11)
set(Caffe_DIR /home/sh/workspace/caffe/)

add_definitions(-DCPU_ONLY)
add_compile_options( -fpic -fopenmp -lm -lz)

include_directories(${Caffe_DIR}/include)
link_directories(${Caffe_DIR}/build/lib)
link_directories(/usr/local/ffmpeg/lib)
include_directories(
    ${Qt5Core_INCLUDE_DIRS} 
    ${Qt5Gui_INCLUDE_DIRS} 
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS}
    )
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libFaceDetect
                    ${CMAKE_CURRENT_SOURCE_DIR}/libFeature
                    ${CMAKE_CURRENT_SOURCE_DIR}/libSearch/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/qtUI)
include_directories(${Qt5Widgets_LIBRARIES})
include_directories(/usr/local/ffmpeg/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/qtUI SRC_LIST)

set(FFMPEG_LIBRARIES avcodec avdevice avfilter avformat avutil )
set(LIB_LIST 
    searchtool 
    feature
    faceDetect
    caffe
    boost_system 
    boost_filesystem 
    glog protobuf 
    ${FFMPEG_LIBRARIES} 
    swscale 
    swresample)

add_subdirectory(libFaceDetect)
add_subdirectory(libSearch)
add_subdirectory(libFeature)
add_subdirectory(libFPGA)

qt5_add_resources(qrc_FILES resources/resources.qrc)
add_executable(${PROJECT_NAME} main.cpp ${SRC_LIST}  ${qrc_FILES})
target_link_libraries(${PROJECT_NAME} ${LIB_LIST} ${OpenCV_LIBS} Qt5::Sql Qt5::Widgets Qt5::PrintSupport)

