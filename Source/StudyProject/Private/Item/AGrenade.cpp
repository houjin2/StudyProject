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
    ProjectileMovementComponent->InitialSpeed = 3000.f;  // 발사 속도 설정
    ProjectileMovementComponent->MaxSpeed = 3000.f;      // 최대 속도 설정
    ProjectileMovementComponent->bRotationFollowsVelocity = true; // 회전이 방향을 따르도록 설정
    ProjectileMovementComponent->bShouldBounce = false;   // 바운스 여부 설정
    //ProjectileMovementComponent->Bounciness = 0.3f;      // 바운스 정도
    ProjectileMovementComponent->ProjectileGravityScale = 0.8f;  // 중력 설정

    RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
    RadialForceComponent->SetupAttachment(RootComponent);
    RadialForceComponent->Radius = 500.f; // 폭발범위
    RadialForceComponent->ImpulseStrength = 50000.f; // 폭발강도
    RadialForceComponent->bImpulseVelChange = true;
    RadialForceComponent->bAutoActivate = false; // Will be activated manually during explosion
    RadialForceComponent->bIgnoreOwningActor = true; // Don't affect the owner

    bReplicates = true;  // 객체가 복제될 수 있도록 설정
    SetReplicateMovement(true);  // 객체의 움직임을 네트워크 상에서 복제

    GrenadeMesh->OnComponentHit.AddDynamic(this, &AAGrenade::OnHit);
    GrenadeMesh->SetVisibility(true);  // 메시가 보이도록 설정
    GrenadeMesh->SetSimulatePhysics(true);  // 물리 활성화 필요 시
    GrenadeMesh->SetNotifyRigidBodyCollision(true);  // 충돌 감지 활성화

}

// Called when the game starts or when spawned
void AAGrenade::BeginPlay()
{
    Super::BeginPlay();

    if (GetNetMode() != NM_DedicatedServer)  // 클라이언트에서만 실행
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
    if (HasAuthority())  // 서버에서만 데미지 처리
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

    // NetMulticast로 모든 클라이언트에서 폭발 효과 재생
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