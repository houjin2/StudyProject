// SWidgetComponent.cpp


#include "Component/SWidgetComponent.h"
#include "UI/StudyWidget.h"

USWidgetComponent::USWidgetComponent()
{
}

void USWidgetComponent::InitWidget()
{
	Super::InitWidget();

	UStudyWidget* SWidget = Cast<UStudyWidget>(GetWidget());
	if (true == ::IsValid(SWidget))
	{
		SWidget->SetOwningActor(GetOwner());
	}
}
