// SPlayerPawn.cpp


#include "Character/SPlayerPawn.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
ASPlayerPawn::ASPlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	float CharacterHalfHeigt = 90.f;
	float CharacterRadius = 20.f;

#pragma region InitializeCapsuleComponent
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	SetRootComponent(CapsuleComponent);
	CapsuleComponent->SetCapsuleHalfHeight(CharacterHalfHeigt);
	CapsuleComponent->SetCapsuleRadius(CharacterRadius);
#pragma endregion

#pragma region InitializeSkeletalMesh
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	FVector PivotPosition(0.f, 0.f, -CharacterHalfHeigt);
	FRotator PivotRotation(0.f, -90.f, 0.f);
	SkeletalMeshComponent->SetRelativeLocationAndRotation(PivotPosition, PivotRotation);
	
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstanceClassInfo(TEXT("'/Game/StudyProject/Animation/AnimationBlueprint/ABP_PlayerPawn.ABP_PlayerPawn_C'"));

	if (AnimInstanceClassInfo.Succeeded() == true)
	{
		SkeletalMeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		SkeletalMeshComponent->SetAnimClass(AnimInstanceClassInfo.Class);
	}


#pragma endregion

#pragma region InitializeCamera
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 400.f;
	SpringArmComponent->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

#pragma endregion

	FloatingPawnMovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
}

void ASPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("UpDown"), this, &ThisClass::UpDown);
	PlayerInputComponent->BindAxis(TEXT("LeftRight"), this, &ThisClass::LeftRight);
}

void ASPlayerPawn::BeginPlay()
{
	Super::BeginPlay();

}

void ASPlayerPawn::UpDown(float InAxisValue)
{
	AddMovementInput(GetActorForwardVector(), InAxisValue);
}

void ASPlayerPawn::LeftRight(float InAxisValue)
{
	AddMovementInput(GetActorRightVector(), InAxisValue);

}


