// Fill out your copyright notice in the Description page of Project Settings.

#include "EnrollHelper.h"
#include <Runtime/Core/Public/Misc/Paths.h>
#include <Runtime/Core/Public/Misc/Guid.h>
#include <Runtime/Core/Public/HAL/PlatformFilemanager.h>
#include <Runtime/Core/Public/Async/AsyncWork.h>
#include <Runtime/Core/Public/Async/Async.h>
#include <Runtime/Core/Public/Math/RandomStream.h>
#include <Runtime/Core/Public/Misc/DateTime.h>

#include "GlanceGlobals.h"

UEnrollHelper::UEnrollHelper()
{
	GlanceHelper = NewObject<UGlanceHelper>();
	RecognizerHelper = NewObject<URecognizerHelper>();
	Rng = FRandomStream( FDateTime::UtcNow().GetMillisecond() );
}

UEnrollHelper::~UEnrollHelper()
{
}

void UEnrollHelper::AddImagePath( FString Path ) {
	FilePaths.insert( Path );
}

void UEnrollHelper::RemoveImage( FString imagePath) {
	FilePaths.erase( imagePath );
}

bool UEnrollHelper::CreateUser() {

	if ( !UserCreated ) {
		User = NewObject<UGlanceUser>();
		User->NewUser( Username );
		User->ImgShowPeriod = ImgShowPeriod;

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		// Copy the images to the user directory
		for ( auto imgPath : FilePaths ) {
			PlatformFile.CopyFile( *FPaths::Combine( User->ImagesDirectory, FPaths::GetCleanFilename( imgPath ) ), *imgPath );
		}

		AsyncTask( ENamedThreads::AnyBackgroundThreadNormalTask, [=]() { 
			UserCreated = true;

			LoadImages();
			GlanceHelper->InitializeRecorder( User->TracesDirectory );

			cv::Mat ReadyImg = cv::imread( FSTR_TO_STR( FPaths::Combine( ContentDirectory, TEXT( "Images/Ready.jpg" ) ) ) );
			materialHelper->DrawFrame( ReadyImg );

		} );

		return true;
	}

	return false;
}

void UEnrollHelper::StartLoading() {
	materialHelper = NewObject<UDynamicMaterialHelper>();
	materialHelper->InitializeCanvas( BillboardWidth, BillboardHeight );

}

void UEnrollHelper::LoadImages() {
	if ( UserCreated ) {
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		TArray< FString > UserImages;

		PlatformFile.FindFiles( UserImages, *(User->ImagesDirectory), NULL );
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
			bool LeftOrRight = FMath::RandBool();

			std::string UserPath = std::string( TCHAR_TO_UTF8( *UserImages[i] ) );
			std::string NeutralPath = std::string( TCHAR_TO_UTF8( *NeutralImages[*(NeutralImagesIterator++)] ) );

			cv::Mat Left = cv::imread( LeftOrRight ? UserPath : NeutralPath );
			cv::Mat Right = cv::imread( LeftOrRight ? NeutralPath : UserPath );

			cv::Mat LeftResized;
			cv::Mat RightResized;
			
			cv::resize( Left, LeftResized, cv::Size( BillboardWidth / 2, BillboardHeight ) );
			cv::resize( Right, RightResized, cv::Size( BillboardWidth / 2, BillboardHeight ) );

			cv::hconcat( LeftResized, RightResized , CurrentImage );

			// Save the selection in the user
			User->ImagePaths.Add( *UserImages[i] );
			User->UserImagesPositions.Add( LeftOrRight );
		}


	}
}

bool UEnrollHelper::InitializeEnrollment() {

	// Class to carry out the loading task asynch. without slowing the game thread
	class FEnrollLoaderTask : public FNonAbandonableTask {
		friend class FAutoDeleteAsyncTask<FEnrollLoaderTask>;

	public:
		UEnrollHelper *Enroller;

		FEnrollLoaderTask( UEnrollHelper *Helper ) : Enroller( Helper ) {};
		void DoWork() {

			cv::Mat LoadingImg = cv::imread( FSTR_TO_STR( FPaths::Combine( ContentDirectory, TEXT( "Images/Loading.jpg" ) ) ) );
			Enroller->materialHelper->DrawFrame( LoadingImg );

			InitializeLabelsMap();

			Enroller->GlanceHelper->InitializeWebcam();

			Enroller->RecognizerHelper->InitRecognizer();

			Enroller->EnrollmentInitialized = true;
		}

		FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT( FEnrollLoaderTask, STATGROUP_ThreadPoolAsyncTasks ); }
	};

	if ( !HelpersLoaded ) {
		HelpersLoaded = true;
		(new FAutoDeleteAsyncTask<FEnrollLoaderTask>( this ))->StartBackgroundTask();
		return true;
	}

	return false;
}

void UEnrollHelper::StartRecording() {
	GlanceHelper->StartThread();
}

void UEnrollHelper::StopRecording() {
	GlanceHelper->StopRecording();
	GlanceHelper->StopThread();

	cv::Mat FaceSnapsImage = cv::imread( FSTR_TO_STR( FPaths::Combine( ContentDirectory, TEXT( "Images/LookHere.png" ) ) ) );
	materialHelper->DrawFrame( FaceSnapsImage );
}

bool UEnrollHelper::SnapUserImage() {
	if ( UserCreated ) {
		cv::Mat Face;
		GlanceHelper->SnapUserImage( Face );

		if ( !(Face.rows == 0) ) {

			std::string SaveFile = FSTR_TO_STR( FPaths::Combine( User->FacesDirectory, FString::FromInt( UserSnaps ) + FString( TEXT( ".png" ) ) ) );

			cv::Mat Resized;
			cv::resize( Face, Resized, FaceImageSize );

			cv::Mat GrayScaled;
			cv::cvtColor( Resized, GrayScaled, cv::COLOR_BGR2GRAY );
			FaceImages.push_back( GrayScaled );

			cv::imwrite( SaveFile, GrayScaled );

			return true;
		}
	}

	return false;
}

void UEnrollHelper::Close() {
	GlanceHelper->Close();
	GlanceHelper->~UGlanceHelper();
}

void UEnrollHelper::DrawNextImage() {
	if ( EnrollmentInitialized ) {
		materialHelper->DrawFrame( TracingImages[CurrentImageNo++ % ImagesNo] );
	}
}

void UEnrollHelper::EndEnrollment() {
	RecognizerHelper->AddUserImages( FaceImages, User->Label );
	User->SaveUserData();
	LabelsMap[User->Label] = User->Id;
	cv::Mat EnrollmentFinished = cv::imread( FSTR_TO_STR( FPaths::Combine( ContentDirectory, TEXT( "Images/EnrollComplete.png" ) ) ) );
	materialHelper->DrawFrame( EnrollmentFinished );
}