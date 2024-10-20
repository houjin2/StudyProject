// SPlayerCharacter.cpp


#include "Character/SPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Input/SInputConfig.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/SWeaponActor.h"
#include "Animation/SAnimInstance.h"
#include "Particles/ParticleSystemComponent.h"
#include "Component/SStatComponent.h"
#include "SPlayerCharacterSettings.h"		//비동기 에셋
#include "Engine/AssetManager.h"			//비동기 에셋
#include "Engine/StreamableManager.h"		//비동기 에셋
#include "Controller/SPlayerController.h"	//Menu구현
#include "Engine/EngineTypes.h"				//사격구현
#include "Engine/DamageEvents.h"			//사격구현
#include "WorldStatic/SLandMine.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"			//사격 동기화
#include "Kismet/KismetSystemLibrary.h"		//사격 동기화
#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"



ASPlayerCharacter::ASPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 400.f;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->SetupAttachment(GetRootComponent());
	ParticleSystemComponent->SetAutoActivate(false);

	const USPlayerCharacterSettings* CDO = GetDefault<USPlayerCharacterSettings>();
	if (0 < CDO->PlayerCharacterMeshMaterialPaths.Num())
	{
		for (FSoftObjectPath PlayerCharacterMeshPath : CDO->PlayerCharacterMeshMaterialPaths)
		{
			UE_LOG(LogTemp, Warning, TEXT("Path: %s"), *(PlayerCharacterMeshPath.ToString()));
		}
	}

	TimeBetweenFire = 60.f / FirePerMinute;

	IsReloading = false;
	Magazine = 30;
	CurrentAmmo = Magazine;

	static ConstructorHelpers::FClassFinder<AAGrenade> GrenadeBPClass(TEXT("'/Game/StudyProject/Item/BP_Grenade.BP_Grenade_C'"));
	if (GrenadeBPClass.Succeeded())
	{
		GrenadeClass = GrenadeBPClass.Class;
	}

}

void ASPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (IsValid(PlayerController) == true)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (IsValid(Subsystem) == true)
		{
			Subsystem->AddMappingContext(PlayerCharacterInputMappingContext, 0);
		}
	}

	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
	if (true == ::IsValid(AnimInstance))
	{
		AnimInstance->OnCheckReloadEnd.AddDynamic(this, &ThisClass::OnCheckReloadEnd);
		AnimInstance->OnCheckSpawnWeapon.AddDynamic(this, &ThisClass::OnCheckSpawnWeapon);
	}

	/*
	const USPlayerCharacterSettings* CDO = GetDefault<USPlayerCharacterSettings>();
	int32 RandIndex = FMath::RandRange(0, CDO->PlayerCharacterMeshMaterialPaths.Num() - 1);
	CurrentPlayerCharacterMeshMaterialPath = CDO->PlayerCharacterMeshMaterialPaths[RandIndex];
	AssetStreamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		CurrentPlayerCharacterMeshMaterialPath,
		FStreamableDelegate::CreateLambda([this]() -> void
			{
				AssetStreamableHandle->ReleaseHandle();
				TSoftObjectPtr<UMaterial> LoadedMaterialInstanceAsset(CurrentPlayerCharacterMeshMaterialPath);
				if (LoadedMaterialInstanceAsset.IsValid() == true)
				{
					GetMesh()->SetMaterial(0, LoadedMaterialInstanceAsset.Get());
				}
			})
	);
	*/
	SetViewMode(EViewMode::TPSView);

}

void ASPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

}

void ASPlayerCharacter::SetViewMode(EViewMode InViewMode)
{
	if (CurrentViewMode == InViewMode)
	{
		return;
	}

	CurrentViewMode = InViewMode;

	switch (CurrentViewMode)
	{
	case EViewMode::BackView:
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = false;
		bUseControllerRotationRoll = false;

		//SpringArmComponent->TargetArmLength = 400.f;
		//SpringArmComponent->SetRelativeRotation(FRotator::ZeroRotator);
		// ControlRotation이 Pawn의 회전과 동기화 -> Pawn의 회전이 SpringArm의 회전 동기화. 이로 인해 SetRotation()이 무의미.

		SpringArmComponent->bUsePawnControlRotation = true;

		SpringArmComponent->bInheritPitch = true;
		SpringArmComponent->bInheritYaw = true;
		SpringArmComponent->bInheritRoll = false;

		SpringArmComponent->bDoCollisionTest = true;
		
		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;

		break;
	case EViewMode::QuarterView:
	{
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = false;
		bUseControllerRotationRoll = false;

		//SpringArmComponent->TargetArmLength = 800.f;
		//SpringArmComponent->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));

		SpringArmComponent->bUsePawnControlRotation = false;

		SpringArmComponent->bInheritPitch = false;
		SpringArmComponent->bInheritYaw = false;
		SpringArmComponent->bInheritRoll = false;

		SpringArmComponent->bDoCollisionTest = false;

		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
	}
	case EViewMode::TPSView:
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = true;
		bUseControllerRotationRoll = false;

		SpringArmComponent->TargetArmLength = 400.f;

		SpringArmComponent->bUsePawnControlRotation = true;

		SpringArmComponent->bInheritPitch = true;
		SpringArmComponent->bInheritYaw = true;
		SpringArmComponent->bInheritRoll = false;

		SpringArmComponent->bDoCollisionTest = false;

		SpringArmComponent->SetRelativeLocation(FVector(0.f, 50.f, 0.f)); //약간 옆에서 볼 수 있게 함

		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;

		break;

	case EViewMode::None:
	case EViewMode::End:
	default:
		break;
	}
}

void ASPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaSeconds, 35.f);
	CameraComponent->SetFieldOfView(CurrentFOV);

	if (IsValid(GetController()) == true)
	{
		/*	1인 플레이시 사용가능
		FRotator ControlRotation = GetController()->GetControlRotation();
		CurrentAimPitch = ControlRotation.Pitch;
		CurrentAimYaw = ControlRotation.Yaw;
		*/

		PreviousAimPitch = CurrentAimPitch;
		PreviousAimYaw = CurrentAimYaw;

		FRotator ControlRotation = GetController()->GetControlRotation();
		CurrentAimPitch = ControlRotation.Pitch;
		CurrentAimYaw = ControlRotation.Yaw;
	}

	if (PreviousAimPitch != CurrentAimPitch || PreviousAimYaw != CurrentAimYaw)
	{
		if (HasAuthority() == false)
		{
			UpdateAimValue_Server(CurrentAimPitch, CurrentAimYaw);
		}
	}

	if (PreviousForwardInputValue != ForwardInputValue || PreviousRightInputValue != RightInputValue)
	{
		if (HasAuthority() == false)
		{
			UpdateInputValue_Server(ForwardInputValue, RightInputValue);
		}
	}

	if (bIsNowRagdollBlending == true)
	{
		CurrentRagdollBlendWeight = FMath::FInterpTo(CurrentRagdollBlendWeight, TargetRagdollBlendWeight, DeltaSeconds, 10.f);

		FName PivotBoneName = FName(TEXT("spine_01"));
		GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(PivotBoneName, CurrentRagdollBlendWeight);

		if (CurrentRagdollBlendWeight - TargetRagdollBlendWeight < KINDA_SMALL_NUMBER)
		{
			GetMesh()->SetAllBodiesBelowSimulatePhysics(PivotBoneName, false);
			bIsNowRagdollBlending = false;
		}

		if (IsValid(GetStatComponent()) && GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)	//사망시 렉돌이 피격 렉돌과 겹침.
		{
			GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(FName(TEXT("root")), 1.f);	//모든 본에 렉돌 가중치
			GetMesh()->SetSimulatePhysics(true);	//전체 렉돌을 킴
			bIsNowRagdollBlending = false;
		}
	}

	return;
}

