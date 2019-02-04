#pragma once

#include "CoreMinimal.h"
#include <Runtime/Core/Public/Misc/Guid.h>
#include <Runtime/Core/Public/Misc/Paths.h>
#include <opencv2/world.hpp>
#include <map>

#define FSTR_TO_STR( fstring ) std::string(TCHAR_TO_UTF8(*fstring))
#define STR_TO_FSTR( stdstring ) FString( UTF8_TO_TCHAR( stdstring.c_str() ) )

#define GLANCE_IMAGE_LEFT 0
#define GLANCE_IMAGE_RIGHT 1

static const FString ResourcesDirectory = FPaths::Combine( FPaths::ProjectDir(), TEXT( "./Resources/" ) );
static const FString UsersDirectory = FPaths::Combine( FPaths::ProjectDir(), TEXT( "./Resources/Users" ) );
static const FString NeutralImagesDirectory = FPaths::Combine( FPaths::ProjectDir(), TEXT("./Resources/NeutralImages/"));
static const FString ContentDirectory = FPaths::Combine( FPaths::ProjectDir(), TEXT( "./Content" ) );

static const int BillboardWidth = 600 * 2;
static const int BillboardHeight = 600;

const cv::Size FaceImageSize = { 400, 400 };

extern std::map<int, FGuid> LabelsMap;

void InitializeLabelsMap();
