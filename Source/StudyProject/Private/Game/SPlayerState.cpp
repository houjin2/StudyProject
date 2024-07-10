// SPlayerState.cpp


#include "Game/SPlayerState.h"
#include "Character/SPlayerCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ASPlayerState::ASPlayerState()
{
}

void ASPlayerState::InitPlayerState()
{
	CurrentKillCount = 0;
	MaxKillCount = 99;

	// 만약 경험치 개념을 구현한다면 이 함수에서 GameInstance를 통한 초기화 필요.

	const FString SavedDirectoryPath = FPaths::Combine(FPlatformMisc::ProjectDir(), TEXT("Saved"));
	const FString SavedFileName(TEXT("PlayerInfo.txt"));
	FString AbsoluteFilePath = FPaths::Combine(*SavedDirectoryPath, *SavedFileName);
	FPaths::MakeStandardFilename(AbsoluteFilePath);

	FString PlayerInfoJsonString;
	FFileHelper::LoadFileToString(PlayerInfoJsonString, *AbsoluteFilePath);
	TSharedRef<TJsonReader<TCHAR>> JsonReaderArchive = TJsonReaderFactory<TCHAR>::Create(PlayerInfoJsonString);

	TSharedPtr<FJsonObject> PlayerInfoJsonObject = nullptr;
	if (FJsonSerializer::Deserialize(JsonReaderArchive, PlayerInfoJsonObject) == true)
	{
		FString PlayerNameString = PlayerInfoJsonObject->GetStringField(TEXT("playername"));
		SetPlayerName(PlayerNameString);

		uint8 PlayerTeamNumber = PlayerInfoJsonObject->GetIntegerField(TEXT("team"));
		PlayerTeam = static_cast<EPlayerTeam>(PlayerTeamNumber);
		ASPlayerCharacter* PlayerCharacter = Cast<ASPlayerCharacter>(GetPawn());
		if (IsValid(PlayerCharacter) == true)
		{
			PlayerCharacter->SetMeshMaterial(PlayerTeam);
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("PlayerNameString : %s, PlayerTeam : %d"), *PlayerNameString, PlayerTeamNumber));
		}
	}
}

void ASPlayerState::AddCurrentKillCount(int32 InCurrentKillCount)
{
	OnCurrentKillCountChangedDelegate.Broadcast(CurrentKillCount, CurrentKillCount + InCurrentKillCount);

	CurrentKillCount = FMath::Clamp(CurrentKillCount + InCurrentKillCount, 0, MaxKillCount);

	ASPlayerCharacter* PlayerCharacter = Cast<ASPlayerCharacter>(GetPawn());
	checkf(IsValid(PlayerCharacter) == true, TEXT("InValid PlayerCharacter"));
	PlayerCharacter->GetParticleSystem()->Activate(true);
}
