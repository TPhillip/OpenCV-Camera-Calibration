#include <vector>
#include <string>
#include <iostream>

#include "opencv2/opencv.hpp"
#include "frameProcessor.h"

const int board_w = 6;
const int board_h = 4;

int main(int argc, char** argv){
	std::string in;
	cv::CommandLineParser parser(argc, argv, "{input i||input image}{camera_num c||v4L video device number}{calib_frames f||amount of frames to consider a sucessful calibration}{help h||show help message}");
	
	if (parser.has("help"))
	{
		parser.printMessage();
		return 0;
	}

	if(parser.has("input")){ //if user provides an image process image
		in = parser.get<std::string>("input");
		frameProcessor fp(IMAGE_FILE, cv::Size(board_w,board_h));
		fp.processSingleImg(cv::imread(in));
		return 0;
	}

	else if(parser.has("camera_num") && parser.has("calib_frames")){ //Otherwise get frames from VideoCapture
		int camera = parser.get<int>("camera_num"); //Get the  V4L camera number
		int cFrames = parser.get<int>("calib_frames"); //amount sucessful frames to store for calibration
		frameProcessor fp(CAPTURE_DEVICE, cv::Size(board_w,board_h), camera, cFrames);
		fp.loopProcessCaptureDev();
		return 0;
	}

	parser.printMessage();
}

