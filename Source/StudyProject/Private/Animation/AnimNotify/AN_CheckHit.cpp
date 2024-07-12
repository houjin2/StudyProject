// AN_CheckHit.h


#include "Animation/AnimNotify/AN_CheckHit.h"
#include "Character/SPlayerCharacter.h"

void UAN_CheckHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (IsValid(MeshComp) == true)
	{
		ASCharacter* AttackingCharacter = Cast<ASCharacter>(MeshComp->GetOwner());
		if (IsValid(AttackingCharacter) == true)
		{
			AttackingCharacter->OnCheckHit();
		}
	}
}
