// Fill out your copyright notice in the Description page of Project Settings.

#include "RecognizerHelper.h"
#include "GlanceGlobals.h"
#include "csvparser/csv.h"
#include <Runtime/Core/Public/Misc/Paths.h>
#include <Developer/DesktopPlatform/Public/IDesktopPlatform.h>
#include <Developer/DesktopPlatform/Public/DesktopPlatformModule.h>
#include <Runtime/Core/Public/Misc/FileHelper.h>
#include <Runtime/Core/Public/HAL/PlatformFilemanager.h>
#include <opencv2/face.hpp>
#include <Runtime/Core/Public/Async/AsyncWork.h>
#include <Runtime/Core/Public/Async/Async.h>
#include <Runtime/Core/Public/Misc/DateTime.h>
#include "GlanceUserStruct.h"

URecognizerHelper::URecognizerHelper() {
	materialHelper = NewObject<UDynamicMaterialHelper>();
	GlanceHelper = NewObject<UGlanceHelper>();
	Rng = FRandomStream( FDateTime::UtcNow().GetMillisecond() );
}

URecognizerHelper::~URecognizerHelper() {
}



int URecognizerHelper::InitRecognizer() {

	if ( !RecognizerInitialized ) {
		RecognizerInitialized = true;

		InitializeLabelsMap();

		RecognizerPath = FSTR_TO_STR( FPaths::Combine( ResourcesDirectory, FString( TEXT( "FaceRecognizerModel.xml" ) ) ) );
		cv::String cvRecognizerPath = cv::String( RecognizerPath );

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		if ( PlatformFile.FileExists( *STR_TO_FSTR( RecognizerPath ) ) ) {
			Recognizer = Recognizer->load< cv::face::LBPHFaceRecognizer >( cvRecognizerPath );
		} else {
			Recognizer = cv::face::LBPHFaceRecognizer::create();
		}
		
		materialHelper->InitializeCanvas( BillboardWidth, BillboardHeight );


		return 0;
	}

	return -1;
}

int URecognizerHelper::SetRecognizedUser( FGuid UserId ) {

	if ( HelpersLoaded ) {

		LastPredictionId = UserId;

		ImagesLoaded = false;
		User = UGlanceUser::LoadUserData( UserId );
		ImgShowPeriod = User->ImgShowPeriod;

		LastPredictionLabel = UGlanceUser::CreateLabel( UserId );

		AsyncTask( ENamedThreads::AnyNormalThreadNormalTask, [=]() {
			LoadImages();
			LoadUserTrace();
		} );

		return 0;
	}

	return -1;
}

bool URecognizerHelper::LoadHelpers() {
	if ( !HelpersLoaded ) {
		// Class to carry out the loading task asynch. without slowing the game thread
		class FRecognizerLoaderTask : public FNonAbandonableTask {
			friend class FAutoDeleteAsyncTask<FRecognizerLoaderTask>;

		public:
			URecognizerHelper *Recognizer;

			FRecognizerLoaderTask( URecognizerHelper *Helper ) : Recognizer( Helper ) {};
			void DoWork() {

				cv::Mat LoadingImg = cv::imread( FSTR_TO_STR( FPaths::Combine( ContentDirectory, TEXT( "Images/Loading.jpg" ) ) ) );
				Recognizer->materialHelper->DrawFrame( LoadingImg );

				Recognizer->GlanceHelper->InitializeWebcam();

				cv::Mat ReadyImg = cv::imread( FSTR_TO_STR( FPaths::Combine( ContentDirectory, TEXT( "Images/Ready.jpg" ) ) ) );
				Recognizer->materialHelper->DrawFrame( ReadyImg );

				AsyncTask( ENamedThreads::GameThread, [=]() { Recognizer->RecognizerLoaded.Broadcast(); } );
			}

			FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT( FEnrollLoaderTask, STATGROUP_ThreadPoolAsyncTasks ); }
		};

		HelpersLoaded = true;
		(new FAutoDeleteAsyncTask<FRecognizerLoaderTask>( this ))->StartBackgroundTask();

		return true;
	}

	return false;
}

