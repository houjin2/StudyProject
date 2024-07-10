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

	//HPBar UI ����ȭ
	UFUNCTION(NetMulticast, Reliable)
	void OnCurrentHPChanged_NetMulticast(float InOldCurrentHP, float InNewCurrentHP);

public:	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "USStatComponent", meta = (AllowprivateAccess))
	TObjectPtr<class USGameInstance> GameInstance;

	UPROPERTY(Replicated,VisibleInstanceOnly, BlueprintReadOnly, Category = "USStatComponent", meta = (AllowprivateAccess))
	float MaxHP;

	//Transient : csv���Ͽ� ����ǰų� �о���� ���� InitializeComponent() ���� Max������ �ʱ�ȭ��.(�ø������� �ʿ���°͵�.) ��κ��� Current�� Transient.
	UPROPERTY(Replicated, Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "USStatComponent", meta = (AllowprivateAccess))
	float CurrentHP;

		
};
