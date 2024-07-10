// SStatComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOutOfCurrentHPDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentHPChangeDelegate, float, InOldCurrentHP, float, InNewCurrentHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxHPChangeDelegate, float, InOldMaxHP, float, InNewMaxHP);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STUDYPROJECT_API USStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USStatComponent();

	virtual void BeginPlay() override;

	float GetMaxHP() const { return MaxHP; }

	void SetMaxHP(float InMaxHP);

	float  GetCurrentHP() const { return CurrentHP; }

	void SetCurrentHP(float InCurrentHP);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;


public:
	FOnOutOfCurrentHPDelegate OnOutOfCurrentHPDelegate;

	FOnCurrentHPChangeDelegate OnCurrentHPChangedDelegate;

	FOnMaxHPChangeDelegate OnMaxHPChangeDelegate;

	//HPBar UI 동기화
	UFUNCTION(NetMulticast, Reliable)
	void OnCurrentHPChanged_NetMulticast(float InOldCurrentHP, float InNewCurrentHP);

public:	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "USStatComponent", meta = (AllowprivateAccess))
	TObjectPtr<class USGameInstance> GameInstance;

	UPROPERTY(Replicated,VisibleInstanceOnly, BlueprintReadOnly, Category = "USStatComponent", meta = (AllowprivateAccess))
	float MaxHP;

	//Transient : csv파일에 저장되거나 읽어오지 않음 InitializeComponent() 에서 Max값으로 초기화됨.(시리얼라이즈가 필요없는것들.) 대부분의 Current는 Transient.
	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "USStatComponent", meta = (AllowprivateAccess))
	float CurrentHP;

		
};
