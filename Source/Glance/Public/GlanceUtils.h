// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlanceUserStruct.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GlanceUtils.generated.h"

/**
 * 
 */
UCLASS()
class GLANCE_API UGlanceUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		UFUNCTION( BlueprintCallable, Category = GlanceUtils )
		static TArray<UGlanceUserStruct*> LoadAllUsers();
	
};