void ASPlayerCharacter::SetMeshMaterial(const EPlayerTeam& InPlayerTeam)
{
	uint8 TeamIdx = 0u;
	switch (InPlayerTeam)
	{
	case EPlayerTeam::Black:
		TeamIdx = 0u;
		break;
	case EPlayerTeam::White:
		TeamIdx = 1u;
		break;
	default:
		TeamIdx = 1u;
		break;
	}

	const USPlayerCharacterSettings* CDO = GetDefault<USPlayerCharacterSettings>();
	CurrentPlayerCharacterMeshMaterialPath = CDO->PlayerCharacterMeshMaterialPaths[TeamIdx];
	AssetStreamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		CurrentPlayerCharacterMeshMaterialPath,
		FStreamableDelegate::CreateLambda([this]() -> void
			{
				AssetStreamableHandle->ReleaseHandle();
				TSoftObjectPtr<UMaterial> LoadedMaterialInstanceAsset(CurrentPlayerCharacterMeshMaterialPath);
				if (LoadedMaterialInstanceAsset.IsValid() == true)
				{
					GetMesh()->SetMaterial(0, LoadedMaterialInstanceAsset.Get());
				}
			})
	);
}

float ASPlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	/*	1인 플레이시 사용가능
	if (IsValid(GetStatComponent()) == false)
	{
		return FinalDamage;
	}

	if (GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
	{
		GetMesh()->SetSimulatePhysics(true);
	}
	else
	{
		FName PivotBoneName = FName(TEXT("spine_01"));
		GetMesh()->SetAllBodiesBelowSimulatePhysics(PivotBoneName, true);
		//float BlendWeight = 1.f;	//렉돌 포즈에 치우쳐지게끔 하는 가중치.
		//GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(PivotBoneName, BlendWeight);
		TargetRagdollBlendWeight = 1.f;

		HittedRagdollRestoreTimerDelegate.BindUObject(this, &ThisClass::OnHittedRagdollRestoreTimerElapsed);
		GetWorld()->GetTimerManager().SetTimer(HittedRagdollRestoreTimer, HittedRagdollRestoreTimerDelegate, 1.f, false);
	}
	*/
	PlayRagdoll_NetMulticast();

	return FinalDamage;
}

void ASPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ForwardInputValue);
	DOREPLIFETIME(ThisClass, RightInputValue);
	DOREPLIFETIME(ThisClass, CurrentAimPitch);
	DOREPLIFETIME(ThisClass, CurrentAimYaw);
	DOREPLIFETIME(ThisClass, Magazine);
	DOREPLIFETIME(ThisClass, CurrentWeapon);
	DOREPLIFETIME(ThisClass, CurrentAmmo);

}

void ASPlayerCharacter::SetCurrentWeapon_Server_Implementation(const FString& NewWeapon)
{
	if (NewWeapon == "Rifle")
	{
		// 이전 무기의 탄약을 저장
		if (CurrentWeapon == "Shotgun")
		{
			ShotgunAmmo = CurrentAmmo;
		}
		else if (CurrentWeapon == "GrenadeLauncher")
		{
			GrenadeLauncherAmmo = CurrentAmmo;
		}

		// 라이플로 전환
		CurrentWeapon = "Rifle";
		CurrentAmmo = RifleAmmo;
		Magazine = RifleMagazine;
		SpawnWeaponInstance1_Server();

	}
	else if (NewWeapon == "Shotgun")
	{
		// 샷건으로 전환
		CurrentWeapon = "Shotgun";
		CurrentAmmo = ShotgunAmmo;
		Magazine = ShotgunMagazine;
		SpawnWeaponInstance2_Server();

	}
	else if (NewWeapon == "GrenadeLauncher")
	{
		// 그레네이드 런처로 전환
		CurrentWeapon = "GrenadeLauncher";
		CurrentAmmo = GrenadeLauncherAmmo;
		Magazine = GrenadeLauncherMagazine;
		SpawnWeaponInstance3_Server();

	}

}

void ASPlayerCharacter::ItemUse_Server_Implementation()
{
	StatComponent->SetCurrentHP(StatComponent->GetCurrentHP() + 20.f);
}

void ASPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (IsValid(EnhancedInputComponent) == true)
	{
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Move, ETriggerEvent::Triggered, this, &ThisClass::InputMove);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Look, ETriggerEvent::Triggered, this, &ThisClass::InputLook);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->ChangeView, ETriggerEvent::Started, this, &ThisClass::InputChangeView);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->QuickSlot01, ETriggerEvent::Started, this, &ThisClass::InputQuickSlot01);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->QuickSlot02, ETriggerEvent::Started, this, &ThisClass::InputQuickSlot02);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->QuickSlot03, ETriggerEvent::Started, this, &ThisClass::InputQuickSlot03);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->GrenadeLaunchar, ETriggerEvent::Started, this, &ThisClass::GrenadeLauncher);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Attack, ETriggerEvent::Started, this, &ThisClass::InputAttack);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Menu, ETriggerEvent::Started, this, &ThisClass::InputMenu);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->IronSight, ETriggerEvent::Started, this, &ThisClass::StartIronSight);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->IronSight, ETriggerEvent::Completed, this, &ThisClass::EndIronSight);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Trigger, ETriggerEvent::Started, this, &ThisClass::ToggleTrigger);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Attack, ETriggerEvent::Started, this, &ThisClass::StartFire);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Attack, ETriggerEvent::Completed, this, &ThisClass::StopFire);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->LandMine, ETriggerEvent::Started, this, &ThisClass::SpawnLandMine);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->LookSpeedUp, ETriggerEvent::Started, this, &ThisClass::InputLookSpeedUp);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->LookSpeedDown, ETriggerEvent::Started, this, &ThisClass::InputLookSpeedDown);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Reload, ETriggerEvent::Started, this, &ThisClass::InputReload);
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfig->Pickup, ETriggerEvent::Started, this, &ThisClass::InputPickup);
	
	}
}

void ASPlayerCharacter::OnCheckReloadEnd()
{
	if (CurrentWeapon == "Rifle")
	{
		RifleAmmo = RifleMagazine;  // 라이플 탄약 갱신
	}
	else if (CurrentWeapon == "Shotgun")
	{
		ShotgunAmmo = ShotgunMagazine;  // 샷건 탄약 갱신
	}
	else if (CurrentWeapon == "GrenadeLauncher")
	{
		GrenadeLauncherAmmo = GrenadeLauncherMagazine;  // 그레네이드 런처 탄약 갱신
	}

	CurrentAmmo = (CurrentWeapon == "Rifle") ? RifleMagazine :
		(CurrentWeapon == "Shotgun") ? ShotgunMagazine :
		GrenadeLauncherMagazine;  // 현재 무기의 탄약 갱신

	IsReloading = false;  // 재장전 완료 상태로 설정

	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());

	// 재장전 끝날 때의 이벤트 연결 해제
	if (AnimInstance && AnimInstance->OnCheckHit.IsAlreadyBound(this, &ThisClass::OnCheckReloadEnd))
	{
		AnimInstance->OnCheckHit.RemoveDynamic(this, &ThisClass::OnCheckReloadEnd);
	}

	// 탄약 갱신을 서버와 클라이언트 간에 동기화
	SyncAmmo_Server(CurrentAmmo);
}

