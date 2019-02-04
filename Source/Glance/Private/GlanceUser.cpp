// Fill out your copyright notice in the Description page of Project Settings.

#include "GlanceUser.h"
#include "Json.h"
#include <Runtime/Core/Public/Misc/FileHelper.h>
#include "GlanceGlobals.h"
#include <Runtime/Core/Public/Misc/Paths.h>

UGlanceUser::UGlanceUser()
{
}

int UGlanceUser::NewUser( FString name ) {
	if ( !name.IsEmpty() ) {
		Username = name;
	} else {
		Username = FGuid::NewGuid().ToString();
	}

	Id = FGuid::NewGuid();
	Label = UGlanceUser::CreateLabel( Id );

	UserDirectory = FPaths::Combine( UsersDirectory, Id.ToString() );
	ImagesDirectory = FPaths::Combine( UserDirectory, TEXT( "Images" ) );
	FacesDirectory = FPaths::Combine( UserDirectory, TEXT( "Faces" ) );
	TracesDirectory = FPaths::Combine( UserDirectory, TEXT( "Traces" ) );

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if ( !PlatformFile.DirectoryExists( *UserDirectory ) ) {

		if ( PlatformFile.CreateDirectory( *UserDirectory ) &&
			 PlatformFile.CreateDirectory( *ImagesDirectory ) &&
			 PlatformFile.CreateDirectory( *TracesDirectory ) &&
			 PlatformFile.CreateDirectory( *FacesDirectory ) ) {


			FString RecognitionDataFile = FPaths::Combine( TracesDirectory, TEXT( "RecognitionData.csv" ) );
			IFileHandle * UserDataFile = PlatformFile.OpenWrite( *RecognitionDataFile );
			delete(UserDataFile);


			FString DataHeader = TEXT( "Id,Time,FaceConfidence,GazeConfidence,FaceThreshold,GazeThreshold,ImgPeriod\r\n" );
			FFileHelper::SaveStringToFile( DataHeader, *RecognitionDataFile );

			return 0;
		}
	}

	return -1;
}

int UGlanceUser::SaveUserData() {
	TSharedPtr<FJsonObject> JsonObject = MakeShareable( new FJsonObject );

	JsonObject->SetStringField( "Username", Username );
	JsonObject->SetStringField( "Id", Id.ToString() );
	JsonObject->SetNumberField( "ImgShowPeriod", ImgShowPeriod );
	TArray < TSharedPtr <FJsonValue >> ImageArray;
	TArray < TSharedPtr <FJsonValue >> PositionArray;

	for ( auto img : ImagePaths ) {
		ImageArray.Add( MakeShareable( new FJsonValueString( img ) ) );
	}

	for ( auto position : UserImagesPositions ) {
		PositionArray.Add( MakeShareable( new FJsonValueBoolean( position ) ) );
	}

	JsonObject->SetArrayField( "ImagePaths", ImageArray );
	JsonObject->SetArrayField( "UserImagesPositions", PositionArray );

	FString OutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create( &OutputString );
	FJsonSerializer::Serialize( JsonObject.ToSharedRef(), Writer );

	FFileHelper::SaveStringToFile( OutputString, *FPaths::Combine( UserDirectory, TEXT( "User.json" ) ) );

	return 0;
}

UGlanceUser* UGlanceUser::LoadUserData( FGuid UserId ) {
	UGlanceUser *User = NewObject<UGlanceUser>();
	User->Id = UserId;
	User->Label = UGlanceUser::CreateLabel( UserId );

	User->UserDirectory = FPaths::Combine( UsersDirectory, User->Id.ToString() );
	User->ImagesDirectory = FPaths::Combine( User->UserDirectory, TEXT( "Images" ) );
	User->FacesDirectory = FPaths::Combine( User->UserDirectory, TEXT( "Faces" ) );
	User->TracesDirectory = FPaths::Combine( User->UserDirectory, TEXT( "Traces" ) );

	FString UserDataString;
	FFileHelper::LoadFileToString( UserDataString, *FPaths::Combine( User->UserDirectory, TEXT( "User.json" ) ) );
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create( UserDataString );

	if ( FJsonSerializer::Deserialize( Reader, JsonObject ) ) {
		User->Username = JsonObject->GetStringField( TEXT( "Username" ) );

		for ( auto ImgPath : JsonObject->GetArrayField( TEXT( "ImagePaths" ) ) ) {
			User->ImagePaths.Add( ImgPath->AsString() );
		}

		for ( auto ImgPosition : JsonObject->GetArrayField( TEXT( "UserImagesPositions" ) ) ) {
			User->UserImagesPositions.Add( ImgPosition->AsBool() );
		}

		User->ImgShowPeriod = JsonObject->GetNumberField( TEXT( "ImgShowPeriod" ) );
	}
	 
	return User;
}

FString UGlanceUser::GetUsername( FGuid UserId ) {
	UGlanceUser *User = NewObject<UGlanceUser>();
	User->Id = UserId;

	User->UserDirectory = FPaths::Combine( UsersDirectory, User->Id.ToString() );

	FString UserDataString;
	FFileHelper::LoadFileToString( UserDataString, *FPaths::Combine( User->UserDirectory, TEXT( "User.json" ) ) );

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create( UserDataString );

	FString Username;

	if ( FJsonSerializer::Deserialize( Reader, JsonObject ) ) {
		 Username = JsonObject->GetStringField( TEXT( "Username" ) );
	}

	return Username;
}


int UGlanceUser::CreateLabel( FGuid UserId ) {
	return UserId.A ^ UserId.B ^ UserId.C ^ UserId.D;
}

UGlanceUser::~UGlanceUser()
{
}
