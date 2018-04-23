#ifndef CENTER_FACE_H
#define CENTER_FACE_H

// caffe
#include <caffe/caffe.hpp>
#include <caffe/layers/memory_data_layer.hpp>
// C++
#include <string>
#include <vector>
// opencv
#include <opencv2/opencv.hpp>
// boost
#include "boost/make_shared.hpp"
//#include <omp.h>
// #define CPU_ONLY
using namespace caffe;

class CenterFace {
public:
    CenterFace(const string &proto_model_dir);
    ~CenterFace();
    unsigned int GetBlobIndex(boost::shared_ptr<Net<float> > &net, const std::string &queryBlobName);
    std::vector<float> ExtractFeature(const cv::Mat &img);
private:
    void WrapInputLayer(std::vector<cv::Mat>* input_channels);
    void Preprocess(const cv::Mat &img, std::vector<cv::Mat>* input_channels);

    boost::shared_ptr<Net<float> > cNet_;
    std::vector<float> feature_;
    unsigned int blobId_;

    cv::Size input_geometry_;
    int num_channels_;
};

#endif // CENTER_FACE_H
