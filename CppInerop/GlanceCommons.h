#pragma once

#include <opencv2/opencv.hpp>

struct GlanceResult {
	cv::Point3f GazeLeft, GazeRight;
	cv::Vec2f GazeAngle = { 0.f, 0.f };
	cv::Vec6f HeadPose = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
};

struct GlanceDetectionFlags {
	bool FlagVisualized = false;
	bool FlagHeadPose = true;
	bool FlagGaze = true;
	bool FlagLog = false;
};