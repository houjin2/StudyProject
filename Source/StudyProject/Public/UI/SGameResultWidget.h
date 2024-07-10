// SGameResultWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SGameResultWidget.generated.h"

class UTextBlock;
class UButton;

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API USGameResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnReturnToLobbyButtonClicked();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USGameResultWidget", meta = (BindWidget))
	TObjectPtr<UTextBlock> RankingText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USGameResultWidget", meta = (BindWidget))
	TObjectPtr<UButton> ReturnToLobbyButton;
};
