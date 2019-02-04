// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include <memory>

#include <opencv2/opencv.hpp>
#include <opencv2/world.hpp>

#include "Object.h"
#include "Engine/Texture2D.h"
#include "DynamicMaterialHelper.generated.h"


UCLASS( Blueprintable, BlueprintType )
class GLANCE_API UDynamicMaterialHelper : public UObject{
	GENERATED_BODY()

public:

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		UTexture2D* dynamicCanvas;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		bool initialized = false;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		int canvasWidth;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		int canvasHeight;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		TArray<FColor> canvasPixelData;

	UFUNCTION( BlueprintCallable, Category = DrawingTools )
		void InitializeCanvas( int width, int height );
	UFUNCTION( BlueprintCallable, Category = DrawingTools )
		void UpdateCanvas();
	UFUNCTION( BlueprintCallable, Category = DrawingTools )
		void ClearCanvas();

public:
	void DrawFrame( cv::Mat &Frame );

	UDynamicMaterialHelper();
	~UDynamicMaterialHelper();

protected:
	// Use this function to update the texture rects you want to change: 
	// NOTE: There is a method called UpdateTextureRegions in UTexture2D but it is compiled WITH_EDITOR and is not marked as ENGINE_API so it cannot be linked 
	// from plugins. 
	// FROM: https://wiki.unrealengine.com/Dynamic_Textures 
	inline void UpdateTextureRegions( UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData );

private:
	int bytesPerPixel;
	int bufferPitch;
	int bufferSize;

	std::unique_ptr<FUpdateTextureRegion2D> echoUpdateTextureRegion;
	inline void setPixelColor( uint32 index, FColor color );
};