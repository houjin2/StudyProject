// SStatComponent.cpp


#include "Component/SStatComponent.h"
#include "Game/SGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

USStatComponent::USStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	//PostInitializeComponents() �Լ��� �����ϴ� ������Ʈ �Լ�.  PostInitializeComponents() �Լ��� ȣ��Ǳ� �ٷ� ���� ȣ���. ������Ʈ�� �ʱ�ȭ ������ ����. ȣ��Ƿ��� bWantsInitializeComponent �Ӽ��� true
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

void USStatComponent::SetCurrentHP(float InCurrentHP)	//SetCurrentHP�� �������� ȣ��Ǵ� ���̱� ������ NetMulticast�� ����
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


