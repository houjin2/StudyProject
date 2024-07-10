// SItemBox.cpp


#include "WorldStatic/SItemBox.h"

// Sets default values
ASItemBox::ASItemBox()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);
	BoxComponent->SetBoxExtent(FVector(40.f, 42.f, 30.f));
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ASItemBox::OnOverlapBegin);
	BoxComponent->SetCollisionProfileName(FName(TEXT("SItemBox")));	//콜리젼프로파일의 이름

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
	StaticMeshComponent->SetRelativeLocation(FVector(0.0f, -3.5f, -30.0f));
	StaticMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));

	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->SetupAttachment(GetRootComponent());
	ParticleSystemComponent->SetAutoActivate(false);
}

void ASItemBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	ParticleSystemComponent->Activate(true);
	StaticMeshComponent->SetHiddenInGame(true);
	SetActorEnableCollision(false);
	ParticleSystemComponent->OnSystemFinished.AddDynamic(this, &ASItemBox::OnEffectFinish);
}

void ASItemBox::OnEffectFinish(UParticleSystemComponent* ParticleSystem)
{
	Destroy();
}