void ASPlayerCharacter::OnCheckSpawnWeapon()
{
	FName WeaponSocket(TEXT("WeaponSocket"));

	if (IsValid(WeaponInstance) == true)
	{
		WeaponInstance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocket);
	}

	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());

	if (true == AnimInstance->OnCheckSpawnWeapon.IsAlreadyBound(this, &ThisClass::OnCheckSpawnWeapon))
	{
		AnimInstance->OnCheckSpawnWeapon.RemoveDynamic(this, &ThisClass::OnCheckSpawnWeapon);
	}
}

void ASPlayerCharacter::InputMove(const FInputActionValue& InValue)
{
	if (GetCharacterMovement()->GetGroundMovementMode() == MOVE_None || StatComponent->GetCurrentHP() <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	FVector2D MovementVector = InValue.Get<FVector2D>();

	switch (CurrentViewMode)
	{
	case EViewMode::BackView:
	{ // Switch-Case 구문 내에서 Scope를 지정하면 해당 Scope 내에서 변수 선언이 가능해짐.
		const FRotator ControlRotation = GetController()->GetControlRotation();
		const FRotator ControlRotationYaw(0.f, ControlRotation.Yaw, 0.f);

		const FVector ForwardVector = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::X);
		const FVector RightVector = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardVector, MovementVector.X);
		AddMovementInput(RightVector, MovementVector.Y);

		break;
	}
	case EViewMode::QuarterView:
		DirectionToMove.X = MovementVector.X;
		DirectionToMove.Y = MovementVector.Y;

		break;
	case EViewMode::TPSView: 
		{
		const FRotator ControlRotation = GetController()->GetControlRotation();
		const FRotator ControlRotationYaw(0.f, ControlRotation.Yaw, 0.f);

		const FVector ForwardVector = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::X);
		const FVector RightVector = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardVector, MovementVector.X);
		AddMovementInput(RightVector, MovementVector.Y);

		ForwardInputValue = MovementVector.X;
		RightInputValue = MovementVector.Y;

		break;
		}
	case EViewMode::None:
	case EViewMode::End:
	default:
		AddMovementInput(GetActorForwardVector(), MovementVector.X);
		AddMovementInput(GetActorRightVector(), MovementVector.Y);
		break;
	}
}

void ASPlayerCharacter::InputLook(const FInputActionValue& InValue)
{
	if (GetCharacterMovement()->GetGroundMovementMode() == MOVE_None || StatComponent->GetCurrentHP() <= KINDA_SMALL_NUMBER)
	{
		return;
	}
	
	FVector2D LookVector = InValue.Get<FVector2D>();

	switch (CurrentViewMode)
	{
	case EViewMode::BackView:
		AddControllerYawInput(LookVector.X);
		AddControllerPitchInput(LookVector.Y);
		break;
	case EViewMode::QuarterView:
	case EViewMode::TPSView:
		AddControllerYawInput(LookVector.X * LookSpeed);
		AddControllerPitchInput(LookVector.Y * LookSpeed);
		break;
	case EViewMode::None:
	case EViewMode::End:
	default:
		break;
	}
}

void ASPlayerCharacter::InputChangeView(const FInputActionValue& InValue)
{
	switch (CurrentViewMode)
	{
	case EViewMode::BackView:
		/* Case 1. BackView에서 QuarterView로
		BackView는 컨트롤 로테이션이 스프링암 로테이션에 동기화 되고 있음.
		QuarterView는 컨트롤 로테이션에 폰 로테이션이 동기화 됨.
		따라서 시점 변경 전에 컨트롤 로테이션에 폰의 로테이션을 세팅해둬야 함.
		*/
		GetController()->SetControlRotation(GetActorRotation());
		DestArmLength = 800.f;
		DestArmRotation = FRotator(-45.f, 0.f, 0.f);
		SetViewMode(EViewMode::QuarterView);
		break;
	case EViewMode::QuarterView:
		/* Case 2. QuarterView에서 BackView로
		QuarterView는 컨트롤 로테이션에 폰 로테이션이 동기화 됨.
		BackView는 컨트롤 로테이션이 스프링암 로테이션에 동기화 되고 있음.
		따라서 시점 변경 전에 컨트롤 로테이션에 스프링암 로테이션을 세팅해둬야 함.
		*/
		GetController()->SetControlRotation(SpringArmComponent->GetRelativeRotation());
		DestArmLength = 400.f;
		DestArmRotation = FRotator::ZeroRotator;
		SetViewMode(EViewMode::BackView);
		break;
	case EViewMode::TPSView:
	case EViewMode::None:
	case EViewMode::End:
	default:
		break;
	}
}

void ASPlayerCharacter::InputQuickSlot01(const FInputActionValue& InValue)
{
	/*	1인 플레이일때 사용가능
	FName WeaponSocket(TEXT("WeaponSocket"));
	if (GetMesh()->DoesSocketExist(WeaponSocket) == true && IsValid(WeaponInstance) == false)
	{
		WeaponInstance = GetWorld()->SpawnActor<ASWeaponActor>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator);
		if (IsValid(WeaponInstance) == true)
		{
			WeaponInstance->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocket);
		}

		//AnimLayer
		TSubclassOf<UAnimInstance> RifleCharacterAnimLayer = WeaponInstance->GetArmedCharacterAnimLayer();
		if (IsValid(RifleCharacterAnimLayer) == true)
		{
			GetMesh()->LinkAnimClassLayers(RifleCharacterAnimLayer);
		}

		USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
		if (IsValid(AnimInstance) == true && IsValid(WeaponInstance->GetEquipAnimMontage()))
		{
			AnimInstance->Montage_Play(WeaponInstance->GetEquipAnimMontage());
		}
	}
	*/
	// 이전 무기 탄약 저장
	if (HasAuthority())
	{
		// 이전 무기의 탄약을 저장
		if (CurrentWeapon == "Shotgun")
		{
			ShotgunAmmo = CurrentAmmo;
		}
		else if (CurrentWeapon == "GrenadeLauncher")
		{
			GrenadeLauncherAmmo = CurrentAmmo;
		}

		// 라이플로 무기 전환
		CurrentWeapon = "Rifle";
		CurrentAmmo = RifleAmmo;
		Magazine = RifleMagazine;

		SpawnWeaponInstance1_Server();
	}
	else
	{
		// 클라이언트가 서버에 요청
		SetCurrentWeapon_Server("Rifle");
	}
}

void ASPlayerCharacter::InputQuickSlot02(const FInputActionValue& InValue)
{
	//이전 무기 탄약 저장
	if (HasAuthority())
	{
		// 이전 무기의 탄약을 저장
		if (CurrentWeapon == "Rifle")
		{
			RifleAmmo = CurrentAmmo;
		}
		else if (CurrentWeapon == "GrenadeLauncher")
		{
			GrenadeLauncherAmmo = CurrentAmmo;
		}

		// 라이플로 무기 전환
		CurrentWeapon = "Shotgun";
		CurrentAmmo = ShotgunAmmo;
		Magazine = ShotgunMagazine;

		SpawnWeaponInstance2_Server();
	}
	else
	{
		// 클라이언트가 서버에 요청
		SetCurrentWeapon_Server("Shotgun");
	}
}

