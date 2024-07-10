// SGameMode.cpp


#include "Game/SGameMode.h"
#include "Controller/SPlayerController.h"
#include "Character/SPlayerPawn.h"
#include "Game/SPlayerState.h"
#include "Game/SGameState.h"
#include "Kismet/GameplayStatics.h"


ASGameMode::ASGameMode()
{
	PlayerControllerClass = ASPlayerController::StaticClass();	//클래스정보는 StaticClass() 함수에 의해 컴파일 단계에서의 클래스 정보를 가져올 수 있음. 런타임 중의 클래스 정보는 GetClass() 함수를 통해 가능.
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

	GetWorld()->GetTimerManager().SetTimer(MainTimerHandle, this, &ThisClass::OnMainTimerElapsed, 1.f, true);	//타이머 구현
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

			RemainWaitingTimeForPlaying = WaitingTime;	//최소인원이 안된다면 대기시간 초기화
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
			// 만약 데디 서버가 게임 세션 서비스들과 연동되어 있다면,
			// 이렇게 레벨을 다시 준비된 뒤 세션 서버한테 알려줌. "새로운 플레이어 들어올 수 있음."
			// 그럼 세션 서비스는 새로운 플레이어들에게 데디 서버의 IP 주소를 전달해줘서 접속 유도.
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
