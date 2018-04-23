# set(Caffe_DIR /home/sh/workspace/caffe/)
# include_directories(${Caffe_DIR}/include)
# link_directories(${Caffe_DIR}/build/lib)

add_library(faceDetect SHARED mtcnn.cpp)
link_libraries(caffe)