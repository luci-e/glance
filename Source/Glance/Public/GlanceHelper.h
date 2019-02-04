// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "GlanceAPI.h"
#include "GlanceGlobals.h"
#include <opencv2/opencv.hpp>
#include <opencv2/world.hpp>

#include "CoreMinimal.h"
#include "Core/Public/HAL/Runnable.h"
#include "Core/Public/HAL/RunnableThread.h"
#include "Object.h"
#include "DynamicMaterialHelper.h"
#include "GlanceHelper.generated.h"

/**
 * 
 */
UCLASS( Blueprintable, BlueprintType )
class GLANCE_API UGlanceHelper : public UObject, public FRunnable
{
	GENERATED_BODY()

public:

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		FVector2D TopLeft;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		FVector2D TopMid;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		FVector2D TopRight;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		FVector2D MiddleLeft;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		FVector2D MiddleMid;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		FVector2D MiddleRight;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		FVector2D BottomLeft;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		FVector2D BottomMid;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		FVector2D BottomRight;


	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		bool initialized = false;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		bool stopped = true;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		bool drawing = false;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		bool ThreadRunning = false;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		UDynamicMaterialHelper *materialHelper;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		FString LogMsg;


	UFUNCTION( BlueprintCallable, Category = GlanceTools )
		int InitializeWebcam();
	UFUNCTION( BlueprintCallable, Category = GlanceTools )
		int InitializeCanvas();
	UFUNCTION( BlueprintCallable, Category = GlanceTools )
		int InitializeRecorder(const FString &UserDirectory, const FString &OutputFile = TEXT( "trace.csv" ) );

	UFUNCTION( BlueprintCallable, Category = GlanceTools )
		void LoopFunction();
	UFUNCTION( BlueprintCallable, Category = GlanceTools )
		FVector2D GetUVCoordinates();
	UFUNCTION( BlueprintCallable, Category = GlanceTools )
		int Close();
	UFUNCTION( BlueprintCallable, Category = GlanceTools )
		int ProcessNextFrame();
	UFUNCTION( BlueprintCallable, Category = GlanceTools )
		void StopRecording();

	UFUNCTION( BlueprintCallable, Category = GlanceTools )
		void StartThread();
	UFUNCTION( BlueprintCallable, Category = GlanceTools )
		void StopThread();

	UFUNCTION( BlueprintCallable, Category = GlanceTools )
		FVector2D GetCurrentAngle();
	UFUNCTION( BlueprintCallable, Category = GlanceTools )
		FVector GetHeadRotation();

public:
	cv::Mat Frame;
	cv::Mat Face;

	int Width, Height;
	GlanceResult LastResult;
	std::function<void( cv::Mat&, GlanceResult )> CustomLoopFunction = nullptr;

	UGlanceHelper();
	~UGlanceHelper();

	void SnapUserImage( cv::Mat &Face );

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

private:

	Glance::GlanceAPI GlanceManager;
	FRunnableThread * WorkerThread = nullptr;
};
