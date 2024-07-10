// BTTask_TurnToTarget.cpp

#include "AI/BTTask_TurnToTarget.h"
#include "Character/SNonPlayerCharacter.h"
#include "Controller/SAIController.h"
#include "Character/SCharacter.h"
#include "Character/SPlayerCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_TurnToTarget::UBTTask_TurnToTarget()
{
	NodeName = TEXT("TurnToTarget");
}

EBTNodeResult::Type UBTTask_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	ASAIController* AIC = Cast<ASAIController>(OwnerComp.GetAIOwner());
	checkf(IsValid(AIC) == true, TEXT("InValid AIController."));

	ASNonPlayerCharacter* NPC = Cast<ASNonPlayerCharacter>(AIC->GetPawn());
	checkf(IsValid(NPC) == true, TEXT("InValid NPC."));

	ASPlayerCharacter* TargetPC = Cast<ASPlayerCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AIC->TargetActorKey));
	if (IsValid(TargetPC) == true)
	{
		FVector LookVector = TargetPC->GetActorLocation() - NPC->GetActorLocation();
		LookVector.Z = 0.f;
		FRotator TargetRotation = FRotationMatrix::MakeFromX(LookVector).Rotator();
		NPC->SetActorRotation(FMath::RInterpTo(NPC->GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 2.f));

		return Result = EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
