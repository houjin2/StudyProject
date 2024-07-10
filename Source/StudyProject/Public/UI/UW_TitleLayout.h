// UW_TitleLayout.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_TitleLayout.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API UUW_TitleLayout : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UUW_TitleLayout(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnPlayButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USTitleWidget", meta = (AllowprivateAccess, BindWidget))
	TObjectPtr<UButton> PlayButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USTitleWidget", meta = (AllowprivateAccess, BindWidget))
	TObjectPtr<UButton> ExitButton;

};
