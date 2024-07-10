// SGameInstance.cpp


#include "Game/SGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SUnrealObject.h"
#include "Example/SEagle.h"
#include "Example/SFlyable.h"
#include "Example/SPigeon.h"
#include "JsonObjectConverter.h"
#include "UObject//SavePackage.h"


void USGameInstance::Init()
{
	Super::Init(); // 엔진 업데이트 루틴을 지키기 위해서, 언리얼 엔지니어가 작성한 코드가 먼저 실행되게끔 하기 위함.

	if (IsValid(CharacterStatDataTable) == false || CharacterStatDataTable->GetRowMap().Num() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Not Enough data in CharacterStatDataTable"));
	}
	else
	{
		for (int32 i = 1; i <= CharacterStatDataTable->GetRowMap().Num(); ++i)
		{
			check(nullptr != GetCharacterStatDataTableRow(i));
		}
	}
}

void USGameInstance::Shutdown()
{
	Super::Shutdown();
}

FSStatTableRow* USGameInstance::GetCharacterStatDataTableRow(int32 InLevel)
{
	if (IsValid(CharacterStatDataTable) == true)
	{
		return CharacterStatDataTable->FindRow<FSStatTableRow>(*FString::FromInt(InLevel), TEXT(""));
	}

	return nullptr;
}
