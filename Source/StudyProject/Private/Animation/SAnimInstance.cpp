// SAnimInstance.cpp


#include "Animation/SAnimInstance.h"
#include "Character/SPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/SStatComponent.h"

USAnimInstance::USAnimInstance()
{
}

void USAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CurrentSpeed = 0.f;

	Velocity = FVector::ZeroVector;

	bIsFalling = false;

	bIsCrouching = false;

	bIsDead = false;
}

void USAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ASCharacter* OwnerCharacter = Cast<ASCharacter>(TryGetPawnOwner());	//Try = 가져올 PawnOwner가 죽었을지, 있을지 없을지 모르기 때문에 사용

	if (IsValid(OwnerCharacter) == true)
	{
		UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
		if (IsValid(CharacterMovementComponent) == true)
		{
			Velocity = CharacterMovementComponent->GetLastUpdateVelocity();
			CurrentSpeed = Velocity.Size();
			/*	IsFalling(): 현재 공중에 떠있는지		IsSwimming(): 현재 수영 중인지
				IsCrouching(): 현재 쭈그려 앉아있는지	IsMoveOnGround(): 땅 위에서 이동 중인지
			*/
			bIsFalling = CharacterMovementComponent->IsFalling();
			bIsCrouching = CharacterMovementComponent->IsCrouching();
			bIsDead = OwnerCharacter->GetStatComponent()->GetCurrentHP() <= KINDA_SMALL_NUMBER;


			//Animation Layer
			Acceleration = CharacterMovementComponent->GetCurrentAcceleration();

			if (Acceleration.Length() < KINDA_SMALL_NUMBER && Velocity.Length() < KINDA_SMALL_NUMBER)
			{
				LocomotionState = ELocomotionState::Idle;
			}
			else
			{
				LocomotionState = ELocomotionState::Walk;
			}

			ASPlayerCharacter* OwnerPlayerCharacter = Cast<ASPlayerCharacter>(OwnerCharacter);
			if (IsValid(OwnerPlayerCharacter) == true)
			{
				if (KINDA_SMALL_NUMBER < OwnerPlayerCharacter->GetForwardInputValue())
				{
					MovementDirection = EMovementDirection::Fwd;
				}

				if (OwnerPlayerCharacter->GetForwardInputValue() < -KINDA_SMALL_NUMBER)
				{
					MovementDirection = EMovementDirection::Bwd;
				}

				if (KINDA_SMALL_NUMBER < OwnerPlayerCharacter->GetRightInputValue())
				{
					MovementDirection = EMovementDirection::Right;
				}

				if (OwnerPlayerCharacter->GetRightInputValue() < -KINDA_SMALL_NUMBER)
				{
					MovementDirection = EMovementDirection::Left;
				}

				//Aimoffset
				ControlRotation.Pitch = OwnerPlayerCharacter->GetCurrentAimPitch();
				ControlRotation.Yaw = OwnerPlayerCharacter->GetCurrentAimYaw();
			}
		}
	}

}

void USAnimInstance::PlayAnimMontage(UAnimMontage* InAnimMontage)
{
	checkf(IsValid(InAnimMontage) == true, TEXT("InValid AnimMontage"));

	if (Montage_IsPlaying(InAnimMontage) == false)
	{
		Montage_Play(InAnimMontage);
	}
}

void USAnimInstance::AnimNotify_CheckHit()
{
	if (OnCheckHit.IsBound() == true)	// 해당 델리게이트에 1개의 함수라도 바인드 되어있다면 true 반환
	{
		OnCheckHit.Broadcast();
	}
}

void USAnimInstance::AnimNotify_CheckAttackInput()
{
	if (OnCheckAttackInput.IsBound() == true)	// 해당 델리게이트에 1개의 함수라도 바인드 되어있다면 true 반환
	{
		OnCheckAttackInput.Broadcast();
	}
}
