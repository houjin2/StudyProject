// SPlayerPawn.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SPlayerPawn.generated.h"

class UCapsuleComponent;		//충돌 처리. 루트컴포넌트. 캐릭터 절반 높이와 몸둘레를 설정.
class USkeletalMeshComponent;	//캐릭터 랜더링과 애니메이션.
class UFloatingPawnMovement;	//플레이어 입력에 따른 움직임.
class USpringArmComponent;		//3인칭 시점으로 카메라 구도를 편리하게 설정할 수 있게끔 하는 컴포넌트.
class UCameraComponent;			//가상 세계의 모습을 화면에 전송해주는 컴포넌트.

UCLASS()
class STUDYPROJECT_API ASPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	ASPlayerPawn();

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;
private:
	void UpDown(float InAxisValue);

	void LeftRight(float InAxisValue);

protected:
	UPROPERTY(EditAnywhere, Category = "ASPlayerPawn", meta = (AllowPrivateAccess))
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(EditAnywhere, Category = "ASPlayerPawn", meta = (AllowPrivateAccess))
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(EditAnywhere, Category = "ASPlayerPawn", meta = (AllowPrivateAccess))
	TObjectPtr<UFloatingPawnMovement> FloatingPawnMovementComponent;

	UPROPERTY(EditAnywhere, Category = "ASPlayerPawn", meta = (AllowPrivateAccess))
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(EditAnywhere, Category = "ASPlayerPawn", meta = (AllowPrivateAccess))
	TObjectPtr<UCameraComponent> CameraComponent;
};
