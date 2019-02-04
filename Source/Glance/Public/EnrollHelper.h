// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <set>
#include <memory>
#include <list>
#include "CoreMinimal.h"
#include "Object.h"
#include "GlanceHelper.h"
#include "RecognizerHelper.h"
#include "DynamicMaterialHelper.h"
#include "GlanceGlobals.h"
#include "GlanceUser.h"
#include <Runtime/Core/Public/Containers/Array.h>
#include <Runtime/Core/Public/Misc/Guid.h>
#include <Runtime/Core/Public/Math/RandomStream.h>
#include <opencv2/opencv.hpp>
#include "EnrollHelper.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FLoadCompleteEvent );

/**
 * 
 */
UCLASS( Blueprintable, BlueprintType )
class GLANCE_API UEnrollHelper : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		FString Username;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		bool UserCreated = false;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		bool EnrollmentInitialized = false;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		bool HelpersLoaded = false;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		int ImagesNo = 0;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		int CurrentImageNo = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		int UserSnaps = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		float ImgShowPeriod;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		UDynamicMaterialHelper *materialHelper;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		UGlanceHelper *GlanceHelper;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		URecognizerHelper *RecognizerHelper;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		UGlanceUser * User;

	UFUNCTION( BlueprintCallable, Category = EnrollTools )
		void AddImagePath(FString Path);
	UFUNCTION( BlueprintCallable, Category = EnrollTools )
		void RemoveImage( FString imagePath );
	UFUNCTION( BlueprintCallable, Category = EnrollTools )
		bool CreateUser();
	UFUNCTION( BlueprintCallable, Category = EnrollTools )
		void StartLoading();
	UFUNCTION( BlueprintCallable, Category = EnrollTools )
		bool InitializeEnrollment();
	UFUNCTION( BlueprintCallable, Category = EnrollTools )
		void StartRecording();
	UFUNCTION( BlueprintCallable, Category = EnrollTools )
		void StopRecording();
	UFUNCTION( BlueprintCallable, Category = EnrollTools )
		bool SnapUserImage();
	UFUNCTION( BlueprintCallable, Category = EnrollTools )
		void Close();
	UFUNCTION( BlueprintCallable, Category = EnrollTools )
		void DrawNextImage();
	UFUNCTION( BlueprintCallable, Category = EnrollTools )
		void EndEnrollment();

	UPROPERTY( BlueprintAssignable, Category = EnrollTools )
		FLoadCompleteEvent LoadComplete;


public:
	std::set<FString> FilePaths;

	FRandomStream Rng;
	TArray< cv::Mat > TracingImages;
	std::vector< cv::Mat > FaceImages;

	UEnrollHelper();
	~UEnrollHelper();

	void LoadImages();
};
