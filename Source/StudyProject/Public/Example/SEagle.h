// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SFlyable.h"
#include "SEagle.generated.h"

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API USEagle 
	: public UObject
	, public ISFlyable
{
	GENERATED_BODY()
	
public:
	USEagle();

	virtual void Fly() override;

private:
	UPROPERTY()
	FString Name;

};
