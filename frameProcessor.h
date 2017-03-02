#include "opencv2/opencv.hpp"

enum imageSource {CAPTURE_DEVICE, IMAGE_FILE};

class frameProcessor{
public:
	frameProcessor(imageSource image_source, cv::Size boardSize, int source_v4l_device = 0, int calibrationFramesCount = 0);
	void loopProcessCaptureDev();
    void processSingleImg(cv::Mat image);

private:
	cv::Size *boardSize;
	std::vector<cv::Point2f> *centers;
	std::vector<cv::Point3f> *obj;
	std::vector<std::vector<cv::Point3f>> *objVec;
    std::vector<std::vector<cv::Point2f>> *centersVec;

    cv::VideoCapture capDev;
    int source_v4l_device;
    int calibration_frames_count;

    imageSource image_source;
};