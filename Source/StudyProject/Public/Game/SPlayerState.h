// SPlayerState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentKillCountChangedDelegate, int32, InOldCurrentKillCount, int32, InNewCurrentKillCount);

UENUM(BlueprintType)
enum class EPlayerTeam : uint8
{
	None,
	Black,
	White,
	End
};

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ASPlayerState();
	
	void InitPlayerState();

	int32 GetMaxKillCount() const { return MaxKillCount; }

	void SetMaxKillCount(int32 InMaxKillCount) { MaxKillCount = InMaxKillCount; }

	int32 GetCurrentKillCount() const { return CurrentKillCount; }

	void AddCurrentKillCount(int32 InCurrentKillCount);

	EPlayerTeam GetPlayerTeam() const { return PlayerTeam; }

public:
	FOnCurrentKillCountChangedDelegate OnCurrentKillCountChangedDelegate;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	int32 MaxKillCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	int32 CurrentKillCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	EPlayerTeam PlayerTeam = EPlayerTeam::None;
};
