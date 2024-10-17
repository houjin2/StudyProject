// SW_Bar.h

#pragma once

#include "CoreMinimal.h"
#include "UI/StudyWidget.h"
#include "SW_Bar.generated.h"

class UProgressBar;

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API USW_Bar : public UStudyWidget
{
	GENERATED_BODY()
	
public:
	USW_Bar(const FObjectInitializer& ObjectInitializer);
	// ���� Widget Ŭ������ �����ڿ� ������ �ۼ��Ϸ��� �ݵ�� �� �����ڸ� ����/�����ؾ���.

	void SetMaxFigure(float InMaxFigure);

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USW_Bar")
	TObjectPtr<UProgressBar> Bar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USW_Bar")
	float MaxFigure;
};
