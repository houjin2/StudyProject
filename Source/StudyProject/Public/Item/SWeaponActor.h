// SWeaponActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeaponActor.generated.h"

class UAnimMontage;
class UAnimInstance;

UCLASS()
class STUDYPROJECT_API ASWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASWeaponActor();

	USkeletalMeshComponent* GetMesh() const { return Mesh; }

	UAnimMontage* GetMeleeAttackMontage() const { return MeleeAttackMontage; }

	TSubclassOf<UAnimInstance> GetUnarmedCharacterAnimLayer() const { return UnarmedCharacterAnimLayer; }	//AnimLayer

	TSubclassOf<UAnimInstance> GetArmedCharacterAnimLayer() const { return ArmedCharacterAnimLayer; }		//AnimLayer

	UAnimMontage* GetEquipAnimMontage() const { return EquipAnimMontage; }

	UAnimMontage* GetUnequipAnimMontage() const { return UnquipAnimMontage; }

	//사격구현
	float GetMaxRange() const { return MaxRange; }

	UAnimMontage* GetRifleFireAnimMontage() const { return RifleFireAnimMontage; }

	UAnimMontage* GetRifleReloadAnimMontage() const { return RifleReloadAnimMontage; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<UAnimMontage> MeleeAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASWeaponActor|AnimLayer", meta = (AllowprivateAccess))
	TSubclassOf<UAnimInstance> UnarmedCharacterAnimLayer;	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASWeaponActor|AnimLayer", meta = (AllowprivateAccess))
	TSubclassOf<UAnimInstance> ArmedCharacterAnimLayer;		//Unarmed는 항상 같지만 Armed는 총마다 달라진다.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<UAnimMontage> EquipAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<UAnimMontage> UnquipAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess, Units=cm))
	float MaxRange = 25000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<UAnimMontage> RifleFireAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<UAnimMontage> RifleReloadAnimMontage;
};
