// Fill out your copyright notice in the Description page of Project Settings.

#include "GlanceUtils.h"
#include "GlanceGlobals.h"
#include "GlanceUser.h"
#include <Runtime/Core/Public/HAL/FileManager.h>
#include <Runtime/Core/Public/GenericPlatform/GenericPlatformFile.h>

TArray<UGlanceUserStruct*> UGlanceUtils::LoadAllUsers() {
	TArray<UGlanceUserStruct*> FoundUsers;

	IFileManager& FileManager = IFileManager::Get();

	TArray<FString> Directories;
	TArray<FString> Files;

	FString Selector = TEXT( "/*" );

	FileManager.FindFiles( Directories, *(UsersDirectory + Selector), false, true );

	FoundUsers.Reserve( Directories.Num() );

	for ( auto Dir : Directories ) {
		FGuid User;
		FGuid::Parse( Dir, User );
		UGlanceUserStruct * u = NewObject<UGlanceUserStruct>();


		u->Init( User, UGlanceUser::GetUsername( User ));
		FoundUsers.Add( u );
	}

	return FoundUsers;
}
