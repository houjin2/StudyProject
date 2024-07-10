// SStatComponent.cpp


#include "Component/SStatComponent.h"
#include "Game/SGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

USStatComponent::USStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	//PostInitializeComponents() 함수에 대응하는 컴포넌트 함수.  PostInitializeComponents() 함수가 호출되기 바로 전에 호출됨. 컴포넌트의 초기화 로직을 구현. 호출되려면 bWantsInitializeComponent 속성이 true
	bWantsInitializeComponent = false;
}


// Called when the game starts
void USStatComponent::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<USGameInstance>(GetWorld()->GetGameInstance());
	if (IsValid(GameInstance) == true)
	{
		if (nullptr != GameInstance->GetCharacterStatDataTable() || nullptr != GameInstance->GetCharacterStatDataTableRow(1))
		{
			float NewMaxHP = GameInstance->GetCharacterStatDataTableRow(1)->MaxHP;
			SetMaxHP(NewMaxHP);
			SetCurrentHP(MaxHP);
		}
	}
}

void USStatComponent::SetMaxHP(float InMaxHP)
{
	if (OnMaxHPChangeDelegate.IsBound() == true)
	{
		OnMaxHPChangeDelegate.Broadcast(MaxHP, InMaxHP);
	}

	MaxHP = FMath::Clamp<float>(InMaxHP, 0.f, 9999);
}

void USStatComponent::SetCurrentHP(float InCurrentHP)	//SetCurrentHP는 서버에서 호출되는 것이기 때문에 NetMulticast도 가능
{
	if (OnCurrentHPChangedDelegate.IsBound() == true)
	{
		OnCurrentHPChangedDelegate.Broadcast(CurrentHP, InCurrentHP);
	}

	CurrentHP = FMath::Clamp<float>(InCurrentHP, 0.f, MaxHP);

	if (CurrentHP < KINDA_SMALL_NUMBER)
	{
		OnOutOfCurrentHPDelegate.Broadcast();
		CurrentHP = 0.f;
	}
	
	OnCurrentHPChanged_NetMulticast(CurrentHP, CurrentHP);
}

void USStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MaxHP);
	DOREPLIFETIME(ThisClass, CurrentHP);
}

void USStatComponent::OnCurrentHPChanged_NetMulticast_Implementation(float InOldCurrentHP, float InNewCurrentHP)
{
	if (OnCurrentHPChangedDelegate.IsBound() == true)
	{
		OnCurrentHPChangedDelegate.Broadcast(InOldCurrentHP, InNewCurrentHP);
	}

	if (InNewCurrentHP < KINDA_SMALL_NUMBER)
	{
		OnOutOfCurrentHPDelegate.Broadcast();
	}
}


