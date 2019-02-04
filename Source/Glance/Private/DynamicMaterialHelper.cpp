// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicMaterialHelper.h"

#include <opencv2/opencv.hpp>
#include <opencv2/world.hpp>

UDynamicMaterialHelper::UDynamicMaterialHelper() {
}

UDynamicMaterialHelper::~UDynamicMaterialHelper() {
	echoUpdateTextureRegion.release();
}

void UDynamicMaterialHelper::InitializeCanvas(int width, int height) {
	if ( !initialized ) {
		canvasWidth = width;
		canvasHeight = height;

		dynamicCanvas = UTexture2D::CreateTransient( canvasWidth, canvasHeight );
#if WITH_EDITORONLY_DATA
		dynamicCanvas->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
		dynamicCanvas->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
		dynamicCanvas->SRGB = false;
		dynamicCanvas->AddToRoot();
		dynamicCanvas->Filter = TextureFilter::TF_Nearest;
		dynamicCanvas->UpdateResource();

		echoUpdateTextureRegion = std::unique_ptr<FUpdateTextureRegion2D>( new FUpdateTextureRegion2D( 0, 0, 0, 0, canvasWidth, canvasHeight ) );

		// buffers initialization
		bytesPerPixel = 4; // r g b a
		bufferPitch = canvasWidth * bytesPerPixel;
		bufferSize = canvasWidth * canvasHeight * bytesPerPixel;
		canvasPixelData.Init( FColor( 0, 0, 0, 255 ), canvasWidth * canvasHeight );

		ClearCanvas();

		initialized = true;
	}
}

void UDynamicMaterialHelper::setPixelColor( uint32 index, FColor color ) {
	canvasPixelData[index] = color;
}

void UDynamicMaterialHelper::ClearCanvas() {
	for ( int row = 0; row < canvasHeight; row++ ) {
		for ( int col = 0; col < canvasWidth; col++ ) {
			int index = canvasWidth * row + col;
			canvasPixelData[index] = FColor{ 0, 255, 0, 255 };
		}
	}
	UpdateCanvas();
}

void UDynamicMaterialHelper::UpdateCanvas() {
	if ( echoUpdateTextureRegion ) {
		UpdateTextureRegions( dynamicCanvas, ( int32 ) 0, ( uint32 ) 1, echoUpdateTextureRegion.get(), ( uint32 ) bufferPitch, ( uint32 ) bytesPerPixel, ( uint8* ) canvasPixelData.GetData(), false );
	}
}

void UDynamicMaterialHelper::UpdateTextureRegions( UTexture2D * Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D * Regions, uint32 SrcPitch, uint32 SrcBpp, uint8 * SrcData, bool bFreeData ) {
	if ( Texture->Resource ) {

		struct FUpdateTextureRegionsData {
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = ( FTexture2DResource* ) Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER( UpdateTextureRegionsData,
													FUpdateTextureRegionsData*,
													RegionData,
													RegionData,
													bool,
													bFreeData,
													bFreeData,
													{
														for ( uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex ) {
															int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
															if ( RegionData->MipIndex >= CurrentFirstMip ) {
																RHIUpdateTexture2D( RegionData->Texture2DResource->GetTexture2DRHI(),
																					RegionData->MipIndex - CurrentFirstMip,
																					RegionData->Regions[RegionIndex],
																					RegionData->SrcPitch,
																					RegionData->SrcData + RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch + RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
																);
															}
														} if ( bFreeData ) {
															FMemory::Free( RegionData->Regions );
															FMemory::Free( RegionData->SrcData );
														} delete RegionData;
													}
		);
	}

}

void UDynamicMaterialHelper::DrawFrame(cv::Mat &Frame) {

	cv::Mat Resized;
	uint8 *_FrameData;

	if ( !((Frame.cols == canvasWidth) && (Frame.rows == canvasHeight)) ) {
		cv::resize( Frame, Resized, cv::Size( canvasWidth, canvasHeight) );
		_FrameData = Resized.data;
	} else {
		_FrameData = Frame.data;
	}

	for ( int row = 0; row < canvasHeight; row++ ) {
		for ( int col = 0; col < canvasWidth; col++ ) {
			uint32 index = canvasWidth * row + col;
			uint32 i_3 = index * 3;
			setPixelColor( index, FColor{ *(_FrameData + i_3 + 2), *(_FrameData + i_3 + 1) , *(_FrameData + i_3 + 0), 255 } );
		}
	}
	UpdateCanvas();
}

