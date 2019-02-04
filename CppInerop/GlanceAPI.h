#pragma once

#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/world.hpp>
#include "GlanceCommons.h"

namespace Glance {

	class GlanceAPI {
	public:

		struct impl;  // forward declaration of the implementation class

		std::unique_ptr<impl> pImpl;

		GlanceAPI();
		~GlanceAPI();

		int Init( std::string RootDirectory, std::string &LogMsg );
		int InitializeRecorder( std::string UserDirectory );
		void StopRecorder();

		void Close();

		cv::Size GetFrameSize();

		int GetFrame( cv::Mat &frame, GlanceResult &Result);
		cv::Rect GetFaceRect();

		void SetFlags( GlanceDetectionFlags DetectionFlags );
		void ToggleVisualization();
		bool IsInitialized();
	};

}