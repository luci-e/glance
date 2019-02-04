// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <Runtime/Core/Public/Misc/Paths.h>
#include "FileStruct.h"
#include "FileSystemExplorerHelper.generated.h"

/**
 * 
 */
UCLASS( Blueprintable, BlueprintType )
class GLANCE_API UFileSystemExplorerHelper : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		FString CurrentDirectory = FPaths::ProjectDir();

	UFUNCTION( BlueprintCallable, Category = ExplorerTools )
		TArray<UFileStruct*> GetCurrentDirectoryContent();
	UFUNCTION( BlueprintCallable, Category = ExplorerTools )
		void ChangeDirectory(FString Directory);
	UFUNCTION( BlueprintCallable, Category = ExplorerTools )
		TArray<UFileStruct*> ChangeDirectoryAndGet( FString Directory );
	
};
