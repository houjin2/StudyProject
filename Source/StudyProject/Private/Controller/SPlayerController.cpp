// SPlayerController.cpp


#include "Controller/SPlayerController.h"
#include "UI/SHUD.h"
#include "Game/SPlayerState.h"
#include "Component/SStatComponent.h"
#include "Character/SCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"		//DeadPlayer count
#include "Game/SGameMode.h"				//DeadPlayer count
#include "UI/SGameResultWidget.h"		//Player Rank 
#include "Components/TextBlock.h"		//Player Rank 
#include "Character/SPlayerCharacter.h"

ASPlayerController::ASPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;	//true로 해야 입력에 문제가 없음.

}

void ASPlayerController::ToggleInGameMenu()
{
	checkf(IsValid(InGameMenuInstance) == true, TEXT("InValid InGameMenuInstance"));

	if (bIsInGameMenuOn == false)
	{
		InGameMenuInstance->SetVisibility(ESlateVisibility::Visible);

		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(InGameMenuInstance->GetCachedWidget());
		SetInputMode(Mode);


		// SetPause(true); 만약 게임 일시 정지를 원한다면.
		// InputAction 애셋의 TriggerWhenPaused 속성을 true로 지정해야 Pause 상태에서도 해당 입력 액션이 동작함.
		bShowMouseCursor = true;
	}
	else
	{
		InGameMenuInstance->SetVisibility(ESlateVisibility::Collapsed);

		FInputModeGameOnly InputGameModeOnly;
		SetInputMode(InputGameModeOnly);

		// SetPause(false); 만약 게임 일시 정지를 원한다면 이 코드가 필요함.
		// InputAction 애셋의 TriggerWhenPaused 속성을 true로 지정해야 Pause 상태에서도 해당 입력 액션이 동작함.

		bShowMouseCursor = false;
	}

	bIsInGameMenuOn = !bIsInGameMenuOn;
}

void ASPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}

void ASPlayerController::OnOwningCharacterDead()
{
	ASGameMode* GameMode = Cast<ASGameMode>(UGameplayStatics::GetGameMode(this));
	if (HasAuthority() == true && IsValid(GameMode) == true)
	{
		GameMode->OnControllerDead(this);
	}
}

void ASPlayerController::ShowWinnerUI_Implementation()
{
	if (HasAuthority() == false)
	{
		if (IsValid(WinnerUIClass) == true)
		{
			USGameResultWidget* WinnerUI = CreateWidget<USGameResultWidget>(this, WinnerUIClass);
			if (IsValid(WinnerUI) == true)
			{
				WinnerUI->AddToViewport(3);
				WinnerUI->RankingText->SetText(FText::FromString(TEXT("#01")));

				FInputModeUIOnly Mode;
				Mode.SetWidgetToFocus(WinnerUI->GetCachedWidget());
				SetInputMode(Mode);

				bShowMouseCursor = true;
			}
		}
	}
}

void ASPlayerController::ShowLoserUI_Implementation(int32 InRanking)
{
	if (HasAuthority() == false)
	{
		if (IsValid(LoserUIClass) == true)
		{
			USGameResultWidget* LoserUI = CreateWidget<USGameResultWidget>(this, LoserUIClass);
			if (IsValid(LoserUI) == true)
			{
				LoserUI->AddToViewport(3);
				FString RankingString = FString::Printf(TEXT("#%02d"), InRanking);
				LoserUI->RankingText->SetText(FText::FromString(RankingString));

				FInputModeUIOnly Mode;
				Mode.SetWidgetToFocus(LoserUI->GetCachedWidget());
				SetInputMode(Mode);

				bShowMouseCursor = true;
			}
		}
	}
}

void ASPlayerController::ReturnToLobby_Implementation()
{
	if (HasAuthority() == false)
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Loading")), true, FString(TEXT("NextLevel=Lobby?Saved=false")));
	}
}

void ASPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly InputModeGameOnly;	//시작하자마자 움직일수 있도록 함
	SetInputMode(InputModeGameOnly);

	if (HasAuthority() == true)
	{
		return;
	}
	
	if (IsValid(HUDWidgetClass) == true)
	{
		HUDWidget = CreateWidget<USHUD>(this, HUDWidgetClass);
		if (IsValid(HUDWidget) == true) 
		{
			HUDWidget->AddToViewport();

			ASPlayerState* SPlayerState = GetPlayerState<ASPlayerState>();
			if (IsValid(SPlayerState) == true)
			{
				HUDWidget->BindPlayerState(SPlayerState);
			}

			ASCharacter* PlayerCharacter = GetPawn<ASCharacter>();
			if (IsValid(PlayerCharacter) == true)
			{
				USStatComponent* StatComponent = PlayerCharacter->GetStatComponent();
				if (IsValid(StatComponent) == true)
				{
					HUDWidget->BindStatComponent(StatComponent);
				}
			}

			ASPlayerCharacter* SPlayerCharacter = GetPawn<ASPlayerCharacter>();
			if (IsValid(SPlayerCharacter) == true)
			{
				HUDWidget->BindPlayerCharacter(SPlayerCharacter);
			}
		}
	}

	if (IsValid(InGameMenuClass) == true)
	{
		InGameMenuInstance = CreateWidget<UUserWidget>(this, InGameMenuClass);
		if (IsValid(InGameMenuInstance) == true)
		{
			InGameMenuInstance->AddToViewport(3);	//상위에 띄우기 위함

			InGameMenuInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (IsValid(CrosshairUiClass) == true)
	{
		UUserWidget* CrosshairUIInstance = CreateWidget<UUserWidget>(this, CrosshairUiClass);
		if (IsValid(CrosshairUIInstance) == true)
		{
			CrosshairUIInstance->AddToViewport(1);

			CrosshairUIInstance->SetVisibility(ESlateVisibility::Visible);
		}
	}

	if (IsValid(NotificationTextClass) == true)
	{
		UUserWidget* NotificationTextUI = CreateWidget<UUserWidget>(this, NotificationTextClass);
		if (IsValid(NotificationTextUI) == true)
		{
			NotificationTextUI->AddToViewport(1);

			NotificationTextUI->SetVisibility(ESlateVisibility::Visible);
		}
	}
}