void ASPlayerCharacter::InputQuickSlot03(const FInputActionValue& InValue)
{
	if (IsValid(WeaponInstance) == true)
	{
		DestroyWeaponInstance_Server();
	}
}

void ASPlayerCharacter::GrenadeLauncher(const FInputActionValue& InValue)
{
	//이전 무기 탄약 저장
	if (HasAuthority())
	{
		// 이전 무기의 탄약을 저장
		if (CurrentWeapon == "Rifle")
		{
			RifleAmmo = CurrentAmmo;
		}
		else if (CurrentWeapon == "Shotgun")
		{
			ShotgunAmmo = CurrentAmmo;
		}

		// 라이플로 무기 전환
		CurrentWeapon = "GrenadeLauncher";
		CurrentAmmo = GrenadeLauncherAmmo;
		Magazine = GrenadeLauncherMagazine;

		SpawnWeaponInstance3_Server();
	}
	else
	{
		// 클라이언트가 서버에 요청
		SetCurrentWeapon_Server("GrenadeLauncher");
	}
}

void ASPlayerCharacter::InputAttack(const FInputActionValue& InValue)
{
	if (GetCharacterMovement()->IsFalling() == true)
	{
		return;
	}

	if (IsReloading == true)
	{
		return;
	}
	/*
	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
	checkf(IsValid(AnimInstance) == true, TEXT("InValid AnimInstance"));

	if (IsValid(WeaponInstance) == true)
	{
		if (IsValid(WeaponInstance->GetMeleeAttackMontage()) == true)
		{
			if (CurrentComboCount == 0)
			{
				BeginAttack();
			}
			else
			{
				ensure(FMath::IsWithinInclusive<int32>(CurrentComboCount, 1, MaxComboCount));
				bIsAttackKeyPressed = true;
			}
		}
	}
	*/
	if (bIsTriggerToggle == false)
	{
		TryFire();
	}
}

void ASPlayerCharacter::InputMenu(const FInputActionValue& InValue)
{
	ASPlayerController* PlayerController = GetController<ASPlayerController>();
	if (IsValid(PlayerController) == true)
	{
		PlayerController->ToggleInGameMenu();
	}
}

void ASPlayerCharacter::InputLookSpeedUp(const FInputActionValue& InValue)
{
	if (LookSpeed < 2) 
	{
		LookSpeed = LookSpeed + 0.1f;
	}
}

void ASPlayerCharacter::InputLookSpeedDown(const FInputActionValue& InValue)
{
	if (LookSpeed > KINDA_SMALL_NUMBER)
	{
		LookSpeed = LookSpeed - 0.1f;
	}
}

void ASPlayerCharacter::InputReload(const FInputActionValue& InValue)
{
	if (IsReloading == false)
	{
		Reload();
	}
}

void ASPlayerCharacter::InputPickup(const FInputActionValue& InValue)
{
	FindOverlappingItems();
}

