// SFlyable.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SFlyable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USFlyable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STUDYPROJECT_API ISFlyable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Fly() = 0;
	// IsFlyable 인터페이스에서 구현 해도되고 안하도됨. 보통은 안함.
};