bool URecognizerHelper::TryFaceRecognition() {
	if ( HelpersLoaded ) {
		cv::Mat Face, Scaled;

		GlanceHelper->SnapUserImage( Face );

		if ( !Face.empty() ) {
			cv::resize( Face, Scaled, FaceImageSize );
			cv::cvtColor( Scaled, LastFace, cv::COLOR_BGR2GRAY );

			Recognizer->predict( LastFace, LastPredictionLabel, LastPredictionConfidence );

			if ( LastPredictionConfidence >= FaceThreshold ) {
				LastPredictionId = LabelsMap[LastPredictionLabel];

				if ( IsValid(User) ) {
					User->BeginDestroy();
				}

				ImagesLoaded = false;
				User = UGlanceUser::LoadUserData( LastPredictionId );
				ImgShowPeriod = User->ImgShowPeriod;

				AsyncTask( ENamedThreads::AnyNormalThreadNormalTask, [=]() { 
					LoadImages(); 
					LoadUserTrace();
				} );

				return true;
			}
		} else {
			LastPredictionConfidence = 0.f;
		}
	}

	return false;
}

float URecognizerHelper::GetLastConfidence() {
	return (float) LastPredictionConfidence;
}

float URecognizerHelper::GetLastTraceDistance() {
	return (float) LastTraceDistance;
}

int URecognizerHelper::AddUserImages( std::vector<cv::Mat>& Images, int label ) {
	
	if ( RecognizerInitialized ) {
		Recognizer->update( Images, std::vector<int>( Images.size(), label ) );
		Recognizer->save( RecognizerPath );
		return 0;
	}

	return -1;
}

void URecognizerHelper::LoadImages() {
	if ( IsValid(User) ) {
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		TArray< FString > UserImages = User->ImagePaths;
		ImagesNo = UserImages.Num();

		TArray< FString > NeutralImages;
		PlatformFile.FindFiles( NeutralImages, *NeutralImagesDirectory, NULL );

		int PickedElements = 0;
		std::set<int> ChosenElements;

		while ( PickedElements < ImagesNo ) {
			int NextElement = Rng.RandRange( 0, NeutralImages.Num() );
			if ( ChosenElements.find( NextElement ) == ChosenElements.end() ) {
				ChosenElements.insert( NextElement );
				PickedElements++;
			}
		}

		auto NeutralImagesIterator = ChosenElements.begin();

		TracingImages.SetNum( ImagesNo );
		for ( int i = 0; i < ImagesNo; i++ ) {
			cv::Mat &CurrentImage = TracingImages[i];
			bool LeftOrRight = User->UserImagesPositions[i];

			std::string UserPath = std::string( TCHAR_TO_UTF8( *UserImages[i] ) );
			std::string NeutralPath = std::string( TCHAR_TO_UTF8( *NeutralImages[*(NeutralImagesIterator++)] ) );

			cv::Mat Left = cv::imread( LeftOrRight ? UserPath : NeutralPath );
			cv::Mat Right = cv::imread( LeftOrRight ? NeutralPath : UserPath );

			cv::Mat LeftResized;
			cv::Mat RightResized;

			cv::resize( Left, LeftResized, cv::Size( BillboardWidth / 2, BillboardHeight ) );
			cv::resize( Right, RightResized, cv::Size( BillboardWidth / 2, BillboardHeight ) );

			cv::hconcat( LeftResized, RightResized, CurrentImage );
		}

		ImagesLoaded = true;

	}
}

void URecognizerHelper::LoadUserTrace() {
	if ( IsValid( User ) ) {
		std::string TraceFile = FSTR_TO_STR( FPaths::Combine( User->TracesDirectory, TEXT( "trace.csv" ) ) );
		io::CSVReader<2, io::trim_chars<' '>> traceReader( TraceFile.c_str() );
		traceReader.read_header( io::ignore_extra_column, "gaze_angle_x", "gaze_angle_y" );

		float GazeX, GazeY;

		while ( traceReader.read_row( GazeX, GazeY ) ) {
			CurrentUserTrace.push_back( Point{ (double) GazeX, (double) GazeY, 0} );
		}
	}
}

void URecognizerHelper::DrawNextImage() {
	if ( ImagesLoaded ) {
		materialHelper->DrawFrame( TracingImages[CurrentImageNo++ % ImagesNo] );
	}
}

bool URecognizerHelper::StartRecording() {
	if ( IsValid( User ) && !TracingDone && !Tracing ) {
		Tracing = true;

		LastPredictionTraceId = FGuid::NewGuid();
		GlanceHelper->InitializeRecorder( User->TracesDirectory, LastPredictionTraceId.ToString() + TEXT(".csv") );

		GlanceHelper->CustomLoopFunction = [this]( cv::Mat &Frame, GlanceResult Result ) {
			RecordedUserTrace.push_back( Point{ Result.GazeAngle[0], Result.GazeAngle[1], 0 } );
		};
		GlanceHelper->StartThread();
		return true;
	}

	return false;
}

