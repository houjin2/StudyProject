// AN_CheckSpawnWeapon.h


#include "Animation/AnimNotify/AN_CheckSpawnWeapon.h"
#include "Character/SPlayerCharacter.h"

void UAN_CheckSpawnWeapon::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (IsValid(MeshComp) == true)
	{
		ASPlayerCharacter* PlayerCharacter = Cast<ASPlayerCharacter>(MeshComp->GetOwner());
		if (IsValid(PlayerCharacter) == true)
		{
			PlayerCharacter->OnCheckSpawnWeapon();
		}
	}
}
