// SCharacter.cpp


#include "Character/SCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "Engine/DamageEvents.h"
#include "Animation/SAnimInstance.h"
#include "Item/SWeaponActor.h"
#include "Component/SStatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Controller/SPlayerController.h"	//DeadPlayer count
#include "Kismet/GameplayStatics.h"
#include "Game/SGameState.h"
#include "Character/SPlayerCharacter.h"

int32 ASCharacter::ShowAttackDebug = 1;

FAutoConsoleVariableRef CVarShowAttackDebug(
	TEXT("StudyProject.ShowAttackDebug"),
	ASCharacter::ShowAttackDebug,
	TEXT(""),
	ECVF_Cheat
);

ASCharacter::ASCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	float CharacterHalfHeight = 95.f;
	float CharacterRadius = 20.f;

	GetCapsuleComponent()->InitCapsuleSize(CharacterRadius, CharacterHalfHeight);

	FVector PivotPosition(0.f, 0.f, -CharacterHalfHeight);
	FRotator PivotRotation(0.f, -90.f, 0.f);
	GetMesh()->SetRelativeLocationAndRotation(PivotPosition, PivotRotation);

	GetCharacterMovement()->MaxWalkSpeed = 350.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	StatComponent = CreateDefaultSubobject<USStatComponent>(TEXT("StatComponent"));
	StatComponent->SetIsReplicated(true);
}

void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
	if (true == ::IsValid(AnimInstance))
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::OnMeleeAttackMontageEnded);
		AnimInstance->OnCheckHit.AddDynamic(this, &ThisClass::OnCheckHit);
		AnimInstance->OnCheckAttackInput.AddDynamic(this, &ThisClass::OnCheckAttackInput);
	}

	if (IsValid(StatComponent) == true && StatComponent->OnOutOfCurrentHPDelegate.IsAlreadyBound(this, &ThisClass::OnCharacterDeath) == false)
	{
		StatComponent->OnOutOfCurrentHPDelegate.AddDynamic(this, &ThisClass::OnCharacterDeath);
	}
}

float ASCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	float FinalDamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	ASGameState* SGameState = Cast<ASGameState>(UGameplayStatics::GetGameState(this));
	if (IsValid(SGameState) == true && SGameState->MatchState != EMatchState::Playing)	//Plating 상태가 아니라면 밑 대미지 로직 수행 X
	{
		return FinalDamageAmount;
	}

	StatComponent->SetCurrentHP(StatComponent->GetCurrentHP() - FinalDamageAmount);

	if (ShowAttackDebug == 1)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s [%.1f / %.1f]"), *GetName(), StatComponent->GetCurrentHP(), StatComponent->GetMaxHP()));
	}

	return FinalDamageAmount;
}



void ASCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (StatComponent->OnOutOfCurrentHPDelegate.IsAlreadyBound(this, &ThisClass::OnCharacterDeath))
	{
		StatComponent->OnOutOfCurrentHPDelegate.RemoveDynamic(this, &ThisClass::OnCharacterDeath);
	}

	Super::EndPlay(EndPlayReason);
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, WeaponInstance);
}



void ASCharacter::OnMeleeAttackMontageEnded(UAnimMontage* InMontage, bool bInterruped)
{
	if (InMontage->GetName().Equals(TEXT("AM_Rifle_Fire_Melee"), ESearchCase::IgnoreCase) == true)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		bIsNowAttacking = false;
	}

	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
	if (true == AnimInstance->OnMontageEnded.IsAlreadyBound(this, &ThisClass::OnMeleeAttackMontageEnded))
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &ThisClass::OnMeleeAttackMontageEnded);
	}
}

