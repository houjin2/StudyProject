// UW_LobbyLayout.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_LobbyLayout.generated.h"

class UButton;
class UEditableText;
struct FStreamableHandle;
class UMaterialInstance;

/**
 *
 */
UCLASS()
class STUDYPROJECT_API UUW_LobbyLayout : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnBlackTeamButtonClicked();

	UFUNCTION()
	void OnWhiteTeamButtonClicked();

	UFUNCTION()
	void OnSubmitButtonClicked();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USLobbyLevelUI", meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> BlackTeamButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USLobbyLevelUI", meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> WhiteTeamButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USLobbyLevelUI", meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UEditableText> EditPlayerName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USLobbyLevelUI", meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> SubmitButton;

	TWeakObjectPtr<USkeletalMeshComponent> CurrentSkeletalMeshComponent;

	TArray<TSharedPtr<FStreamableHandle>> StreamableHandles;

	TArray<TSoftObjectPtr<UMaterial>> LoadedMaterialInstanceAssets;

	uint8 SelectedTeam = 1;

	//���� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "USLobbyLevelUI", meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UEditableText> EditServerIP;
};
