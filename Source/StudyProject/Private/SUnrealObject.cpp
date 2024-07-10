// SUnrealObject.cpp


#include "SUnrealObject.h"


USUnrealObject::USUnrealObject()
{
	Name = TEXT("USUnrealObject CDO");
}

void USUnrealObject::HelloUnreal()
{
	UE_LOG(LogTemp, Log, TEXT("USUnrealObjectClass::HelloUnreal() has been called."));
}
