// SW_Bar.cpp


#include "UI/SW_Bar.h"
#include "Components/ProgressBar.h"

USW_Bar::USW_Bar(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void USW_Bar::SetMaxFigure(float InMaxFigure)
{
	if (InMaxFigure < KINDA_SMALL_NUMBER)
	{
		MaxFigure = 0.f;
		return;
	}

	MaxFigure = InMaxFigure;
}

void USW_Bar::NativeConstruct()
{
	Super::NativeConstruct();

	Bar = Cast<UProgressBar>(GetWidgetFromName("BarWidget"));
	//�ϵ��ڵ�. ���� �ٸ���� ����
	checkf(IsValid(Bar) == true, TEXT("Invalid Bar"));
}
