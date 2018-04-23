#include "center_face.h"
#include <iostream>
#include "global.h"

CenterFace::CenterFace(const std::string &proto_model_dir){
    FLAGS_minloglevel = 2;
#ifdef CPU_ONLY
    Caffe::set_mode(Caffe::CPU);
#else
    Caffe::set_mode(Caffe::GPU);
    Caffe::SetDevice(GpuId);
#endif
    /* Load the network. */
    cNet_.reset(new Net<float>((proto_model_dir + "/center_face_deploy.prototxt"), TEST));
    cNet_->CopyTrainedLayersFrom(proto_model_dir + "/center_face.caffemodel");
    Blob<float> *input_layer = cNet_->input_blobs()[0];
    num_channels_ = input_layer->channels();
    CHECK(num_channels_ == 3|| num_channels_==1)
            <<"input layer should have 1 or 3 channels";
    input_geometry_ = cv::Size(input_layer->width(), input_layer->height());
    input_layer->Reshape(1, num_channels_, input_geometry_.height, input_geometry_.width);
    cNet_->Reshape();

    std::string queryBlobName = "fc5";
    blobId_ = GetBlobIndex(cNet_, queryBlobName);
}

unsigned int CenterFace::GetBlobIndex(boost::shared_ptr<Net<float> > &net, const string &queryBlobName){
    std::vector<string> const &blob_names = net->blob_names();
    for(unsigned int i = 0; i != blob_names.size(); ++i){
        if(queryBlobName == blob_names[i]) return i;
    }
    return 0;
}

std::vector<float> CenterFace::ExtractFeature(const cv::Mat &img){
    feature_.clear();
    std::vector<float> feature512;

    /* source image */
    std::vector<cv::Mat> input_channels;
    WrapInputLayer(&input_channels);
    Preprocess(img, &input_channels);
    cNet_->Forward();

    boost::shared_ptr<Blob<float> > blob = cNet_->blobs()[blobId_];
    unsigned int numData = blob->count();
    const float *blobPtr = (const float*)blob->cpu_data();
    for(unsigned int i = 0; i != numData; ++i){
        feature512.push_back(*blobPtr++);
    }
    feature_.insert(feature_.end(), feature512.begin(), feature512.end());

    /* flip image */
    cv::Mat flipImg;
    cv::flip(img, flipImg, 1);
    Preprocess(flipImg, &input_channels);
    cNet_->Forward();
    blob = cNet_->blobs()[blobId_];
    numData = blob->count();
    blobPtr = (const float*)blob->cpu_data();
    for(unsigned int i = 0; i != numData; ++i){
        feature512[i] = (*blobPtr++);
    }
    feature_.insert(feature_.end(), feature512.begin(), feature512.end());

    return feature_;
}

void CenterFace::WrapInputLayer(std::vector<cv::Mat> *input_channels)
{
    Blob<float>* input_layer = cNet_->input_blobs()[0];
    int width = input_layer->width();
    int height = input_layer->height();
    float* input_data = input_layer->mutable_cpu_data();
    for(int i = 0; i < input_layer->channels(); ++i){
        cv::Mat channel(height, width, CV_32FC1, input_data);
        input_channels->push_back(channel);
        input_data += width * height;
    }
}

void CenterFace::Preprocess(const cv::Mat &img, std::vector<cv::Mat> *input_channels)
{
    /* Convert the input image to the input image format of the network. */
    cv::Mat sample;
    if(img.channels() == 3 && num_channels_ == 1)
        cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
    else if(img.channels() == 4 && num_channels_ == 1)
        cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
    else if(img.channels() == 4 && num_channels_ == 3)
        cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
    else if (img.channels() == 1 && num_channels_ == 3)
        cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
    else
        sample = img;

    //resize() 耗时，第一次执行时耗时228ms
    cv::Mat sample_resized;
    if(sample.size() != input_geometry_)
        cv::resize(sample, sample_resized, input_geometry_);
    else
        sample_resized = sample;

    cv::Mat sample_float;
    if(num_channels_ == 3)
        sample_resized.convertTo(sample_float, CV_32FC3);
    else
        sample_resized.convertTo(sample_float, CV_32FC1);

//#pragma omp parallel for
    // for(int i = 0; i < sample_float.channels(); ++i)
    //     for(int j = 0; j < sample_float.rows; ++j)
    //         for(int k = 0; k < sample_float.cols; ++k){
    //             float value = sample_float.at<cv::Vec3f>(j, k)[i];
    //             sample_float.at<cv::Vec3f>(j,k)[i] = (value - 127.5)/128;
    //         }
    if(num_channels_ == 3)
        sample_float.convertTo(sample_float, CV_32FC3, 0.0078125, -127.5*0.0078125);
    else
        sample_float.convertTo(sample_float, CV_32FC1, 0.0078125, -127.5*0.0078125);

    cv::split(sample_float, *input_channels);
}

CenterFace::~CenterFace(){
}
