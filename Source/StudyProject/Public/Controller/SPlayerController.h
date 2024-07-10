// SPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SPlayerController.generated.h"

class USHUD;
class USGameResultWidget;

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API ASPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ASPlayerController();

	void ToggleInGameMenu();	//ESC Menu 구현

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void OnOwningCharacterDead();

	UFUNCTION(Client, Reliable)
	void ShowWinnerUI();

	UFUNCTION(Client, Reliable)
	void ShowLoserUI(int32 InRanking);

	UFUNCTION(Client, Reliable)
	void ReturnToLobby();

protected:
	virtual void BeginPlay() override;


public:
	//생존자 수 UI
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "ASPlayerController", meta = (AllowprivateAccess))
	FText NotificationText;

private:
	UPROPERTY();
	TObjectPtr<USHUD> HUDWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowprivateAccess))
	TSubclassOf<USHUD> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASPlayerController", meta = (AllowprivateAccess))
	TSubclassOf<UUserWidget> InGameMenuClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ASPlayerController", meta = (AllowprivateAccess))
	TObjectPtr<UUserWidget> InGameMenuInstance;

	bool bIsInGameMenuOn = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASPlayerController", meta = (AllowprivateAccess))
	TSubclassOf<UUserWidget> CrosshairUiClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASPlayerController", meta = (AllowprivateAccess))
	TSubclassOf<UUserWidget> NotificationTextClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASPlayerController", meta = (AllowprivateAccess))
	TSubclassOf<USGameResultWidget> WinnerUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ASPlayerController", meta = (AllowprivateAccess))
	TSubclassOf<USGameResultWidget> LoserUIClass;
};
