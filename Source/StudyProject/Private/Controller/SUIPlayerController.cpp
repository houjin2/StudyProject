// SUIPlayerController.cpp


#include "Controller/SUIPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"


void ASUIPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(UIWidgetClass) == true)
	{
		UIWidgetInstance = CreateWidget<UUserWidget>(this, UIWidgetClass);
		// CreateWidget()이 호출될 때 UIWidgetInstance->NativeOnInitialize() 함수가 호출됨.
		if (IsValid(UIWidgetInstance) == true)
		{
			UIWidgetInstance->AddToViewport();
			// AddToViewport()가 호출 될 때 UIWidgetInstance->NativeConstruct() 함수가 호출됨.

			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(UIWidgetInstance->GetCachedWidget());
			SetInputMode(Mode);

			bShowMouseCursor = true;
		}
	}
}

void ASUIPlayerController::JoinServer(const FString& InIPAddress)
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Loading"), true, FString::Printf(TEXT("NextLevel=%s?Saved=false"), *InIPAddress));
}
