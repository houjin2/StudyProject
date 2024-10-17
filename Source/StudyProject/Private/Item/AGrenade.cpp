// AGrenade.cpp


#include "Item/AGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Classes/PhysicsEngine/RadialForceComponent.h"
#include "Kismet//GameplayStatics.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AAGrenade::AAGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Create a Static Mesh Component for the grenade
    GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
    RootComponent = GrenadeMesh;

    // Create a Projectile Movement Component and configure its properties
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->InitialSpeed = 3000.f;  // �߻� �ӵ� ����
    ProjectileMovementComponent->MaxSpeed = 3000.f;      // �ִ� �ӵ� ����
    ProjectileMovementComponent->bRotationFollowsVelocity = true; // ȸ���� ������ �������� ����
    ProjectileMovementComponent->bShouldBounce = false;   // �ٿ ���� ����
    //ProjectileMovementComponent->Bounciness = 0.3f;      // �ٿ ����
    ProjectileMovementComponent->ProjectileGravityScale = 0.8f;  // �߷� ����

    RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
    RadialForceComponent->SetupAttachment(RootComponent);
    RadialForceComponent->Radius = 500.f; // ���߹���
    RadialForceComponent->ImpulseStrength = 50000.f; // ���߰���
    RadialForceComponent->bImpulseVelChange = true;
    RadialForceComponent->bAutoActivate = false; // Will be activated manually during explosion
    RadialForceComponent->bIgnoreOwningActor = true; // Don't affect the owner

    bReplicates = true;  // ��ü�� ������ �� �ֵ��� ����
    SetReplicateMovement(true);  // ��ü�� �������� ��Ʈ��ũ �󿡼� ����

    GrenadeMesh->OnComponentHit.AddDynamic(this, &AAGrenade::OnHit);
    GrenadeMesh->SetVisibility(true);  // �޽ð� ���̵��� ����
    GrenadeMesh->SetSimulatePhysics(true);  // ���� Ȱ��ȭ �ʿ� ��
    GrenadeMesh->SetNotifyRigidBodyCollision(true);  // �浹 ���� Ȱ��ȭ

}

// Called when the game starts or when spawned
void AAGrenade::BeginPlay()
{
    Super::BeginPlay();

    if (GetNetMode() != NM_DedicatedServer)  // Ŭ���̾�Ʈ������ ����
    {
        FString NetRoleString;
        switch (GetLocalRole())
        {
        case ROLE_Authority:
            NetRoleString = "Authority";
            break;
        case ROLE_AutonomousProxy:
            NetRoleString = "Autonomous Proxy";
            break;
        case ROLE_SimulatedProxy:
            NetRoleString = "Simulated Proxy";
            break;
        default:
            NetRoleString = "None";
            break;
        }
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade NetMode: %d, Role: %s"), GetNetMode(), *NetRoleString));
    }
}

void AAGrenade::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade Hit Detected")));


    Explode();
    Destroy();
}

void AAGrenade::Explode()
{
    if (HasAuthority())  // ���������� ������ ó��
    {
        RadialForceComponent->FireImpulse();

        TArray<AActor*> IgnoredActors;
        UGameplayStatics::ApplyRadialDamage(
            GetWorld(),
            50.0f,
            GetActorLocation(),
            RadialForceComponent->Radius,
            UDamageType::StaticClass(),
            IgnoredActors,
            this,
            GetInstigatorController(),
            true
        );
    }

    // NetMulticast�� ��� Ŭ���̾�Ʈ���� ���� ȿ�� ���
    ExplosionEffect_NetMulticast();
}

void AAGrenade::ExplosionEffect_NetMulticast_Implementation()
{
    if (ExplosionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
    }
    else
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ExplosionEffect is nullptr")));
    }

    if (ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
    }
    else
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ExplosionSound is nullptr")));
    }
}

void AAGrenade::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, GrenadeMesh);
    DOREPLIFETIME(ThisClass, RadialForceComponent);
    DOREPLIFETIME(ThisClass, ExplosionEffect);
    DOREPLIFETIME(ThisClass, ExplosionSound);
}

// Set the direction of the grenade's movement
void AAGrenade::FireInDirection(const FVector& ShootDirection)
{
    if (ProjectileMovementComponent)
    {
        ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
    }
}