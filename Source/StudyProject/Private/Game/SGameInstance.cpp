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
	Super::Init(); // ���� ������Ʈ ��ƾ�� ��Ű�� ���ؼ�, �𸮾� �����Ͼ �ۼ��� �ڵ尡 ���� ����ǰԲ� �ϱ� ����.

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
