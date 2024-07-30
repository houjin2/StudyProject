// AN_CheckSpawnWeapon.h

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_CheckSpawnWeapon.generated.h"

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API UAN_CheckSpawnWeapon : public UAnimNotify
{
	GENERATED_BODY()

private:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference);

};