/*
void ASPlayerCharacter::TryFire()
{
	APlayerController* PlayerController = GetController<APlayerController>();

	if (!IsValid(PlayerController) || !IsValid(WeaponInstance))
	{
		return;		//PlayerController or WeaponInstance가 없으면 중단.
	}

	if (CurrentAmmo > 0 && CurrentWeapon == "Rifle")
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CurrentWeapon = %s"), *CurrentWeapon));
		//APlayerController* PlayerController = GetController<APlayerController>();
		if (IsValid(PlayerController) == true && IsValid(WeaponInstance) == true)
		{
#pragma region CalculateTargetTransform
			float FocalDistance = 400.f;
			FVector FocalLocation;
			FVector CameraLocation;
			FRotator CameraRotation;

			PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

			FVector AimDirectionFromCamera = CameraRotation.Vector().GetSafeNormal();
			FocalLocation = CameraLocation + (AimDirectionFromCamera * FocalDistance);	//character 위치

			FVector WeaponMuzzleLocation = WeaponInstance->GetMesh()->GetSocketLocation(TEXT("MuzzleFlash"));
			FVector FinalFocalLocation = FocalLocation + (((WeaponMuzzleLocation - FocalLocation) | AimDirectionFromCamera) * AimDirectionFromCamera);	//Muzzle위치까지 생각한 최종 출발 위치

			FTransform TargetTransform = FTransform(CameraRotation, FinalFocalLocation);

			if (ShowAttackDebug == 1)
			{
				DrawDebugSphere(GetWorld(), WeaponMuzzleLocation, 2.f, 16, FColor::Red, false, 60.f);

				DrawDebugSphere(GetWorld(), CameraLocation, 2.f, 16, FColor::Yellow, false, 60.f);

				DrawDebugSphere(GetWorld(), FinalFocalLocation, 2.f, 16, FColor::Magenta, false, 60.f);

				// (WeaponLoc - FocalLoc)
				DrawDebugLine(GetWorld(), FocalLocation, WeaponMuzzleLocation, FColor::Yellow, false, 60.f, 0, 2.f);

				// AimDir
				DrawDebugLine(GetWorld(), CameraLocation, FinalFocalLocation, FColor::Blue, false, 60.f, 0, 2.f);

				// Project Direction Line
				DrawDebugLine(GetWorld(), WeaponMuzzleLocation, FinalFocalLocation, FColor::Red, false, 60.f, 0, 2.f);
			}
#pragma endregion

#pragma region PerformLineTracing

			FVector BulletDirection = TargetTransform.GetUnitAxis(EAxis::X);
			FVector StartLocation = TargetTransform.GetLocation();
			FVector EndLocation = StartLocation + BulletDirection * WeaponInstance->GetMaxRange();

			FHitResult HitResult;
			FCollisionQueryParams TraceParams(NAME_None, false, this);
			TraceParams.AddIgnoredActor(WeaponInstance);

			bool IsCollided = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel2, TraceParams);
			if (IsCollided == true)
			{
				HitResult.TraceStart = StartLocation;
				HitResult.TraceEnd = EndLocation;
			}

			if (ShowAttackDebug == 2)
			{
				if (IsCollided == true)
				{
					DrawDebugSphere(GetWorld(), StartLocation, 2.f, 16, FColor::Red, false, 60.f);

					DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 2.f, 16, FColor::Green, false, 60.f);

					DrawDebugLine(GetWorld(), StartLocation, HitResult.ImpactPoint, FColor::Blue, false, 60.f, 0, 2.f);
				}
				else
				{
					DrawDebugSphere(GetWorld(), StartLocation, 2.f, 16, FColor::Red, false, 60.f);

					DrawDebugSphere(GetWorld(), EndLocation, 2.f, 16, FColor::Green, false, 60.f);

					DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, false, 60.f, 0, 2.f);
				}
			}
#pragma endregion

			ApplyDamageAndDrawLine_Server(HitResult);

			// Owning Client 에서 몽타주 재생
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (IsValid(AnimInstance) == true && IsValid(WeaponInstance) == true)
			{
				if (AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()) == false)
				{
					AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
				}
			}

			// Other Client 에서도 재생하기 위해 Server RPC 호출.
			PlayAttackMontage_Server();

		}

		if (IsValid(FireShake) == true && GetOwner() == UGameplayStatics::GetPlayerController(this, 0))
		{
			PlayerController->ClientStartCameraShake(FireShake);
		}
		--CurrentAmmo;
		RifleAmmo = CurrentAmmo;
	}
	if (CurrentAmmo > 0 && CurrentWeapon == "Shotgun")
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CurrentWeapon = %s"), *CurrentWeapon));
		//APlayerController* PlayerController = GetController<APlayerController>();
		if (IsValid(PlayerController) && IsValid(WeaponInstance))
		{
#pragma region CalculateTargetTransform
			float FocalDistance = 400.f;
			FVector FocalLocation;
			FVector CameraLocation;
			FRotator CameraRotation;

			PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

			FVector AimDirectionFromCamera = CameraRotation.Vector().GetSafeNormal();
			FocalLocation = CameraLocation + (AimDirectionFromCamera * FocalDistance);

			FVector WeaponMuzzleLocation = WeaponInstance->GetMesh()->GetSocketLocation(TEXT("MuzzleFlash"));
			FVector FinalFocalLocation = FocalLocation + (((WeaponMuzzleLocation - FocalLocation) | AimDirectionFromCamera) * AimDirectionFromCamera);

			FTransform TargetTransform = FTransform(CameraRotation, FinalFocalLocation);

			if (ShowAttackDebug == 1)
			{
				DrawDebugSphere(GetWorld(), WeaponMuzzleLocation, 2.f, 16, FColor::Red, false, 60.f);
				DrawDebugSphere(GetWorld(), CameraLocation, 2.f, 16, FColor::Yellow, false, 60.f);
				DrawDebugSphere(GetWorld(), FinalFocalLocation, 2.f, 16, FColor::Magenta, false, 60.f);

				DrawDebugLine(GetWorld(), FocalLocation, WeaponMuzzleLocation, FColor::Yellow, false, 60.f, 0, 2.f);
				DrawDebugLine(GetWorld(), CameraLocation, FinalFocalLocation, FColor::Blue, false, 60.f, 0, 2.f);
				DrawDebugLine(GetWorld(), WeaponMuzzleLocation, FinalFocalLocation, FColor::Red, false, 60.f, 0, 2.f);
			}
#pragma endregion

#pragma region PerformShotgunLineTracing

			int32 PelletCount = 10; // 샷건 펠릿 수
			float SpreadAngle = 10.0f; // 각도 스프레드

			for (int32 i = 0; i < PelletCount; ++i)
			{
				FRotator RandomSpread = CameraRotation;
				RandomSpread.Yaw += FMath::FRandRange(-SpreadAngle, SpreadAngle);
				RandomSpread.Pitch += FMath::FRandRange(-SpreadAngle, SpreadAngle);

				FVector PelletDirection = RandomSpread.Vector();
				FVector StartLocation = WeaponMuzzleLocation;
				FVector EndLocation = StartLocation + PelletDirection * WeaponInstance->GetMaxRange();

				FHitResult HitResult;
				FCollisionQueryParams TraceParams(NAME_None, false, this);
				TraceParams.AddIgnoredActor(WeaponInstance);

				bool IsCollided = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel2, TraceParams);
				if (!IsCollided)
				{
					HitResult.TraceStart = StartLocation;
					HitResult.TraceEnd = EndLocation;
				}

				if (ShowAttackDebug == 2)
				{
					if (IsCollided)
					{
						DrawDebugSphere(GetWorld(), StartLocation, 2.f, 16, FColor::Red, false, 60.f);
						DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 2.f, 16, FColor::Green, false, 60.f);
						DrawDebugLine(GetWorld(), StartLocation, HitResult.ImpactPoint, FColor::Blue, false, 60.f, 0, 2.f);
					}
					else
					{
						DrawDebugSphere(GetWorld(), StartLocation, 2.f, 16, FColor::Red, false, 60.f);
						DrawDebugSphere(GetWorld(), EndLocation, 2.f, 16, FColor::Green, false, 60.f);
						DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Blue, false, 60.f, 0, 2.f);
					}
				}

				ApplyDamageAndDrawLine_Server(HitResult);
			}
#pragma endregion

			// Owning Client 에서 몽타주 재생
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (IsValid(AnimInstance) && IsValid(WeaponInstance))
			{
				if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()))
				{
					AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
				}
			}

			// Other Client 에서도 재생하기 위해 Server RPC 호출.
			PlayAttackMontage_Server();
		}
		if (IsValid(FireShake) == true && GetOwner() == UGameplayStatics::GetPlayerController(this, 0))
		{
			PlayerController->ClientStartCameraShake(FireShake);
		}
		--CurrentAmmo;
		ShotgunAmmo = CurrentAmmo;
	}

	//GrenadeLauncher
	if (HasAuthority())  // 서버에서만 실행
	{
		if (CurrentAmmo > 0 && CurrentWeapon == "GrenadeLauncher")
		{
			//APlayerController* PlayerController = GetController<APlayerController>();
			if (IsValid(PlayerController) && IsValid(WeaponInstance))
			{
				FVector WeaponMuzzleLocation = WeaponInstance->GetMesh()->GetSocketLocation(TEXT("MuzzleFlash"));
				FVector CameraLocation;
				FRotator CameraRotation;

				PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

				FVector AimDirectionFromCamera = CameraRotation.Vector().GetSafeNormal();

				// 그레네이드를 서버에서 스폰
				if (GrenadeClass)
				{
					UWorld* World = GetWorld();
					if (World)
					{
						AAGrenade* SpawnedGrenade = World->SpawnActor<AAGrenade>(
							GrenadeClass,
							WeaponMuzzleLocation,    // 스폰 위치 (무기 입구 위치)
							CameraRotation           // 스폰 방향 (카메라의 방향)
						);

						if (SpawnedGrenade)
						{
							FVector LaunchDirection = AimDirectionFromCamera;
							SpawnedGrenade->FireInDirection(LaunchDirection); // 그레네이드를 발사
							SpawnedGrenade->SetOwner(this);                    // 소유자 설정
						}
					}
				}

				// 탄약 감소는 서버에서만 처리
				--CurrentAmmo;
				GrenadeLauncherAmmo = CurrentAmmo;

				// Owning Client에서 몽타주 재생
				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				if (IsValid(AnimInstance) && IsValid(WeaponInstance))
				{
					if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()))
					{
						AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
					}
				}

				// Other Client에서도 재생하기 위해 Server RPC 호출
				PlayAttackMontage_Server();

				// 재장전 호출
				if (CurrentAmmo <= 0)
				{
					Reload();
				}
			}
		}
	}
	else
	{
		// 클라이언트에서 서버로 요청
		TryFire_Server();
	}

	// 클라이언트에서 카메라 쉐이크 처리
	if (IsValid(FireShake) && GetOwner() == UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->ClientStartCameraShake(FireShake);
	}
}
*/

