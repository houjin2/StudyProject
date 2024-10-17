//AGrenade.h

#pragma once

#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AGrenade.generated.h"


UCLASS()
class STUDYPROJECT_API AAGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAGrenade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void Explode();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, Reliable)
	void ExplosionEffect_NetMulticast();
public:
	// Projectile Movement Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	class UProjectileMovementComponent* ProjectileMovementComponent;

	// Mesh component for the grenade
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* GrenadeMesh;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class URadialForceComponent* RadialForceComponent;

	// Function to initialize the grenade's velocity in the specified direction
	void FireInDirection(const FVector& ShootDirection);

	UPROPERTY(Replicated, EditAnywhere, Category = "Effects")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(Replicated, EditAnywhere, Category = "Effects")
	USoundCue* ExplosionSound;
};
