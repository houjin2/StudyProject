// SPlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "Character/SCharacter.h"
#include "InputActionValue.h"
#include "Item/SWeaponActor.h"
#include "Game/SPlayerState.h"
#include "SPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USInputConfig;
class UInputMappingContext;
class UAnimMontage;
class UParticleSystemComponent;
struct FStreamableHandle;
class UCameraShakeBase;

UENUM(BlueprintType)
enum class EViewMode :uint8
{
	None,
	BackView,
	QuarterView,
	TPSView,
	End
};

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API ASPlayerCharacter : public ASCharacter
{
	GENERATED_BODY()
	
	friend class UAN_CheckReloadEnd;
	
	friend class UAN_CheckSpawnWeapon;

public:
	ASPlayerCharacter();

	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	void SetViewMode(EViewMode InViewMode);

	virtual void Tick(float DeltaSeconds) override;

	float GetForwardInputValue() const { return ForwardInputValue; }

	float GetRightInputValue() const { return RightInputValue; }

	//kill count
	UParticleSystemComponent* GetParticleSystem() const { return ParticleSystemComponent; }

	void SetMeshMaterial(const EPlayerTeam& InPlayerTeam);

	//Aimoffset
	float GetCurrentAimPitch() const { return CurrentAimPitch; }

	float GetCurrentAimYaw() const { return CurrentAimYaw; }
	
	float GetCurrentAmmo() const { return CurrentAmmo; }

	float GetMagazine() const { return Magazine; }

	//�ǰ� ����
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void SetCurrentWeapon_Server(const FString& NewWeapon);

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void OnCheckReloadEnd();

	UFUNCTION()
	void OnCheckSpawnWeapon();

private:
	void InputMove(const FInputActionValue& InValue);

	void InputLook(const FInputActionValue& InValue);

	void InputChangeView(const FInputActionValue& InValue);

	void InputQuickSlot01(const FInputActionValue& InValue);

	void InputQuickSlot02(const FInputActionValue& InValue);

	void InputQuickSlot03(const FInputActionValue& InValue);

	void InputAttack(const FInputActionValue& InValue);

	void InputMenu(const FInputActionValue& InValue);

	void InputLookSpeedUp(const FInputActionValue& InValue);	//���콺 ���� ����

	void InputLookSpeedDown(const FInputActionValue& InValue);	//���콺 ���� ����

	void InputReload(const FInputActionValue& InValue);


	//��ݱ���
	void TryFire();

	void Reload();

	void StartIronSight(const FInputActionValue& InValue);

	void EndIronSight(const FInputActionValue& InValue);

	void ToggleTrigger(const FInputActionValue& InValue);

	void StartFire(const FInputActionValue& InValue);

	void StopFire(const FInputActionValue& InValue);

	void SpawnLandMine(const FInputActionValue& InValue);

	UFUNCTION(Server, Reliable, WithValidation)
	void SpawnLandMine_Server();

	//�ǰ� ��� - �κ� ����
	UFUNCTION()
	void OnHittedRagdollRestoreTimerElapsed();

	//���� ���� ����ȭ
	UFUNCTION(Server, Reliable)
	void SpawnWeaponInstance1_Server();

	UFUNCTION(Server, Reliable)
	void SpawnWeaponInstance2_Server();

	UFUNCTION(Server, Reliable)
	void DestroyWeaponInstance_Server();

	virtual void OnRep_WeaponInstance();

	//�ִϸ��̼� ����ȭ - ���⵿��ȭ
	UFUNCTION(Server, Unreliable)	//�ѵι������� ������ ������ ������ Unreliable���
	void UpdateInputValue_Server(const float& InForwardInputValue, const float& InRightInputValue);

	UFUNCTION(Server, Unreliable)
	void UpdateAimValue_Server(const float& InAimPitchValue, const float& InAimYawValue);

	//��� ����ȭ
	UFUNCTION(Server, Unreliable)
	void PlayAttackMontage_Server();

	UFUNCTION(NetMulticast, Unreliable)
	void PlayAttackMontage_NetMulticast();

	//������ ����ȭ
	UFUNCTION(Server, Reliable)
	void ApplyDamageAndDrawLine_Server(FHitResult HitResult);

	UFUNCTION(NetMulticast, Reliable)
	void DrawLine_NetMulticast(const FVector& InDrawStart, const FVector& InDrawEnd);

	//�ǰ� ��� ����ȭ
	UFUNCTION(NetMulticast, Unreliable)
	void PlayRagdoll_NetMulticast();

	//���� ��� ����ȭ
	UFUNCTION(Server, Unreliable)
	void PlayReloadMontage_Server();

	UFUNCTION(NetMulticast, Unreliable)
	void PlayReloadMontage_NetMulticast();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<USInputConfig> PlayerCharacterInputConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<UInputMappingContext> PlayerCharacterInputMappingContext;

	EViewMode CurrentViewMode = EViewMode::None;	//UPROPERTY() ��ũ�θ� ������� �����Ƿ� �ʱ�ȭ �ʼ�.

	FVector DirectionToMove = FVector::ZeroVector;

	float DestArmLength = 0.f;

	float ArmLengthChangeSpeed = 3.f;

	FRotator DestArmRotation = FRotator::ZeroRotator;

	float ArmRotationChangeSpeed = 10.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	float ForwardInputValue;

	//���� ����ȭ
	float PreviousForwardInputValue = 0.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	float RightInputValue;

	//���� ����ȭ
	float PreviousRightInputValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<UParticleSystemComponent> ParticleSystemComponent;

	FSoftObjectPath CurrentPlayerCharacterMeshMaterialPath = FSoftObjectPath();

	TSharedPtr<FStreamableHandle> AssetStreamableHandle = nullptr;

	//��ݱ��� - ��
	float TargetFOV = 70.f;	

	float CurrentFOV = 70.f;

	//��ݱ��� - ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowprivateAccess))
	float FirePerMinute = 600;

	bool bIsTriggerToggle = false;

	FTimerHandle BetweenShotsTimer;

	float TimeBetweenFire;


	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float CurrentAimPitch = 0.f;

	//���ع��� ����ȭ
	float PreviousAimPitch = 0.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float CurrentAimYaw = 0.f;
	
	//���ع��� ����ȭ
	float PreviousAimYaw = 0.f;


	//CameraShake
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ASPlayerCharacter", meta = (AllowprivateAccess))
	TSubclassOf<UCameraShakeBase> FireShake;

	//�ǰ� ��� - �κ� ����
	FTimerHandle HittedRagdollRestoreTimer;

	FTimerDelegate HittedRagdollRestoreTimerDelegate;

	float TargetRagdollBlendWeight = 0.f;

	float CurrentRagdollBlendWeight = 0.f;

	bool bIsNowRagdollBlending = false;

	//Mine Spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASPlayerCharacter", meta = (AllowpricateAccess))
	TSubclassOf<AActor> LandMineClass;

	//���� ���� ����ȭ
	UPROPERTY()
	TSubclassOf<UAnimInstance> UnarmedCharacterAnimLayer;

	UPROPERTY()
	TObjectPtr<UAnimMontage> UnequipAnimMontage;



	float LookSpeed = 1.f;

	int32 RifleAmmo = 30;

	int32 RifleMagazine = 30;

	int32 ShotgunAmmo = 8;

	int32 ShotgunMagazine = 8;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ASPlayerStat", meta = (AllowprivateAccess))
	int32 CurrentAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ASPlayerStat", meta = (AllowprivateAccess))
	int32 Magazine;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ASPlayerStat", meta = (AllowprivateAccess))
	bool IsReloading;

	UPROPERTY(Replicated)
	FString CurrentWeapon;
};
