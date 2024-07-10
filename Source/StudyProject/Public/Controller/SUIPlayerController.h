// SUIPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SUIPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API ASUIPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	//서버 접속
	void JoinServer(const FString& InIPAddress);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ASUIPlayerController", meta = (AllowprivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ASUIPlayerController", meta = (AllowprivateAccess))
	TObjectPtr<UUserWidget> UIWidgetInstance;
};
