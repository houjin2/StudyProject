// SNonPlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "Character/SCharacter.h"
#include "SNonPlayerCharacter.generated.h"

class USWidgetComponent;

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API ASNonPlayerCharacter : public ASCharacter
{
	GENERATED_BODY()
	
	friend class UBTTask_Attack;

public:
	ASNonPlayerCharacter();

	virtual void BeginPlay() override;
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetWidget(UStudyWidget* InStudyWidget) override;

protected:
	virtual void BeginAttack() override;

	virtual void EndAttack(UAnimMontage* InMontage, bool bInterruped) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<UAnimMontage> MeleeAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<USWidgetComponent> WidgetComponent;
};
