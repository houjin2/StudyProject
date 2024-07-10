// SLandMine.cpp


#include "WorldStatic/SLandMine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"


ASLandMine::ASLandMine()
    : bIsExploded(false)
{
	PrimaryActorTick.bCanEverTick = true;

	BodyBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BodyBoxComponent"));
	SetRootComponent(BodyBoxComponent);

	BodyStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyStaticMeshComponent"));
	BodyStaticMeshComponent->SetupAttachment(GetRootComponent());

    ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
    ParticleSystemComponent->SetupAttachment(GetRootComponent());
    ParticleSystemComponent->SetAutoActivate(false);

	bReplicates = true;
}

void ASLandMine::OnLandMineBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (true == HasAuthority())
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::OnLandMineBeginOverlap(%d) has been called in Server PC."), bIsExploded));
	}
	else
	{
		if (GetOwner() == UGameplayStatics::GetPlayerController(this, 0))
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::OnLandMineBeginOverlap(%d) has been called in OwningClient PC."), bIsExploded));
		}
		else
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ASLandMine::OnLandMineBeginOverlap(%d) has been called in OtherClient PC."), bIsExploded));
		}
	}

    if (true == HasAuthority() && bIsExploded == false)
    {
        SpawnEffect_NetMulticast();
        bIsExploded = true;
    }
}

void ASLandMine::SpawnEffect_NetMulticast_Implementation()
{
    if (HasAuthority() == false)
    {
        ParticleSystemComponent->Activate(true);

		if (IsValid(ExplodedMaterial) == true)
		{
			BodyStaticMeshComponent->SetMaterial(0, ExplodedMaterial);
		}
    }
}

void ASLandMine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsExploded);
}

void ASLandMine::OnRep_IsExploded()
{
	if (IsValid(ExplodedMaterial) == true && HasAuthority() == false)
	{
		BodyStaticMeshComponent->SetMaterial(0, ExplodedMaterial);
	}
}

void ASLandMine::BeginPlay()
{
	Super::BeginPlay();

    if (OnActorBeginOverlap.IsAlreadyBound(this, &ThisClass::OnLandMineBeginOverlap) == false)
    {
        OnActorBeginOverlap.AddDynamic(this, &ThisClass::OnLandMineBeginOverlap);
    }
}

void ASLandMine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (OnActorBeginOverlap.IsAlreadyBound(this, &ThisClass::OnLandMineBeginOverlap) == true)
	{
		OnActorBeginOverlap.RemoveDynamic(this, &ThisClass::OnLandMineBeginOverlap);
	}
}


void ASLandMine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

