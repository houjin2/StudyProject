// UW_InGameMenu.cpp


#include "UI/UW_InGameMenu.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Controller/SPlayerController.h"

void UUW_InGameMenu::NativeConstruct()
{
	ResumeButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnResumeButtonClicked);
	ReturnTitleButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnReturnTitleButtonClicked);
	ExitButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
}

void UUW_InGameMenu::OnResumeButtonClicked()
{
	ASPlayerController* PlayerController = Cast<ASPlayerController>(GetOwningPlayer());
	if (IsValid(PlayerController) == true)
	{
		PlayerController->ToggleInGameMenu();
	}
}

void UUW_InGameMenu::OnReturnTitleButtonClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Loading")), true, FString(TEXT("NextLevel=Title")));
}

void UUW_InGameMenu::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
