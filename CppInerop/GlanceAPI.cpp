#include "GlanceAPI.h"
#include "GlanceMgr.h"
#include <iostream>

using namespace Glance;

struct GlanceAPI::impl{
	GlanceMgr mgr;
};

GlanceAPI::GlanceAPI() {
	pImpl = std::unique_ptr<impl>( new GlanceAPI::impl() );
}

GlanceAPI::~GlanceAPI() {
}

int GlanceAPI::GetFrame( cv::Mat & frame, GlanceResult &Result ) {
	return pImpl->mgr.GetFrame( frame, Result);
}

cv::Rect Glance::GlanceAPI::GetFaceRect() {
	return pImpl->mgr.GetFaceRect();
}


void Glance::GlanceAPI::SetFlags( GlanceDetectionFlags DetectionFlags ) {
	pImpl->mgr.DetectionParameters = DetectionFlags;
}

void Glance::GlanceAPI::ToggleVisualization() {
	pImpl->mgr.DetectionParameters.FlagVisualized = !pImpl->mgr.DetectionParameters.FlagVisualized;
}

bool GlanceAPI::IsInitialized() {
	return pImpl->mgr.IsInitialized();
}

int GlanceAPI::Init( std::string RootDirectory, std::string & LogMsg ) {
	return pImpl->mgr.Init( RootDirectory, LogMsg );
}

int Glance::GlanceAPI::InitializeRecorder( std::string UserDirectory ) {
	return pImpl->mgr.InitializeRecorder( UserDirectory );
}

void Glance::GlanceAPI::StopRecorder() {
	pImpl->mgr.StopRecording();
}

void GlanceAPI::Close() {
	pImpl->mgr.Close();
	pImpl.release();
	
}

cv::Size GlanceAPI::GetFrameSize() {
	return pImpl->mgr.GetFrameSize();
}
