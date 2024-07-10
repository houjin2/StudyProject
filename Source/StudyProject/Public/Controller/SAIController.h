// SAIController.h

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SAIController.generated.h"

class UBlackboardData;
class UBehaviorTree;

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API ASAIController : public AAIController
{
	GENERATED_BODY()
	
	friend class ASNonPlayerCharacter;
public:
	ASAIController();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void BeginAI(APawn* InPawn);

	void EndAI();

public:
	/* ���� Ű �̸��� ���� ������ �ʴ´ٴ� �����Ͽ� static const�� ����ؼ� ���� �ʱⰪ�� ������. 
	�̷��� �����ϸ� ���� �ٸ� �ڵ忡�� ���� ���� �����ϱ� ��������, �ϵ��ڵ����� ���� �����ؾ� �Ѵ�. */
	static const float PatrolRadius;

	static int32 ShowAIDebug;

	static const FName StartPatrolPositionKey;

	static const FName EndPatrolPositionKey;

	static const FName TargetActorKey;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<UBlackboardData> BlackboardDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowprivateAccess))
	TObjectPtr<UBehaviorTree> BehaviorTree;
};
