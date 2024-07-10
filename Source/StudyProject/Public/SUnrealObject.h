// SUnrealObject.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SUnrealObject.generated.h"

/**
 * 
 */
UCLASS()
class STUDYPROJECT_API USUnrealObject : public UObject
{
	GENERATED_BODY()

public:
	USUnrealObject();

	UFUNCTION()
	void HelloUnreal();

	const FString& GetObjectName() const { return Name; }

protected:
	UPROPERTY()
	FString Name;
};
