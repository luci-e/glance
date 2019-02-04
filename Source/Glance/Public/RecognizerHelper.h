// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "dtwlib/vector_dtw/vectordtw.h"
#include <opencv2/face.hpp>
#include "GlanceHelper.h"
#include "GlanceGlobals.h"
#include "GlanceUser.h"
#include "GlanceUserStruct.h"
#include "DynamicMaterialHelper.h"
#include <Runtime/Core/Public/Misc/Guid.h>
#include <Runtime/Core/Public/Math/RandomStream.h>
#include "RecognizerHelper.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FHelpersLoadedEvent );

/**
 * 
 */
UCLASS( Blueprintable, BlueprintType )
class GLANCE_API URecognizerHelper : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		bool RecognizerInitialized = false;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		bool HelpersLoaded = false;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		float FaceThreshold = 0.f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		float GazeThreshold = 0.f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		float ImgShowPeriod = 0.f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		int ImagesNo = 0;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		int CurrentImageNo = 0;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		bool ImagesLoaded = false;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		bool TracingDone = false;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		bool Tracing = false;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		UDynamicMaterialHelper *materialHelper;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		UGlanceHelper *GlanceHelper;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		UGlanceUser *User;


	UFUNCTION( BlueprintCallable, Category = RecognizerTools )
		int InitRecognizer();
	UFUNCTION( BlueprintCallable, Category = RecognizerTools )
		int SetRecognizedUser( FGuid UserId );
	UFUNCTION( BlueprintCallable, Category = RecognizerTools )
		bool LoadHelpers();
	UFUNCTION( BlueprintCallable, Category = RecognizerTools )
		bool TryFaceRecognition();
	UFUNCTION( BlueprintCallable, Category = RecognizerTools )
		float GetLastConfidence();
	UFUNCTION( BlueprintCallable, Category = RecognizerTools )
		float GetLastTraceDistance();
	UFUNCTION( BlueprintCallable, Category = RecognizerTools )
		void DrawNextImage();
	UFUNCTION( BlueprintCallable, Category = RecognizerTools )
		bool StartRecording();
	UFUNCTION( BlueprintCallable, Category = RecognizerTools )
		bool StopRecording();
	UFUNCTION( BlueprintCallable, Category = RecognizerTools )
		void ComputeSimilarity();
	UFUNCTION( BlueprintCallable, Category = RecognizerTools )
		void EndRecognition();


	UPROPERTY( BlueprintAssignable, Category = RecognizerTools )
		FHelpersLoadedEvent RecognizerLoaded;

public:
	URecognizerHelper();
	~URecognizerHelper();

	FRandomStream Rng;

	cv::Ptr<cv::face::FaceRecognizer> Recognizer;
	std::string RecognizerPath;
	TArray< cv::Mat > TracingImages;

	std::vector< Point > CurrentUserTrace;
	std::vector< Point > RecordedUserTrace;
	void ResizeInterpolation( std::vector<Point> &Series, std::vector<Point>& NewSeries, uint NewSize );

	cv::Mat LastFace;
	int LastPredictionLabel;
	double LastPredictionConfidence;
	double LastTraceDistance;
	FGuid LastPredictionId;

	FGuid LastPredictionTraceId;

	int AddUserImages( std::vector<cv::Mat> &Images, int label );
	void LoadImages();
	void LoadUserTrace();

};