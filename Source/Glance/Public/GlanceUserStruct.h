#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "GlanceUserStruct.generated.h"

/**
 * 
 */
UCLASS( Blueprintable, BlueprintType )
class GLANCE_API UGlanceUserStruct : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintReadWrite, Category = Variables )
		FString Username;
	UPROPERTY( BlueprintReadWrite, Category = Variables )
		FGuid Id;

	UGlanceUserStruct() {};

	void Init( FGuid id, FString username ) {
		Id = id;
		Username = username;
	}

};
