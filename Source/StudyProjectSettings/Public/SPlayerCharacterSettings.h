// SPlayerCharacterSettings.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SPlayerCharacterSettings.generated.h"

/**
 * 
 */
UCLASS(config = PlayerCharacterMeshMaterialPaths) // 1. �𸮾� ������  �ʱ�ȭ �ܰ迡�� Config ������ ��ġ�� DefaultPlayerCharacterMeshPaths.ini ������ �о����.
class STUDYPROJECTSETTINGS_API USPlayerCharacterSettings : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(config)	// 2. �о���� PlayerCharacterMeshMaterialPaths.ini ������ �������� �ش� ����� �⺻���� �ʱ�ȭ��.
	TArray<FSoftObjectPath> PlayerCharacterMeshMaterialPaths;
};
