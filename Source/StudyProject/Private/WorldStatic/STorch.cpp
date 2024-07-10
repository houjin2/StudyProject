//STorch.cpp


#include "WorldStatic/STorch.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/RotatingMovementComponent.h"

ASTorch::ASTorch()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);

	BodyStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyStaticMeshComponent"));
	BodyStaticMeshComponent->SetupAttachment(GetRootComponent());
	BodyStaticMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -30.f));

	PointLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLightComponent"));
	PointLightComponent->SetupAttachment(GetRootComponent());
	PointLightComponent->SetRelativeLocation(FVector(0.f, 0.f, 500.f));


	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->SetupAttachment(GetRootComponent());
	ParticleSystemComponent->SetRelativeLocation(FVector(0.f, 0.f, 500.f));

	RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovementComponent"));
	



}

void ASTorch::BeginPlay()
{
	Super::BeginPlay();

	RotationSpeed = 300.f;
	RotatingMovementComponent->RotationRate = FRotator(0.f, RotationSpeed, 0.f);
}

void ASTorch::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);


}


