
///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017, Carnegie Mellon University and University of Cambridge,
// all rights reserved.
//
// ACADEMIC OR NON-PROFIT ORGANIZATION NONCOMMERCIAL RESEARCH USE ONLY
//
// BY USING OR DOWNLOADING THE SOFTWARE, YOU ARE AGREEING TO THE TERMS OF THIS LICENSE AGREEMENT.  
// IF YOU DO NOT AGREE WITH THESE TERMS, YOU MAY NOT USE OR DOWNLOAD THE SOFTWARE.
//
// License can be found in OpenFace-license.txt

//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite at least one of the following works:
//
//       OpenFace 2.0: Facial Behavior Analysis Toolkit
//       Tadas Baltrušaitis, Amir Zadeh, Yao Chong Lim, and Louis-Philippe Morency
//       in IEEE International Conference on Automatic Face and Gesture Recognition, 2018  
//
//       Convolutional experts constrained local model for facial landmark detection.
//       A. Zadeh, T. Baltrušaitis, and Louis-Philippe Morency,
//       in Computer Vision and Pattern Recognition Workshops, 2017.    
//
//       Rendering of Eyes for Eye-Shape Registration and Gaze Estimation
//       Erroll Wood, Tadas Baltrušaitis, Xucong Zhang, Yusuke Sugano, Peter Robinson, and Andreas Bulling 
//       in IEEE International. Conference on Computer Vision (ICCV),  2015 
//
//       Cross-dataset learning and person-specific normalisation for automatic Action Unit detection
//       Tadas Baltrušaitis, Marwa Mahmoud, and Peter Robinson 
//       in Facial Expression Recognition and Analysis Challenge, 
//       IEEE International Conference on Automatic Face and Gesture Recognition, 2015 
//
///////////////////////////////////////////////////////////////////////////////
// FaceTrackingVid.cpp : Defines the entry point for the console application for tracking faces in videos.

// Libraries for landmark detection (includes CLNF and CLM modules)
#include "GlanceMgr.h"
#include <ctime>
#include <GazeEstimation.h>

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace Glance;


GlanceMgr::GlanceMgr() {
}

GlanceMgr::~GlanceMgr() {
}

void GlanceMgr::LogPrint( std::string &buffer, std::string message, uint level ) {
	switch ( level ) {
		case 0:
			(buffer) += "Info:" + message + "\r\n";
			break;
		case 1:
			(buffer) += "Warning:" + message + "\r\n";
			break;
		case 2:
			(buffer) += "Error:" + message + "\r\n";
			break;
		case 3:
			(buffer) += "Fatal error::" + message + "\r\n";
			break;
	}
}

#define INFO_STREAM( buffer, message ) LogPrint( buffer, message, 0)
#define WARNING_STREAM( buffer, message ) LogPrint( buffer, message, 1)
#define ERROR_STREAM( buffer, message ) LogPrint( buffer, message, 2)
#define FATAL_STREAM( buffer, message ) LogPrint( buffer, message, 3)


int GlanceMgr::Init( std::string RootDirectory , std::string &LogMsg) {

	vector<string> arguments = { RootDirectory, "-device",  "0" };

	det_parameters = LandmarkDetector::FaceModelParameters( arguments );

	// The modules that are being used for tracking
	face_model = LandmarkDetector::CLNF( det_parameters.model_location );

	if ( !face_model.loaded_successfully ) {
		FATAL_STREAM( LogMsg, "ERROR: Could not load the landmark detector" );
		return -1;
	}

	if ( !face_model.eye_model ) {
		FATAL_STREAM( LogMsg, "WARNING: no eye model found" );
		return -1;
	}

	// The sequence reader chooses what to open based on command line arguments provided
	if ( !sequence_reader.Open( arguments ) ) {
		ERROR_STREAM( LogMsg, "Device or file opened" );
		return -1;
	}

	LogMsg = "Succesfully Initialized the webcam ";

	srand( time( NULL ) );
	face_id = rand();

	FlagInitialized = true;
	Stopped = false;

	return 0;
}

void GlanceMgr::Close() {	
	Stopped = true;

	if ( recorder.isOpen() ) {
		recorder.Close();
		DetectionParameters.FlagLog = false;
	}

	face_model.Reset();
	sequence_reader.Close();
}

cv::Size GlanceMgr::GetFrameSize() {
	if ( FlagInitialized ) {
		return { sequence_reader.frame_width, sequence_reader.frame_height };
	}

	return { 0, 0 };
}


int GlanceMgr::GetFrame( cv::Mat &frame, GlanceResult &Result ) {
	if ( Stopped ) {
		return -1;
	}

	frame = sequence_reader.GetNextFrame();
	if ( frame.empty() ) {
		return -1;
	}

	// Reading the images
	cv::Mat_<uchar> grayscale_image = sequence_reader.GetGrayFrame();

	// The actual facial landmark detection / tracking
	bool detection_success = LandmarkDetector::DetectLandmarksInVideo( frame, face_model, det_parameters, grayscale_image );

	if ( detection_success ) {

		// If tracking succeeded and we have an eye model, estimate gaze
		if ( DetectionParameters.FlagGaze ) {
			if ( detection_success && face_model.eye_model ) {
				GazeAnalysis::EstimateGaze( face_model, Result.GazeLeft, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy, true );
				GazeAnalysis::EstimateGaze( face_model, Result.GazeRight, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy, false );
				Result.GazeAngle = GazeAnalysis::GetGazeAngle( Result.GazeLeft, Result.GazeRight );
			}
		}


		if ( DetectionParameters.FlagHeadPose ) {
			// Work out the pose of the head from the tracked model
			Result.HeadPose = LandmarkDetector::GetPose( face_model, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy );
		}
		// Keeping track of FPS
		fps_tracker.AddFrame();

		// Displaying the tracking visualizations
		if ( DetectionParameters.FlagVisualized ) {
			visualizer.SetImage( frame, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy );
			visualizer.SetObservationLandmarks( face_model.detected_landmarks, face_model.detection_certainty, face_model.GetVisibilities() );
			visualizer.SetObservationPose( Result.HeadPose, face_model.detection_certainty );
			visualizer.SetObservationGaze( Result.GazeLeft, Result.GazeRight, LandmarkDetector::CalculateAllEyeLandmarks( face_model ), LandmarkDetector::Calculate3DEyeLandmarks( face_model, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy ), face_model.detection_certainty );
			visualizer.SetFps( fps_tracker.GetFPS() );
			frame = visualizer.GetVisImage();
		}

		if ( DetectionParameters.FlagLog ) {
			recorder.WriteLine( face_id, sequence_reader.GetFrameNumber(), sequence_reader.time_stamp, face_model.detection_success, face_model.detection_certainty, {}, {}, {}, {}, Result.HeadPose, Result.GazeLeft, Result.GazeRight, Result.GazeAngle, {}, {}, {}, {} );
		}

		return 0;
	}

	return -1;
}

cv::Rect Glance::GlanceMgr::GetFaceRect() {
	return face_model.GetBoundingBox();
}

bool Glance::GlanceMgr::IsInitialized() {
	return FlagInitialized;
}

int Glance::GlanceMgr::InitializeRecorder( std::string UserDirectory ) {
	bool ret;
	if ( !recorder.isOpen() ) {
		ret  = recorder.Open( UserDirectory, true, false, false, false, true, false, true, 0, 0, 0, {}, {} );
		DetectionParameters.FlagLog = true;
	}
	return ret;
}

void Glance::GlanceMgr::StopRecording() {
	if( recorder.isOpen() ) {
		DetectionParameters.FlagLog = false;
		recorder.Close();
	}
}


