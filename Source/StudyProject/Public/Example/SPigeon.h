// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SFlyable.h"
#include "SPigeon.generated.h"

USTRUCT()
struct FPigeonData
{
	GENERATED_BODY()

public:
	FPigeonData()
	{
	}

	FPigeonData(const FString& InName, int32 InID)
		: Name(InName)
		, ID(InID)
	{
	}

	friend FArchive& operator<<(FArchive& InArchive, FPigeonData& InPigeonData)
	{
		InArchive << InPigeonData.Name;
		InArchive << InPigeonData.ID;
		return InArchive;
	}

public:
	UPROPERTY()
	FString Name;

	UPROPERTY()
	int32 ID;
};

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API USPigeon 
	: public UObject
	, public ISFlyable
{
	GENERATED_BODY()
	
public:
	USPigeon();

	virtual void Fly() override;

	const FString& GetPigeonName() const { return Name; }
	void SetPigeonName(const FString& InName) { Name = InName; }

	int32 GetPigeonID() const { return ID; }
	void SetPigeonID(const int32& InID) { ID = InID; }



public:


private:
	UPROPERTY()
	FString Name;

	UPROPERTY()
	int32 ID;

};
