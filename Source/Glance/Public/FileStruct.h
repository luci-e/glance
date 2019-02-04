// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "FileStruct.generated.h"

/**
 * 
 */
UCLASS( Blueprintable, BlueprintType )
class GLANCE_API UFileStruct : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		FString Path;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		FString Name;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Variables )
		bool IsFile;

	UFileStruct() {};
	
	static UFileStruct* ConstructFileStructure( FString path, FString name, bool isFile ) {
		UFileStruct* FileStruct = NewObject<UFileStruct>();
		FileStruct->Path = path;
		FileStruct->Name = name;
		FileStruct->IsFile = isFile;

		return FileStruct;
	}
};
