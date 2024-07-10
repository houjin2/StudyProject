// BTTask_Attack.cpp


#include "AI/BTTask_Attack.h"
#include "Controller/SAIController.h"
#include "Character/SNonPlayerCharacter.h"

UBTTask_Attack::UBTTask_Attack()
{
    bNotifyTick = true;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    ASAIController* AIC = Cast<ASAIController>(OwnerComp.GetAIOwner());
    checkf(IsValid(AIC) == true, TEXT("InValid AIController."));

    ASNonPlayerCharacter* NPC = Cast<ASNonPlayerCharacter>(AIC->GetPawn());
    checkf(IsValid(NPC) == true, TEXT("InValid NPC."));

    if (NPC->bIsNowAttacking == false)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    ASAIController* AIC = Cast<ASAIController>(OwnerComp.GetAIOwner());
    checkf(IsValid(AIC) == true, TEXT("InValid AIController."));

    ASNonPlayerCharacter* NPC = Cast<ASNonPlayerCharacter>(AIC->GetPawn());
    checkf(IsValid(NPC) == true, TEXT("InValid NPC."));

    NPC->BeginAttack();


    return EBTNodeResult::InProgress;
}
