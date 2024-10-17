// SHUD.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SHUD.generated.h"

class USStatComponent;
class ASPlayerState;
class UTextBlock;
class USW_HPBar;
class ASPlayerCharacter;

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API USHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindStatComponent(USStatComponent* InStatComponent);

	void BindPlayerState(ASPlayerState* InPlayerState);

protected:
	UFUNCTION()
	void OnKillCountChanged(int32 InOldKillCount, int32 InNewKillCount);

protected:
	TWeakObjectPtr<USStatComponent> StatComponent;

	TWeakObjectPtr<ASPlayerState> PlayerState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> KillCountText;
	// BindWidget 키워드를 통해서 하드코딩으로 속성과 위젯을 바인드 하지 않아도 됨.
	// 대신 속성의 이름과 위젯 블루프린트 속 위젯의 이름이 같이야함.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USW_HPBar> HPBar;

};