void ASPlayerCharacter::TryFire()
{
	APlayerController* PlayerController = GetController<APlayerController>();

	if (!IsValid(PlayerController) || !IsValid(WeaponInstance))
	{
		return;  // PlayerController나 WeaponInstance가 유효하지 않으면 실행 중단
	}

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CurrentWeapon = %s"), *CurrentWeapon));

	if (HasAuthority())  // 서버에서만 실행
	{
		if (CurrentAmmo > 0)
		{
			// 무기에 따라 발사 처리
			if (CurrentWeapon == "Rifle")
			{
				PerformRifleFire(PlayerController);
			}
			else if (CurrentWeapon == "Shotgun")
			{
				PerformShotgunFire(PlayerController);
			}
			else if (CurrentWeapon == "GrenadeLauncher")
			{
				PerformGrenadeLauncherFire(PlayerController);
			}

			// 탄약 감소
			--CurrentAmmo;

			// 각 무기별로 탄약 값 저장
			if (CurrentWeapon == "Rifle")
			{
				RifleAmmo = CurrentAmmo;  // 라이플 탄약을 저장
			}
			else if (CurrentWeapon == "Shotgun")
			{
				ShotgunAmmo = CurrentAmmo;  // 샷건 탄약을 저장
			}
			else if (CurrentWeapon == "GrenadeLauncher")
			{
				GrenadeLauncherAmmo = CurrentAmmo;  // 그레네이드 런처 탄약을 저장
			}
		}
		else
		{
			UKismetSystemLibrary::PrintString(this, TEXT("No Ammo Left! Reloading..."));
		}
	}
	else
	{
		if (CurrentAmmo > 0) {
			// 클라이언트에서 서버로 발사 요청
			TryFire_Server();
		}
		else
		{
			Reload();  // 탄약이 없을 때 재장전 호출
		}
	}

	// 클라이언트에서 카메라 쉐이크 처리
	if (IsValid(FireShake) && GetOwner() == UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->ClientStartCameraShake(FireShake);
	}
}

void ASPlayerCharacter::TryFire_Server_Implementation()
{
	TryFire();  // 서버에서 발사 처리
}

void ASPlayerCharacter::PerformRifleFire(APlayerController* PlayerController)
{
#pragma region Rifle Fire Logic
	// 타겟 위치 계산
	FVector WeaponMuzzleLocation = WeaponInstance->GetMesh()->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector AimDirectionFromCamera = CameraRotation.Vector().GetSafeNormal();
	FVector FocalLocation = CameraLocation + (AimDirectionFromCamera * 400.f);
	FVector FinalFocalLocation = FocalLocation + (((WeaponMuzzleLocation - FocalLocation) | AimDirectionFromCamera) * AimDirectionFromCamera);
	FTransform TargetTransform = FTransform(CameraRotation, FinalFocalLocation);

	// 라인 트레이싱
	FVector BulletDirection = TargetTransform.GetUnitAxis(EAxis::X);
	FVector StartLocation = TargetTransform.GetLocation();
	FVector EndLocation = StartLocation + BulletDirection * WeaponInstance->GetMaxRange();

	FHitResult HitResult;
	FCollisionQueryParams TraceParams(NAME_None, false, this);
	TraceParams.AddIgnoredActor(WeaponInstance);

	if (HitResult.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitResult.GetActor()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Actor hit"));
	}

	bool IsCollided = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, TraceParams);

	ApplyDamageAndDrawLine_Server(HitResult);

	// Owning Client에서 애니메이션 재생
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) && IsValid(WeaponInstance))
	{
		if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()))
		{
			AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
		}
	}

	// Other Client에서도 애니메이션 재생을 위한 Server RPC 호출
	PlayAttackMontage_Server();
#pragma endregion
}

void ASPlayerCharacter::PerformShotgunFire(APlayerController* PlayerController)
{
#pragma region Shotgun Fire Logic
	FVector WeaponMuzzleLocation = WeaponInstance->GetMesh()->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector AimDirectionFromCamera = CameraRotation.Vector().GetSafeNormal();
	FTransform TargetTransform = FTransform(CameraRotation, AimDirectionFromCamera);

	// 샷건 발사 - 펠릿 수, 스프레드 설정
	int32 PelletCount = 10;
	float SpreadAngle = 10.0f;

	for (int32 i = 0; i < PelletCount; ++i)
	{
		FRotator RandomSpread = CameraRotation;
		RandomSpread.Yaw += FMath::FRandRange(-SpreadAngle, SpreadAngle);
		RandomSpread.Pitch += FMath::FRandRange(-SpreadAngle, SpreadAngle);

		FVector PelletDirection = RandomSpread.Vector();
		FVector StartLocation = WeaponMuzzleLocation;
		FVector EndLocation = StartLocation + PelletDirection * WeaponInstance->GetMaxRange();

		FHitResult HitResult;
		FCollisionQueryParams TraceParams(NAME_None, false, this);
		TraceParams.AddIgnoredActor(WeaponInstance);

		if (HitResult.GetActor())
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitResult.GetActor()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No Actor hit"));
		}

		bool IsCollided = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel2, TraceParams);
		ApplyDamageAndDrawLine_Server(HitResult);
	}

	// Owning Client에서 애니메이션 재생
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) && IsValid(WeaponInstance))
	{
		if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()))
		{
			AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
		}
	}

	// Other Client에서도 애니메이션 재생을 위한 Server RPC 호출
	PlayAttackMontage_Server();
#pragma endregion
}

void ASPlayerCharacter::PerformGrenadeLauncherFire(APlayerController* PlayerController)
{
	if (HasAuthority())  // 서버에서만 실행
	{
		// 캐릭터 위치 확인
		FVector CharacterLocation = GetActorLocation();
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Character Location: %s"), *CharacterLocation.ToString()));
		FVector WeaponMuzzleLocation;


		// WeaponInstance 확인
		if (!IsValid(WeaponInstance))
		{
			UKismetSystemLibrary::PrintString(this, TEXT("WeaponInstance is invalid!"));
			return;
		}
		else
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("WeaponInstance is valid")));

			// 무기 메쉬와 소켓 위치 확인
			WeaponMuzzleLocation = WeaponInstance->GetMesh()->GetSocketLocation(TEXT("MuzzleFlash"));
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Weapon Muzzle Location: %s"), *WeaponMuzzleLocation.ToString()));

			WeaponMuzzleLocation.Z += 100.f;

			// 추가로 WeaponInstance가 올바르게 장착되었는지 확인
			FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
			WeaponInstance->AttachToComponent(GetMesh(), AttachRules, TEXT("WeaponSocket"));
		}

		// 카메라 위치 및 발사 방향 계산
		FVector CameraLocation;
		FRotator CameraRotation;
		PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

		FVector AimDirectionFromCamera = CameraRotation.Vector().GetSafeNormal();

		// GetWorld()가 유효한지 확인
		if (!GetWorld())
		{
			UKismetSystemLibrary::PrintString(this, TEXT("GetWorld() is invalid!"));
			return;
		}


		// 그레네이드 스폰
		if (GrenadeClass)
		{
			UWorld* World = GetWorld();
			AAGrenade* SpawnedGrenade = World->SpawnActor<AAGrenade>(
				GrenadeClass,
				WeaponMuzzleLocation,  // 총구 위치에서 스폰
				CameraRotation         // 카메라 회전 방향으로 스폰
			);

			if (SpawnedGrenade)
			{
				//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade Spawned on Server")));
				SpawnedGrenade->SetReplicates(true);
				SpawnedGrenade->SetReplicateMovement(true);
				SpawnedGrenade->FireInDirection(AimDirectionFromCamera);  // 발사 방향 설정
				SpawnedGrenade->SetOwner(this);
				FVector GrenadeLocation = SpawnedGrenade->GetActorLocation();
				//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade Location: %s"), *GrenadeLocation.ToString()));


				FTimerHandle TimerHandle;
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, SpawnedGrenade]() {
					SpawnGrenade_NetMulticast(SpawnedGrenade);
					}, 0.2f, false);  // 0.2초 후에 NetMulticast 호출
			}
		}
	}

	// Owning Client에서 애니메이션 재생
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) && IsValid(WeaponInstance))
	{
		if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()))
		{
			AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade FireAnim")));

		}
	}

	// Other Client에서도 애니메이션 재생을 위한 Server RPC 호출
	PlayAttackMontage_Server();
