#include "GlanceGlobals.h"

#include <Runtime/Core/Public/Misc/Paths.h>
#include <Runtime/Core/Public/HAL/FileManager.h>
#include <Runtime/Core/Public/GenericPlatform/GenericPlatformFile.h>
#include "GlanceUser.h"
#include <Runtime/Core/Public/Misc/Guid.h>
#include <map>


std::map<int, FGuid> LabelsMap = std::map<int, FGuid>();

void InitializeLabelsMap() {
	static bool Initialized = false;

	if ( !Initialized ) {
		Initialized = true;

		TArray < FString > UsersGuids;

		IFileManager& FileManager = IFileManager::Get();
		TArray< FString > UserImages;

		FileManager.FindFiles( UsersGuids, *(FPaths::Combine( UsersDirectory, TEXT("*"))), false, true );

		for ( auto User : UsersGuids ) {
			FGuid UserGuid;
			FGuid::Parse( User, UserGuid );
			int label = UGlanceUser::CreateLabel( UserGuid );

			LabelsMap[label] = UserGuid;

		}
	}
}
