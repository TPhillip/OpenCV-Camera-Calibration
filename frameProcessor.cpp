#include "frameProcessor.h"

frameProcessor::frameProcessor(imageSource image_source, cv::Size boardSize, int source_v4l_device, int calibrationFramesCount){
	this->image_source = image_source;
	this->boardSize = &boardSize;
	this->centers = new std::vector<cv::Point2f>();
	this->obj = new std::vector<cv::Point3f>;
	this->source_v4l_device = source_v4l_device;
	this->calibration_frames_count = calibrationFramesCount;

	objVec = new std::vector<std::vector<cv::Point3f>>;
	centersVec = new std::vector<std::vector<cv::Point2f>>;

	for (int i=0; i<boardSize.height; i++) {
		for (int j=0; j<boardSize.width; j++) {
			obj->push_back(cv::Point3f(i, j, 0.0f));
		}
	}
}

void frameProcessor::loopProcessCaptureDev(){ 
	if(image_source == CAPTURE_DEVICE){
		if(capDev.open(source_v4l_device)){
			int good_frames = 0;
			bool result_found = false;
			while(true){
				capDev >> current_frame;
				if(current_frame.empty() ) break;

				//  --- tracking the checkerboard pattern ---

				bool patternWasFound = cv::findChessboardCorners(current_frame, *boardSize, *centers, cv::CALIB_CB_FAST_CHECK | CV_CALIB_CB_ADAPTIVE_THRESH);
				cv::drawChessboardCorners(current_frame, *boardSize, *centers, patternWasFound);

				// --pattern is found, do stuff ---

				if(patternWasFound && (good_frames < calibration_frames_count)){
					std::string text = "Pattern found";
					cv::putText(current_frame, text , cvPoint(30, 30), cv::FONT_HERSHEY_PLAIN, 1.5, cvScalar(0,0,255), 2);
					text = "saved frame(s) " + std::to_string(good_frames) + "/" +  std::to_string(calibration_frames_count);
					cv::putText(current_frame, text, cvPoint(30, 70), cv::FONT_HERSHEY_PLAIN, 1.5, cvScalar(0,0,255), 2);

					objVec->push_back(*obj);
					centersVec->push_back(*centers);

					good_frames++;
				}
				else if(good_frames >= calibration_frames_count){
					runCorrection();
					break;
					std::string text = "Calibration threshold reached!";
					cv::putText(current_frame, text , cvPoint(30, 30), cv::FONT_HERSHEY_PLAIN, 1.5, cvScalar(0,255,0), 2);
					if(!result_found){
						//std::thread result(&frameProcessor::runCorrection, this);
						result_found = true;
					}

				}

				cv::imshow("frame", current_frame);
				if( cv::waitKey(100) == 27){
					break;
				}
			}
		}
		else return;
	}
	else return;
}

void frameProcessor::processSingleImg(cv::Mat image){ 
	this->current_frame = image;
	if(image_source == IMAGE_FILE){

		bool patternWasFound = cv::findChessboardCorners(current_frame, *boardSize, *centers, CV_CALIB_CB_ADAPTIVE_THRESH);
		cv::drawChessboardCorners(current_frame, *boardSize, *centers, patternWasFound);

		// --pattern is found, do stuff ---

		if(patternWasFound){
			objVec->push_back(*obj);
			centersVec->push_back(*centers);
			cv::imshow("originalImage", current_frame);
			runCorrection();
		}
		else{
			cv::imshow("file", image);
		}
		cv::waitKey(0);
	}
}

void frameProcessor::runCorrection(){
	cv::Mat cameraMatrix = cv::Mat(3, 3, CV_64F);
	cameraMatrix.at<double>(0,0) = 1.0;

	cv::Mat distCoeffs;
	distCoeffs = cv::Mat::zeros(8, 1, CV_64F);

	std::vector<cv::Mat> rvecs;
	std::vector<cv::Mat> tvecs;

	cv::Size imageSize = current_frame.size();

	cv::calibrateCamera(*objVec, *centersVec, *boardSize, cameraMatrix, distCoeffs, rvecs, tvecs);
	cv::undistort(current_frame, undistored_frame, cameraMatrix, distCoeffs);
	std::string text = "Image Corrected!";
	cv::putText(undistored_frame, text , cvPoint(30, 30), cv::FONT_HERSHEY_PLAIN, 1.5, cvScalar(0,255,0), 2);
	cv::imshow("Corrected Image", undistored_frame);
	cv::waitKey(0);
}
