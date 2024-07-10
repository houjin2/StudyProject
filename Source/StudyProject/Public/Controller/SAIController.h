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
	/* 관련 키 이름이 절대 변하지 않는다는 가정하에 static const를 사용해서 변수 초기값을 설정함. 
	이렇게 선언하면 향후 다른 코드에서 관련 값을 참조하기 편하지만, 하드코딩으로 값을 변경해야 한다. */
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