#pragma endregion
}

void ASPlayerCharacter::SpawnGrenade_NetMulticast_Implementation(AAGrenade* SpawnedGrenade)
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("NetMulticast function executed")));

	if (SpawnedGrenade)
	{
		FVector ServerLocation = SpawnedGrenade->GetActorLocation();  // 서버에서의 그레네이드 위치

		// **클라이언트에서 위치 강제 동기화**
		if (!HasAuthority())  // 클라이언트에서만 실행
		{
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade Location on Server: %s"), *ServerLocation.ToString()));

			// 서버 위치로 클라이언트에서 강제 동기화
			SpawnedGrenade->SetActorLocation(ServerLocation);

			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade Location on Client after sync: %s"), *SpawnedGrenade->GetActorLocation().ToString()));
		}

		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade is valid on Client")));
	}
}

void ASPlayerCharacter::Reload()
{
	// 무기 인스턴스가 유효하고 재장전 중이 아닌 경우에만 실행
	if (IsValid(WeaponInstance) && !IsReloading)
	{
		// 각 무기별로 재장전이 필요한 경우 처리
		if ((CurrentWeapon == "Rifle" && CurrentAmmo < RifleMagazine) ||
			(CurrentWeapon == "Shotgun" && CurrentAmmo < ShotgunMagazine) ||
			(CurrentWeapon == "GrenadeLauncher" && CurrentAmmo < GrenadeLauncherMagazine))
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (IsValid(AnimInstance) && IsValid(WeaponInstance))
			{
				// 무기 종류에 맞는 재장전 애니메이션 실행
				if (CurrentWeapon == "Rifle")
				{
					if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleReloadAnimMontage()))
					{
						AnimInstance->Montage_Play(WeaponInstance->GetRifleReloadAnimMontage());
					}
				}
				else if (CurrentWeapon == "Shotgun")
				{
					if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleReloadAnimMontage()))
					{
						AnimInstance->Montage_Play(WeaponInstance->GetRifleReloadAnimMontage());
					}
				}
				else if (CurrentWeapon == "GrenadeLauncher")
				{
					if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleReloadAnimMontage()))
					{
						AnimInstance->Montage_Play(WeaponInstance->GetRifleReloadAnimMontage());
					}
				}
			}

			// 서버에서 재장전 몽타주 실행 요청
			PlayReloadMontage_Server();

			// 재장전 중 상태 설정
			IsReloading = true;
		}
	}
}

void ASPlayerCharacter::StartIronSight(const FInputActionValue& InValue)
{
	TargetFOV = 45.f;
}

void ASPlayerCharacter::EndIronSight(const FInputActionValue& InValue)
{
	TargetFOV = 70.f;
}

void ASPlayerCharacter::ToggleTrigger(const FInputActionValue& InValue)
{
	bIsTriggerToggle = !bIsTriggerToggle;
}

void ASPlayerCharacter::StartFire(const FInputActionValue& InValue)
{
	if (bIsTriggerToggle == true)
	{
		GetWorldTimerManager().SetTimer(BetweenShotsTimer, this, &ThisClass::TryFire, TimeBetweenFire, true);
	}
}

void ASPlayerCharacter::StopFire(const FInputActionValue& InValue)
{
	GetWorldTimerManager().ClearTimer(BetweenShotsTimer);
}

void ASPlayerCharacter::SpawnLandMine(const FInputActionValue& InValue)
{
	SpawnLandMine_Server();
}

bool ASPlayerCharacter::SpawnLandMine_Server_Validate()	//검증 함수
{
	return true;
}

void ASPlayerCharacter::SpawnLandMine_Server_Implementation()	//구현 함수
{
	if (IsValid(LandMineClass) == true)
	{
		FVector SpawnedLocation = ((GetActorLocation() + GetActorForwardVector() * 300.f) - FVector(0.f, 0.f, 90.f));
		ASLandMine* SpawnedLandMine = GetWorld()->SpawnActor<ASLandMine>(LandMineClass, SpawnedLocation, FRotator::ZeroRotator);
		SpawnedLandMine->SetOwner(GetController());
	}
}

void ASPlayerCharacter::OnHittedRagdollRestoreTimerElapsed()
{
	FName PivotBoneName = FName(TEXT("spine_01"));
	TargetRagdollBlendWeight = 0.f;
	CurrentRagdollBlendWeight = 1.f;
	bIsNowRagdollBlending = true;
}

void ASPlayerCharacter::SpawnWeaponInstance1_Server_Implementation()
{
	FName WeaponSocket(TEXT("WeaponSocket"));
	if (IsValid(WeaponInstance) == true)
	{
		WeaponInstance->Destroy();
		WeaponInstance = nullptr;
	}
	if (GetMesh()->DoesSocketExist(WeaponSocket) == true && IsValid(WeaponInstance) == false)
	{
		if (IsValid(WeaponClass1)) 
		{
			WeaponInstance = GetWorld()->SpawnActor<ASWeaponActor>(WeaponClass1, FVector::ZeroVector, FRotator::ZeroRotator);
		}
	}
} 

void ASPlayerCharacter::SpawnWeaponInstance2_Server_Implementation()
{
	FName WeaponSocket(TEXT("WeaponSocket"));
	if (IsValid(WeaponInstance) == true)
	{
		WeaponInstance->Destroy();
		WeaponInstance = nullptr;
	}
	if (GetMesh()->DoesSocketExist(WeaponSocket) == true && IsValid(WeaponInstance) == false)
	{
		if (IsValid(WeaponClass2)) 
		{
			WeaponInstance = GetWorld()->SpawnActor<ASWeaponActor>(WeaponClass2, FVector::ZeroVector, FRotator::ZeroRotator);
		}
	}
	
}

void ASPlayerCharacter::SpawnWeaponInstance3_Server_Implementation()
{
	FName WeaponSocket(TEXT("WeaponSocket"));
	if (IsValid(WeaponInstance) == true)
	{
		WeaponInstance->Destroy();
		WeaponInstance = nullptr;
	}
	if (GetMesh()->DoesSocketExist(WeaponSocket) == true && IsValid(WeaponInstance) == false)
	{
		if (IsValid(WeaponClass3))
		{
			WeaponInstance = GetWorld()->SpawnActor<ASWeaponActor>(WeaponClass3, FVector::ZeroVector, FRotator::ZeroRotator);
		}
	}
}


void ASPlayerCharacter::DestroyWeaponInstance_Server_Implementation()
{
	WeaponInstance->Destroy();
	WeaponInstance = nullptr;
}

