// SUIPlayerController_Loading.h


#include "Controller/SUIPlayerController_Loading.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

void ASUIPlayerController_Loading::BeginPlay()
{
	Super::BeginPlay();

	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	if (IsValid(GameMode) == true)
	{
		FString NextLevelString = UGameplayStatics::ParseOption(GameMode->OptionsString, FString(TEXT("NextLevel")));
		UGameplayStatics::OpenLevel(GameMode, *NextLevelString, false);
	}
}