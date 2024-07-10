// SPlayerPawn.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SPlayerPawn.generated.h"

class UCapsuleComponent;		//�浹 ó��. ��Ʈ������Ʈ. ĳ���� ���� ���̿� ���ѷ��� ����.
class USkeletalMeshComponent;	//ĳ���� �������� �ִϸ��̼�.
class UFloatingPawnMovement;	//�÷��̾� �Է¿� ���� ������.
class USpringArmComponent;		//3��Ī �������� ī�޶� ������ ���ϰ� ������ �� �ְԲ� �ϴ� ������Ʈ.
class UCameraComponent;			//���� ������ ����� ȭ�鿡 �������ִ� ������Ʈ.

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
