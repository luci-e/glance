// Fill out your copyright notice in the Description page of Project Settings.

#include "GlanceHelper.h"
#include "GlanceGlobals.h"

#include "GlanceAPI.h"
#include <opencv2/opencv.hpp>
#include <opencv2/world.hpp>

UGlanceHelper::UGlanceHelper()
{

}

UGlanceHelper::~UGlanceHelper()
{
}

int UGlanceHelper::InitializeWebcam() {

	std::string StdLog;

	if ( initialized ) { return 0; }


	if ( GlanceManager.Init( "C:/Users/lucie/source/unreal/Glance/Binaries/Win64/Glance.exe", StdLog ) == 0 ) {
		initialized = true;

		cv::Size _FrameSize = GlanceManager.GetFrameSize();

		Width = _FrameSize.width;
		Height = _FrameSize.height;

		LogMsg = STR_TO_FSTR( StdLog );

		return 0;
	}

	LogMsg = STR_TO_FSTR( StdLog );

	return -1;

}

int UGlanceHelper::InitializeCanvas() {

	if ( initialized ) {
		drawing = true;

		materialHelper = NewObject<UDynamicMaterialHelper>();
		materialHelper->InitializeCanvas( Width, Height );

		GlanceManager.ToggleVisualization();

		return 0;
	}

	return -1;
}

int UGlanceHelper::InitializeRecorder( const FString &UserDirectory, const FString &OutputFile ) {
	GlanceManager.InitializeRecorder( FSTR_TO_STR(FPaths::Combine( UserDirectory, OutputFile ) ) );
	return 0;
}

void UGlanceHelper::StopRecording() {
	return GlanceManager.StopRecorder();
}

void UGlanceHelper::SnapUserImage( cv::Mat &Face ) {
	if ( !ThreadRunning ) {
		ProcessNextFrame();
		cv::Rect FaceRect = GlanceManager.GetFaceRect();
		Face = Frame( FaceRect );
	}
}

int UGlanceHelper::ProcessNextFrame() {
	return GlanceManager.GetFrame( Frame, LastResult );
}

int UGlanceHelper::Close() {
	if ( initialized ) {
		initialized = false;
		StopThread();
		
		if ( WorkerThread != nullptr ) {
			WorkerThread->WaitForCompletion();
		}

		GlanceManager.Close();
		GlanceManager.~GlanceAPI();

		UE_LOG( LogTemp, Warning, TEXT( "Closing Webcam" ) );

		return 0;
	}

	return -1;
}

void UGlanceHelper::LoopFunction() {
	if ( initialized ) {
		ProcessNextFrame();
		if ( CustomLoopFunction != nullptr ) {
			CustomLoopFunction( Frame, LastResult );
		}
	}

}

bool PointInQuad( FVector Point, FVector TopLeft, FVector TopRight, FVector BottomRight, FVector BottomLeft ) {

	FVector TL2TR = TopRight - TopLeft;
	FVector TR2BR = BottomRight - TopRight;
	FVector BR2BL = BottomLeft - BottomRight;
	FVector BL2TL = TopLeft - BottomRight;

	FVector TL = FVector::CrossProduct( TL2TR, Point - TopLeft );
	FVector TR = FVector::CrossProduct( TR2BR, Point - TopRight );
	FVector BR = FVector::CrossProduct( BR2BL, Point - BottomRight );
	FVector BL = FVector::CrossProduct( BL2TL, Point - BottomLeft );


	if ( TL.Z >= 0 && TR.Z >= 0 && BR.Z >= 0 && BL.Z >= 0 ) {
		return true;
	}

	return false;
}

FVector2D UGlanceHelper::GetUVCoordinates() {
	FVector2D angle = GetCurrentAngle();
	
	FVector Point = FVector(angle.X, angle.Y, 0);

	FVector TopLeft3;
	FVector TopRight3;
	FVector BotRight3;
	FVector BotLeft3;

	float MiddleX = (TopMid.X + MiddleMid.X + BottomMid.X) / 3.f;
	float MiddleY = (MiddleLeft.Y + MiddleMid.Y + MiddleRight.Y) / 3.f;

	float u = 0.f;
	float v = 0.f;

	if ( angle.X <= MiddleX ) {

		u = 0.5f;

		if ( angle.Y <= MiddleY ) {
			TopLeft3 = FVector( TopMid.X, TopMid.Y, 0.f );
			TopRight3 = FVector( TopRight.X, TopRight.Y, 0.f );
			BotRight3 = FVector( MiddleRight.X, MiddleRight.Y, 0.f );
			BotLeft3 = FVector( MiddleMid.X, MiddleMid.Y, 0.f );


		} else {
			TopLeft3 = FVector( MiddleMid.X, MiddleMid.Y, 0.f );
			TopRight3 = FVector( MiddleRight.X, MiddleRight.Y, 0.f );
			BotRight3 = FVector( BottomRight.X, BottomRight.Y, 0.f );
			BotLeft3 = FVector( BottomMid.X, BottomMid.Y, 0.f );

			v = 0.5f;
		}

	} else {

		if ( angle.Y <= MiddleY ) {
			TopLeft3 = FVector( TopLeft.X, TopLeft.Y, 0.f );
			TopRight3 = FVector( TopMid.X, TopMid.Y, 0.f );
			BotRight3 = FVector( MiddleMid.X, MiddleMid.Y, 0.f );
			BotLeft3 = FVector( MiddleLeft.X, MiddleLeft.Y, 0.f );

		} else {
			TopLeft3 = FVector( MiddleLeft.X, MiddleLeft.Y, 0.f );
			TopRight3 = FVector( MiddleMid.X, MiddleMid.Y, 0.f );
			BotRight3 = FVector( BottomMid.X, BottomMid.Y, 0.f );
			BotLeft3 = FVector( BottomLeft.X, BottomLeft.Y, 0.f );

			v = 0.5f;
		}

	}

	float distTop = FMath::PointDistToSegment( Point, TopLeft3, TopRight3 );
	float distRight = FMath::PointDistToSegment( Point, TopRight3, BotRight3);
	float distBot = FMath::PointDistToSegment(Point, BotRight3, BotLeft3 );
	float distLeft = FMath::PointDistToSegment( Point, BotLeft3, TopLeft3 );

	u += ( distLeft / (distLeft + distRight) )  / 2.f ;
	v += ( distTop / ( distTop + distBot ) )  / 2.f;

	return FVector2D( u, v );
}

void UGlanceHelper::StartThread() {
	if ( stopped ) {
		stopped = false;
	} else {
		StopThread();
	}

	WorkerThread = FRunnableThread::Create( this, TEXT( "OpenFaceThread" ) );

}

void UGlanceHelper::StopThread() {
	Stop();
	if ( WorkerThread != nullptr ) {
		WorkerThread->WaitForCompletion();
	}
}

FVector2D UGlanceHelper::GetCurrentAngle() {
	cv::Vec2f &GazeAngle = LastResult.GazeAngle;
	return FVector2D(GazeAngle[0], GazeAngle[1]);
}

FVector UGlanceHelper::GetHeadRotation() {
	cv::Vec6d Pose = LastResult.HeadPose;
	return FVector( Pose[3], Pose[4], Pose[5] );
}

bool UGlanceHelper::Init() {
	return true;
}

uint32 UGlanceHelper::Run() {
	ThreadRunning = true;

	while ( !stopped ) {
		LoopFunction();
	}

	ThreadRunning = false;
	return uint32();
}

void UGlanceHelper::Stop() {
	stopped = true;
}

void UGlanceHelper::Exit() {
}