void ASCharacter::OnCheckHit()
{
	FHitResult HitResult;
	FCollisionQueryParams Params(NAME_None, false, this);

	bool bResult = GetWorld()->SweepSingleByChannel(
		HitResult,
		GetActorLocation(),
		GetActorLocation() + MeleeAttackRange * GetActorForwardVector(),
		FQuat::Identity,
		ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(MeleeAttackRadius),
		Params
	);

	if (bResult == true)
	{
		if (IsValid(HitResult.GetActor()) == true)
		{
			FDamageEvent DamageEvent;
			HitResult.GetActor()->TakeDamage(50.f, DamageEvent, GetController(), this);
		}
	}

#pragma region CollisionDebugDrawing
	if (ShowAttackDebug == 1)
	{
		FVector TraceVector = MeleeAttackRange * GetActorForwardVector();
		FVector Center = GetActorLocation() + TraceVector + GetActorUpVector() * 40.f;
		float HalfHeight = MeleeAttackRange * 0.5f + MeleeAttackRadius;
		FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVector).ToQuat();
		FColor DrawColor = true == bResult ? FColor::Green : FColor::Red;
		float DebugLifeTime = 5.f;

		DrawDebugCapsule(
			GetWorld(),
			Center,
			HalfHeight,
			MeleeAttackRadius,
			CapsuleRot,
			DrawColor,
			false,
			DebugLifeTime
		);

		if (true == bResult)
		{
			if (true == ::IsValid(HitResult.GetActor()))
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Actor Name: %s"), *HitResult.GetActor()->GetName()));
			}
		}
	}
#pragma endregion
	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());

	if(true == AnimInstance->OnCheckHit.IsAlreadyBound(this, &ThisClass::OnCheckHit))
	{
		AnimInstance->OnCheckHit.RemoveDynamic(this, &ThisClass::OnCheckHit);
	}

}



void ASCharacter::BeginAttack()
{
	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
	checkf(IsValid(AnimInstance) == true, TEXT("Invalid AnimInstance"));
	checkf(IsValid(WeaponInstance) == true, TEXT("Invalid WeaponInstance"));

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	bIsNowAttacking = true;
	AnimInstance->PlayAnimMontage(WeaponInstance->GetMeleeAttackMontage());

	CurrentComboCount = 1;

	if (OnMeleeAttackMontageEndedDelegate.IsBound() == false)
	{
		OnMeleeAttackMontageEndedDelegate.BindUObject(this, &ThisClass::EndAttack);
		AnimInstance->Montage_SetEndDelegate(OnMeleeAttackMontageEndedDelegate, WeaponInstance->GetMeleeAttackMontage());
	}
}

void ASCharacter::OnCheckAttackInput()
{
	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
	checkf(IsValid(AnimInstance) == true, TEXT("Invalid AnimInstance"));
	checkf(IsValid(WeaponInstance) == true, TEXT("Invalid WeaponInstance"));

	if (bIsAttackKeyPressed == true)
	{
		CurrentComboCount = FMath::Clamp(CurrentComboCount + 1, 1, MaxComboCount);

		FName NextSectionName = *FString::Printf(TEXT("%s%d"), *AttackAnimMontageSectionName, CurrentComboCount);
		AnimInstance->Montage_JumpToSection(NextSectionName, WeaponInstance->GetMeleeAttackMontage());
		bIsAttackKeyPressed = false;
	}

	if (true == AnimInstance->OnCheckAttackInput.IsAlreadyBound(this, &ThisClass::OnCheckAttackInput))
	{
		AnimInstance->OnCheckAttackInput.RemoveDynamic(this, &ThisClass::OnCheckAttackInput);
	}
}

void ASCharacter::EndAttack(UAnimMontage* InMontage, bool bInterruped)
{
	checkf(IsValid(WeaponInstance) == true, TEXT("Invalid WeaponInstance"));

	ensureMsgf(CurrentComboCount != 0, TEXT("CurrentComboCount == 0"));
	CurrentComboCount = 0;
	bIsAttackKeyPressed = false;
	bIsNowAttacking = false;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	if (OnMeleeAttackMontageEndedDelegate.IsBound() == true)
	{
		OnMeleeAttackMontageEndedDelegate.Unbind();
	}
}

void ASCharacter::OnCharacterDeath()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	ASPlayerController* PlayerController = GetController<ASPlayerController>();
	if (IsValid(PlayerController) == true && HasAuthority() == true)
	{
		PlayerController->OnOwningCharacterDead();
	}
}


