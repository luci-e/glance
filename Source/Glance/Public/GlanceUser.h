// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "DynamicMaterialHelper.h"
#include <Runtime/Core/Public/Misc/Guid.h>
#include <list>
#include "GlanceUser.generated.h"

/**
*
*/
UCLASS( Blueprintable, BlueprintType )
class GLANCE_API UGlanceUser : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Variables )
		FString Username = "";
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		FGuid Id;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		int Label;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		float ImgShowPeriod;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		FString UserDirectory;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		FString ImagesDirectory;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		FString FacesDirectory;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		FString TracesDirectory;


public:
	
	TArray <FString> ImagePaths;
	TArray <bool> UserImagesPositions;
	
	UGlanceUser();

	int NewUser( FString name );
	int SaveUserData();

	static UGlanceUser* LoadUserData( FGuid UserId );
	static FString GetUsername( FGuid UserId );

	static int CreateLabel( FGuid UserId );

	~UGlanceUser();
};
