// Fill out your copyright notice in the Description page of Project Settings.


#include "FileSystemExplorerHelper.h"
#include <Runtime/Core/Public/HAL/FileManager.h>
#include <Runtime/Core/Public/GenericPlatform/GenericPlatformFile.h>
#include "FileStruct.h"

TArray<UFileStruct*> UFileSystemExplorerHelper::GetCurrentDirectoryContent() {
	IFileManager& FileManager = IFileManager::Get();
	TArray<UFileStruct*> Found;

	TArray<FString> Directories;
	TArray<FString> Files;

	FString Selector = TEXT("*");

	FileManager.FindFiles( Directories, *(CurrentDirectory + Selector), false, true );
	FileManager.FindFiles( Files, *(CurrentDirectory + Selector), true, false );

	Found.Reserve( Directories.Num() + Files.Num() );

	Found.Add( UFileStruct::ConstructFileStructure( FPaths::Combine( CurrentDirectory, FString(TEXT("..") ) ), FString( TEXT( ".." ) ), false ) );

	for ( auto Dir : Directories ) {
		Found.Add( UFileStruct::ConstructFileStructure( FPaths::Combine(CurrentDirectory, Dir), Dir, false ) );
	}

	for ( auto File : Files) {
		Found.Add( UFileStruct::ConstructFileStructure( FPaths::Combine( CurrentDirectory, File ), File, true ) );
	}

	return Found;
}

void UFileSystemExplorerHelper::ChangeDirectory( FString Directory ) {
	if ( Directory == TEXT( ".." ) ) {
		CurrentDirectory = FPaths::ConvertRelativePathToFull( CurrentDirectory + Directory + TEXT( "/" ) );
	} else {
		CurrentDirectory = FPaths::Combine( CurrentDirectory, Directory ) + TEXT("/");
	}
}

TArray<UFileStruct*> UFileSystemExplorerHelper::ChangeDirectoryAndGet( FString Directory ) {
	ChangeDirectory( Directory );
	FPaths::CollapseRelativeDirectories( CurrentDirectory );
	return GetCurrentDirectoryContent();
}