void ASPlayerCharacter::OnRep_WeaponInstance()
{

	if (IsValid(WeaponInstance) == true)
	{
		TSubclassOf<UAnimInstance> RifleCharacterAnimLayer = WeaponInstance->GetArmedCharacterAnimLayer();
		if (IsValid(RifleCharacterAnimLayer) == true)
		{
			GetMesh()->LinkAnimClassLayers(RifleCharacterAnimLayer);
		}

		USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
		if (IsValid(AnimInstance) == true && IsValid(WeaponInstance->GetEquipAnimMontage()))
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("EquipAnimMontage Play!")));
			AnimInstance->Montage_Play(WeaponInstance->GetEquipAnimMontage());
		}

		UnarmedCharacterAnimLayer = WeaponInstance->GetUnarmedCharacterAnimLayer();
		UnequipAnimMontage = WeaponInstance->GetUnequipAnimMontage();
	}
	else
	{
		if (IsValid(UnarmedCharacterAnimLayer) == true)
		{
			GetMesh()->LinkAnimClassLayers(UnarmedCharacterAnimLayer);
		}

		USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());
		if (IsValid(UnequipAnimMontage) == true)
		{
			AnimInstance->Montage_Play(UnequipAnimMontage);
		}
	}
}

void ASPlayerCharacter::ApplyDamageAndDrawLine_Server_Implementation(FHitResult HitResult)
{

	if (HitResult.GetActor() != nullptr)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Actor: %s"), *HitResult.GetActor()->GetName()));
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, TEXT("No Actor hit"));
		return;  // HitResult가 없을 경우 함수 종료
	}

	ASCharacter* HittedCharacter = Cast<ASCharacter>(HitResult.GetActor());
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CurrentWeapon is %s!"), *CurrentWeapon));

	if (IsValid(HittedCharacter) == true)
	{
		FDamageEvent DamageEvent;

		FString BoneNameString = HitResult.BoneName.ToString();

		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CurrentWeapon is %s!!!"), *CurrentWeapon));

		if (CurrentWeapon == "Rifle") 
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("RifleApplyDamage.")));

			if (BoneNameString.Equals(FString(TEXT("HEAD")), ESearchCase::IgnoreCase) == true)
			{
				HittedCharacter->TakeDamage(100.f, DamageEvent, GetController(), this);
			}
			else
			{
				HittedCharacter->TakeDamage(10.f, DamageEvent, GetController(), this);
			}
			
		}
		if (CurrentWeapon == "Shotgun")
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ShotgunApplyDamage.")));

			if (BoneNameString.Equals(FString(TEXT("HEAD")), ESearchCase::IgnoreCase) == true)
			{
				HittedCharacter->TakeDamage(25.f, DamageEvent, GetController(), this);
			}
			else
			{
				HittedCharacter->TakeDamage(5.f, DamageEvent, GetController(), this);
			}
		}
		if (CurrentWeapon == "Grenade")
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("GrenadeApplyDamage.")));

			// Grenade 폭발 위치와 타겟의 위치 사이 거리 계산
			FVector ExplosionLocation = GetActorLocation(); // 그레네이드가 위치한 곳을 사용
			float Distance = FVector::Dist(ExplosionLocation, HitResult.GetActor()->GetActorLocation());

			// 거리별로 데미지 설정
			float MaxDamage = 100.f;
			float MinDamage = 20.f;
			float MaxDistance = 500.f; // 최대 데미지가 적용되는 거리
			float MinDistance = 1000.f; // 최소 데미지가 적용되는 거리

			float DamageToApply = FMath::GetMappedRangeValueClamped(FVector2D(MaxDistance, MinDistance), FVector2D(MaxDamage, MinDamage), Distance);

			// 계산된 데미지 적용
			HittedCharacter->TakeDamage(DamageToApply, DamageEvent, GetController(), this);
		}
	}

	DrawLine_NetMulticast(HitResult.TraceStart, HitResult.TraceEnd);
}

void ASPlayerCharacter::DrawLine_NetMulticast_Implementation(const FVector& InDrawStart, const FVector& InDrawEnd)
{
	if (HasAuthority() == false)
	{
		if (CurrentWeapon == "Rifle")
		{
			DrawDebugLine(GetWorld(), WeaponInstance->GetMesh()->GetSocketLocation(TEXT("MuzzleFlash")), InDrawEnd, FColor(255, 255, 255, 64), false, 0.1f, 0U, 0.5f);
		}
	}
}

void ASPlayerCharacter::PlayAttackMontage_Server_Implementation()
{
	PlayAttackMontage_NetMulticast();
}

void ASPlayerCharacter::PlayAttackMontage_NetMulticast_Implementation()
{
	//if (HasAuthority() == false && GetOwner() != UGameplayStatics::GetPlayerController(this, 0))
	//{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (IsValid(AnimInstance) == true && IsValid(WeaponInstance) == true)
		{
			if (AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()) == false)
			{
				AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
			}
		}
	//}
}

void ASPlayerCharacter::UpdateInputValue_Server_Implementation(const float& InForwardInputValue, const float& InRightInputValue)
{
	ForwardInputValue = InForwardInputValue;
	RightInputValue = InRightInputValue;
}

void ASPlayerCharacter::UpdateAimValue_Server_Implementation(const float& InAimPitchValue, const float& InAimYawValue)
{
	CurrentAimPitch = InAimPitchValue;
	CurrentAimYaw = InAimYawValue;
}

void ASPlayerCharacter::PlayRagdoll_NetMulticast_Implementation()
{
	if (IsValid(GetStatComponent()) == false)
	{
		return;
	}

	if (GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)
	{
		GetMesh()->SetSimulatePhysics(true);
	}
	else
	{
		FName PivotBoneName = FName(TEXT("spine_01"));
		GetMesh()->SetAllBodiesBelowSimulatePhysics(PivotBoneName, true);
		//float BlendWeight = 1.f;	//렉돌 포즈에 치우쳐지게끔 하는 가중치.
		//GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(PivotBoneName, BlendWeight);
		TargetRagdollBlendWeight = 1.f;

		HittedRagdollRestoreTimerDelegate.BindUObject(this, &ThisClass::OnHittedRagdollRestoreTimerElapsed);
		GetWorld()->GetTimerManager().SetTimer(HittedRagdollRestoreTimer, HittedRagdollRestoreTimerDelegate, 0.1f, false);
	}
}

void ASPlayerCharacter::PlayReloadMontage_Server_Implementation()
{
	PlayReloadMontage_NetMulticast();
}

void ASPlayerCharacter::PlayReloadMontage_NetMulticast_Implementation()
{
	if (HasAuthority() == false && GetOwner() != UGameplayStatics::GetPlayerController(this, 0))
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (IsValid(AnimInstance) == true && IsValid(WeaponInstance) == true)
		{
			if (AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleReloadAnimMontage()) == false)
			{
				AnimInstance->Montage_Play(WeaponInstance->GetRifleReloadAnimMontage());
;			}
		}
	}
}

void ASPlayerCharacter::FindOverlappingItems()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, TSubclassOf<AActor>());
	
	for (AActor* Actor : OverlappingActors)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s"), *Actor->GetClass()->GetName()));
		if (Actor->GetClass()->GetName() == TEXT("BP_RifleItem_C"))
		{
			Actor->Destroy();
		}
		if (Actor->GetClass()->GetName() == TEXT("BP_Soda_C"))
		{
			Actor->Destroy();
		}
	}

}

void ASPlayerCharacter::SyncAmmo_Server_Implementation(int32 NewAmmo)
{
	// 서버에서 클라이언트로 탄약 값 동기화
	SyncAmmo(NewAmmo);
}

void ASPlayerCharacter::SyncAmmo_Implementation(int32 NewAmmo)
{
	CurrentAmmo = NewAmmo;  // 클라이언트에서 탄약 값 업데이트
}