bool URecognizerHelper::StopRecording() {
	if ( IsValid( User ) && !TracingDone && Tracing) {
		Tracing = false;
		GlanceHelper->StopRecording();
		GlanceHelper->StopThread();
		GlanceHelper->CustomLoopFunction = nullptr;
		TracingDone = true;

		return true;
	}
	return false;
}

void URecognizerHelper::ComputeSimilarity() {
	std::vector<Point> InterpolatedSeries;
	std::vector<Point> &LongerTrace = (CurrentUserTrace.size() >= RecordedUserTrace.size()) ? CurrentUserTrace : RecordedUserTrace;
	std::vector<Point> &ShorterTrace = (CurrentUserTrace.size() >= RecordedUserTrace.size()) ? RecordedUserTrace : CurrentUserTrace;

	ResizeInterpolation( ShorterTrace, InterpolatedSeries, LongerTrace.size() );

	VectorDTW TraceDtw( LongerTrace.size(), ( uint ) (( float ) LongerTrace.size() / 10.f) );
	LastTraceDistance = TraceDtw.fastdynamic( LongerTrace, InterpolatedSeries );

	FString RecognitionDataFile = FPaths::Combine( User->TracesDirectory, TEXT( "RecognitionData.csv" ) );

	FString LastPredictionSave = LastPredictionTraceId.ToString() + TEXT( "," ) + \
		FDateTime::UtcNow().ToString() + TEXT( "," ) + \
		FString::SanitizeFloat( ( float ) LastPredictionConfidence ) + TEXT( "," ) + \
		FString::SanitizeFloat( ( float ) LastTraceDistance ) + TEXT(",") + \
		FString::SanitizeFloat( ( float ) FaceThreshold ) + TEXT( "," ) + \
		FString::SanitizeFloat( ( float ) GazeThreshold ) + TEXT( "," ) + \
		FString::SanitizeFloat( ( float ) ImgShowPeriod ) + TEXT( "\r\n" );

	FFileHelper::SaveStringToFile( LastPredictionSave, *RecognitionDataFile, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append );

	std::string resultImagePath;
	if ( LastTraceDistance <= this->GazeThreshold ) {
		resultImagePath = FSTR_TO_STR( FPaths::Combine( ContentDirectory, TEXT( "Images/passed_recognition.png" ) ) );
	} else {
		resultImagePath = FSTR_TO_STR( FPaths::Combine( ContentDirectory, TEXT( "Images/failed_recognition.png" ) ) );
	}

	cv::Mat RecognitionResult = cv::imread( resultImagePath );
	materialHelper->DrawFrame( RecognitionResult );
}



void URecognizerHelper::ResizeInterpolation( std::vector<Point>& Series, std::vector<Point>& NewSeries, uint NewSize ) {
	if ( (Series.size() > NewSize) || (Series.size() == NewSize) ) {
		return;
	}

	uint PointsToAdd = NewSize - Series.size();
	uint PointsAdded = 0;
	uint PointsPerInterval = ( uint ) ceilf( ( float ) PointsToAdd / ( float ) (Series.size() - 1) );
	double SegmentIncrease = ( double ) (PointsPerInterval + 1);

	for ( uint i = 0; i < Series.size() - 1; i++ ) {
		NewSeries.emplace_back( Series[i] );
		if ( PointsAdded < PointsToAdd ) {
			Point PNext = Series[i + 1];
			Point PPrev = Series[i];
			Point NewPoint = PPrev;

			Point Vec = { (PNext.x - PPrev.x) / SegmentIncrease, (PNext.y - PPrev.y) / SegmentIncrease, 0 };

			for ( uint p = 0; p < PointsPerInterval; p++ ) {
				NewPoint.x += Vec.x;
				NewPoint.y += Vec.y;

				NewSeries.emplace_back( NewPoint );

				PointsAdded++;
			}
		}

	}

	NewSeries.emplace_back( Series.back() );
}

void URecognizerHelper::EndRecognition() {
	GlanceHelper->Close();
	GlanceHelper->~UGlanceHelper();

	cv::Mat RecognitionFinished = cv::imread( FSTR_TO_STR( FPaths::Combine( ContentDirectory, TEXT( "Images/RecognitionComplete.png" ) ) ) );
	materialHelper->DrawFrame( RecognitionFinished );
}


