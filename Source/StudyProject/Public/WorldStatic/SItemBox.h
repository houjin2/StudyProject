// SItemBox.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "SItemBox.generated.h"


UCLASS()
class STUDYPROJECT_API ASItemBox : public AActor
{
	GENERATED_BODY()
	
public:	
	ASItemBox();

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	UFUNCTION()
	void OnEffectFinish(UParticleSystemComponent* ParticleSystem);

private:
	UPROPERTY(VisibleAnywhere, Category = "ASItemBox")
	TObjectPtr<UBoxComponent> BoxComponent;

	UPROPERTY(VisibleAnywhere, Category = "ASItemBox")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "ASItemBox")
	TObjectPtr<UParticleSystemComponent> ParticleSystemComponent;

};
