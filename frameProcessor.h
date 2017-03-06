#include "opencv2/opencv.hpp"
#include <thread>

enum imageSource {CAPTURE_DEVICE, IMAGE_FILE};

class frameProcessor{
public:
	frameProcessor(imageSource image_source, cv::Size boardSize, int source_v4l_device = 0, int calibrationFramesCount = 0);
	void loopProcessCaptureDev();
	void processSingleImg(cv::Mat image);
	void runCorrection();

private:
	cv::Mat current_frame;
	cv::Mat undistored_frame;
	cv::Mat grayscale_frame;
	cv::Size *boardSize;
	std::vector<cv::Point2f> *centers;
	std::vector<cv::Point3f> *obj;
	std::vector<std::vector<cv::Point3f>> *objVec;
	std::vector<std::vector<cv::Point2f>> *centersVec;


	//for calibration process
	cv::Mat intrinsic = cv::Mat(3, 3, CV_32FC1);
	cv::Mat distCoeffs;
	bool imageCalibrated;

	//for capture process
	cv::VideoCapture capDev;
	int source_v4l_device;
	int calibration_frames_count;

	imageSource image_source;
};