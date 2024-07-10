// SWidgetComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "SWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API USWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	USWidgetComponent();

	virtual void InitWidget() override;
};
