// SAnimInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SAnimInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheckHit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheckAttackInput);

class UAnimMontage;

UENUM(BlueprintType)
enum class ELocomotionState : uint8
{
	None,
	Idle,
	Walk,
	End
};

UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	None,
	Fwd,
	Bwd,
	Left,
	Right,
	End
};

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API USAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	USAnimInstance();

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void PlayAnimMontage(UAnimMontage* InAnimMontage);

	ELocomotionState GetLocomotionState() const { return LocomotionState; }

	EMovementDirection GetMovementDirection() const { return MovementDirection; }

protected:
	UFUNCTION()
	void AnimNotify_CheckHit();

	UFUNCTION()
	void AnimNotify_CheckAttackInput();

public:
	FOnCheckHit OnCheckHit;

	FOnCheckAttackInput OnCheckAttackInput;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SAnimInstance", meta = (AllowprivateAccess))
	float CurrentSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SAnimInstance", meta = (AllowprivateAccess))
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SAnimInstance", meta = (AllowprivateAccess))
	uint8 bIsFalling : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SAnimInstance", meta = (AllowprivateAccess))
	uint8 bIsCrouching : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SAnimInstance", meta = (AllowprivateAccess))
	uint8 bIsDead : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SAnimInstance", meta = (AllowPrivateAccess))
	FVector Acceleration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ELocomotionState LocomotionState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMovementDirection MovementDirection;

	//Aimoffset
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SAnimInstance", meta = (AllowprivateAccess))
	FRotator ControlRotation;
};
