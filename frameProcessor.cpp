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
	imageCalibrated=false;

	for(int j=0;j<(boardSize.width * boardSize.height);j++)
		obj->push_back(cv::Point3f(j/boardSize.width, j%boardSize.width, 0.0f));
}

void frameProcessor::loopProcessCaptureDev(){ 
	if(image_source == CAPTURE_DEVICE){
		if(capDev.open(source_v4l_device)){
			int good_frames = 0;
			bool result_found = false;
			while(true){
				int key = cv::waitKey(1);
				capDev >> current_frame;
				cv::Mat disposable_frame;
				capDev >> disposable_frame;
				cvtColor(disposable_frame, grayscale_frame, CV_BGR2GRAY);
				if(disposable_frame.empty() ) break;

				//  --- tracking the checkerboard pattern ---

				bool patternWasFound = cv::findChessboardCorners(disposable_frame, *boardSize, *centers, cv::CALIB_CB_FAST_CHECK | CV_CALIB_CB_ADAPTIVE_THRESH);
				//cv::cornerSubPix(grayscale_frame, *centers, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
				cv::drawChessboardCorners(disposable_frame, *boardSize, *centers, patternWasFound);

				// --pattern is found, do stuff ---

				if(patternWasFound && (good_frames < calibration_frames_count)){
					std::string text = "Pattern found";
					cv::putText(disposable_frame, text , cvPoint(30, 30), cv::FONT_HERSHEY_PLAIN, 1.5, cvScalar(0,0,255), 2);
					text = "saved frame(s) " + std::to_string(good_frames) + "/" +  std::to_string(calibration_frames_count);
					cv::putText(disposable_frame, text, cvPoint(30, 70), cv::FONT_HERSHEY_PLAIN, 1.5, cvScalar(0,0,255), 2);

					objVec->push_back(*obj);
					centersVec->push_back(*centers);

					good_frames++;
				}
				else if(good_frames >= calibration_frames_count){
					if(!imageCalibrated)
						runCorrection();					
					
					if(imageCalibrated){
						cv::undistort(current_frame, disposable_frame, intrinsic, distCoeffs);
						std::string text = "Calibration threshold reached!";
						cv::putText(disposable_frame, text , cvPoint(30, 30), cv::FONT_HERSHEY_PLAIN, 1.5, cvScalar(0,255,0), 2);
					}
					
					if(!result_found){
						//std::thread result(&frameProcessor::runCorrection, this);
						result_found = true;
					}

				}

				cv::imshow("current_frame", disposable_frame);
				if(key == 27){
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

		bool patternWasFound = cv::findChessboardCorners(image, *boardSize, *centers, CV_CALIB_CB_ADAPTIVE_THRESH);
		cv::drawChessboardCorners(image, *boardSize, *centers, patternWasFound);

		// --pattern is found, do stuff ---

		if(patternWasFound){
			objVec->push_back(*obj);
			centersVec->push_back(*centers);
			cv::imshow("originalImage", image);
			runCorrection();
		}
		else{
			cv::imshow("file", image);
		}
		cv::waitKey(0);
	}
}

void frameProcessor::runCorrection(){
	std::vector<cv::Mat> rvecs;
	std::vector<cv::Mat> tvecs;

	intrinsic.ptr<float>(0)[0] = 1;
	intrinsic.ptr<float>(1)[1] = 1;

	cv::calibrateCamera(*objVec, *centersVec, current_frame.size(), intrinsic, distCoeffs, rvecs, tvecs);

	imageCalibrated = true;
}
