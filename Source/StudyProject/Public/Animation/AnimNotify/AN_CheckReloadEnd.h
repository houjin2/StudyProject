// AN_CheckReloadEnd.h

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_CheckReloadEnd.generated.h"

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API UAN_CheckReloadEnd : public UAnimNotify
{
	GENERATED_BODY()
	
private:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference);

};
