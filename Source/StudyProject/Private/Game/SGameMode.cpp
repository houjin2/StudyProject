// SGameMode.cpp


#include "Game/SGameMode.h"
#include "Controller/SPlayerController.h"
#include "Character/SPlayerPawn.h"
#include "Game/SPlayerState.h"
#include "Game/SGameState.h"
#include "Kismet/GameplayStatics.h"


ASGameMode::ASGameMode()
{
	PlayerControllerClass = ASPlayerController::StaticClass();	//Ŭ���������� StaticClass() �Լ��� ���� ������ �ܰ迡���� Ŭ���� ������ ������ �� ����. ��Ÿ�� ���� Ŭ���� ������ GetClass() �Լ��� ���� ����.
	DefaultPawnClass = ASPlayerPawn::StaticClass();
}

void ASGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ASGameState* SGameState = GetGameState<ASGameState>();
	if (SGameState == false)
	{
		return;
	}

	if (SGameState->MatchState != EMatchState::Waiting)
	{
		NewPlayer->SetLifeSpan(0.1f);
	}

	ASPlayerState* PlayerState = NewPlayer->GetPlayerState<ASPlayerState>();
	if (IsValid(PlayerState) == true)
	{
		PlayerState->InitPlayerState();
	}

	ASPlayerController* NewPlayerController = Cast<ASPlayerController>(NewPlayer);
	if (IsValid(NewPlayerController) == true)
	{
		AlivePlayerControllers.Add(NewPlayerController);
		NewPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));
	}

}

void ASGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ASPlayerController* ExitingPlayerController = Cast<ASPlayerController>(Exiting);
	if (IsValid(ExitingPlayerController) && INDEX_NONE != AlivePlayerControllers.Find(ExitingPlayerController))
	{
		AlivePlayerControllers.Remove(ExitingPlayerController);
		DeadPlayerControllers.Add(ExitingPlayerController);
	}
}

void ASGameMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(MainTimerHandle, this, &ThisClass::OnMainTimerElapsed, 1.f, true);	//Ÿ�̸� ����
	RemainWaitingTimeForPlaying = WaitingTime;

}

void ASGameMode::OnControllerDead(ASPlayerController* InDeadController)
{
	if (IsValid(InDeadController) == false || AlivePlayerControllers.Find(InDeadController) == INDEX_NONE)
	{
		return;
	}

	InDeadController->ShowLoserUI(AlivePlayerControllers.Num());

	AlivePlayerControllers.Remove(InDeadController);
	DeadPlayerControllers.Add(InDeadController);
}

void ASGameMode::OnMainTimerElapsed()
{
	ASGameState* SGameState = GetGameState<ASGameState>();
	if (IsValid(SGameState) == false)
	{
		return;
	}


	switch (SGameState->MatchState)
	{
	case EMatchState::None:
		break;
	case EMatchState::Waiting:
	{
		FString NotificationString = FString::Printf(TEXT(""));

		if (AlivePlayerControllers.Num() < MinimumPlayerCountForPlaying)
		{
			NotificationString = FString::Printf(TEXT("wait another players for playing."));

			RemainWaitingTimeForPlaying = WaitingTime;	//�ּ��ο��� �ȵȴٸ� ���ð� �ʱ�ȭ
		}
		else
		{
			NotificationString = FString::Printf(TEXT("Wait %d seconds for playing"), RemainWaitingTimeForPlaying);

			--RemainWaitingTimeForPlaying;
		}

		if (RemainWaitingTimeForPlaying == 0)
		{
			NotificationString = FString::Printf(TEXT(""));

			SGameState->MatchState = EMatchState::Playing;
		}

		NotifyToAllPlayer(NotificationString);

		break;
	}
	case EMatchState::Playing:
	{
		if (IsValid(SGameState) == true)
		{
			SGameState->AlivePlayerControllerCount = AlivePlayerControllers.Num();

			FString NotificationString = FString::Printf(TEXT("%d / %d"), SGameState->AlivePlayerControllerCount, SGameState->AlivePlayerControllerCount - DeadPlayerControllers.Num());

			NotifyToAllPlayer(NotificationString);

			if (SGameState->AlivePlayerControllerCount <= 1)
			{
				SGameState->MatchState = EMatchState::Ending;
				AlivePlayerControllers[0]->ShowWinnerUI();
			}
		}

		break;
	}
	case EMatchState::Ending:
	{
		FString NotificationStrong = FString::Printf(TEXT("Waiting %d for return to lobby."), RemainWaitingTimeForEnding);

		NotifyToAllPlayer(NotificationStrong);

		--RemainWaitingTimeForEnding;

		if (RemainWaitingTimeForEnding <= 0)
		{
			for (auto AlivePlayerController : AlivePlayerControllers)
			{
				AlivePlayerController->ReturnToLobby();
			}
			for (auto DeadPlayerController : DeadPlayerControllers)
			{
				DeadPlayerController->ReturnToLobby();
			}

			MainTimerHandle.Invalidate();

			FName CurrentLevelName = FName(UGameplayStatics::GetCurrentLevelName(this));
			UGameplayStatics::OpenLevel(this, CurrentLevelName, true, FString(TEXT("listen")));
			// ���� ���� ������ ���� ���� ���񽺵�� �����Ǿ� �ִٸ�,
			// �̷��� ������ �ٽ� �غ�� �� ���� �������� �˷���. "���ο� �÷��̾� ���� �� ����."
			// �׷� ���� ���񽺴� ���ο� �÷��̾�鿡�� ���� ������ IP �ּҸ� �������༭ ���� ����.
			return;
		}

		break;
	}
	case EMatchState::End:
		break;

	default:
		break;
	}
}

void ASGameMode::NotifyToAllPlayer(const FString& NotificationString)
{
	for (auto AlivePlayerController : AlivePlayerControllers)
	{
		AlivePlayerController->NotificationText = FText::FromString(NotificationString);
	}

	for (auto DeadPlayerController : DeadPlayerControllers)
	{
		DeadPlayerController->NotificationText = FText::FromString(NotificationString);
	}
}
