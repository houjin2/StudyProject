// Fill out your copyright notice in the Description page of Project Settings.


#include "Example/SPigeon.h"

USPigeon::USPigeon()
{
	Name = TEXT("Pigeon");
}

void USPigeon::Fly()
{
	UE_LOG(LogTemp, Log, TEXT("%s is now flying."), *Name);

}

