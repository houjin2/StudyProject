// AN_CheckReloadEnd.cpp


#include "Animation/AnimNotify/AN_CheckReloadEnd.h"
#include "Character/SPlayerCharacter.h"

void UAN_CheckReloadEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (IsValid(MeshComp) == true)
	{
		ASPlayerCharacter* ReloadCharacter = Cast<ASPlayerCharacter>(MeshComp->GetOwner());
		if (IsValid(ReloadCharacter) == true)
		{
			ReloadCharacter->OnCheckReloadEnd();
		}
	}
}
