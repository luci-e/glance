#pragma once

#include <opencv2/world.hpp>
#include <opencv2/opencv.hpp>

#include <LandmarkCoreIncludes.h>
#include <VisualizationUtils.h>
#include <SequenceCapture.h>
#include <Visualizer.h>
#include <RecorderCSV.h>

#include "GlanceCommons.h"

namespace Glance {

	class GlanceMgr {

	private:
		LandmarkDetector::FaceModelParameters det_parameters;
		Utilities::SequenceCapture sequence_reader;

		// The modules that are being used for tracking
		LandmarkDetector::CLNF face_model;

		// A utility for visualizing the results (show just the tracks)
		Utilities::Visualizer visualizer = Utilities::Visualizer( true, false, false, false );

		// Tracking FPS for visualization
		Utilities::FpsTracker fps_tracker;

		// Recorder
		Utilities::RecorderCSV recorder;

		// Face id, assumed to be unique for manager
		int face_id;

	public:
		bool FlagInitialized = false;
		bool Stopped = true;

		GlanceDetectionFlags DetectionParameters;

	public:
		GlanceMgr();
		~GlanceMgr();

		void LogPrint( std::string & buffer, std::string message, uint level );
		int Init( std::string RootDirectory, std::string &LogMsg );

		void Close();

		cv::Size GetFrameSize();

		int GetFrame( cv::Mat &frame, GlanceResult &Result );
		cv::Rect GetFaceRect();
		bool IsInitialized();

		int InitializeRecorder( std::string UserDirectory );
		void StopRecording();
	};
}
