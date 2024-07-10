// SGameState.cpp


#include "Game/SGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, AlivePlayerControllerCount);
	DOREPLIFETIME(ThisClass, MatchState);
}
