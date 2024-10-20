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
#include "SPlayerCharacterSettings.h"		//�񵿱� ����
#include "Engine/AssetManager.h"			//�񵿱� ����
#include "Engine/StreamableManager.h"		//�񵿱� ����
#include "Controller/SPlayerController.h"	//Menu����
#include "Engine/EngineTypes.h"				//��ݱ���
#include "Engine/DamageEvents.h"			//��ݱ���
#include "WorldStatic/SLandMine.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"			//��� ����ȭ
#include "Kismet/KismetSystemLibrary.h"		//��� ����ȭ
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
		// ControlRotation�� Pawn�� ȸ���� ����ȭ -> Pawn�� ȸ���� SpringArm�� ȸ�� ����ȭ. �̷� ���� SetRotation()�� ���ǹ�.

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

		SpringArmComponent->SetRelativeLocation(FVector(0.f, 50.f, 0.f)); //�ణ ������ �� �� �ְ� ��

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
		/*	1�� �÷��̽� ��밡��
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

		if (IsValid(GetStatComponent()) && GetStatComponent()->GetCurrentHP() < KINDA_SMALL_NUMBER)	//����� ������ �ǰ� ������ ��ħ.
		{
			GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(FName(TEXT("root")), 1.f);	//��� ���� ���� ����ġ
			GetMesh()->SetSimulatePhysics(true);	//��ü ������ Ŵ
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

	/*	1�� �÷��̽� ��밡��
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
		//float BlendWeight = 1.f;	//���� ��� ġ�������Բ� �ϴ� ����ġ.
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
		// ���� ������ ź���� ����
		if (CurrentWeapon == "Shotgun")
		{
			ShotgunAmmo = CurrentAmmo;
		}
		else if (CurrentWeapon == "GrenadeLauncher")
		{
			GrenadeLauncherAmmo = CurrentAmmo;
		}

		// �����÷� ��ȯ
		CurrentWeapon = "Rifle";
		CurrentAmmo = RifleAmmo;
		Magazine = RifleMagazine;
		SpawnWeaponInstance1_Server();

	}
	else if (NewWeapon == "Shotgun")
	{
		// �������� ��ȯ
		CurrentWeapon = "Shotgun";
		CurrentAmmo = ShotgunAmmo;
		Magazine = ShotgunMagazine;
		SpawnWeaponInstance2_Server();

	}
	else if (NewWeapon == "GrenadeLauncher")
	{
		// �׷����̵� ��ó�� ��ȯ
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
		RifleAmmo = RifleMagazine;  // ������ ź�� ����
	}
	else if (CurrentWeapon == "Shotgun")
	{
		ShotgunAmmo = ShotgunMagazine;  // ���� ź�� ����
	}
	else if (CurrentWeapon == "GrenadeLauncher")
	{
		GrenadeLauncherAmmo = GrenadeLauncherMagazine;  // �׷����̵� ��ó ź�� ����
	}

	CurrentAmmo = (CurrentWeapon == "Rifle") ? RifleMagazine :
		(CurrentWeapon == "Shotgun") ? ShotgunMagazine :
		GrenadeLauncherMagazine;  // ���� ������ ź�� ����

	IsReloading = false;  // ������ �Ϸ� ���·� ����

	USAnimInstance* AnimInstance = Cast<USAnimInstance>(GetMesh()->GetAnimInstance());

	// ������ ���� ���� �̺�Ʈ ���� ����
	if (AnimInstance && AnimInstance->OnCheckHit.IsAlreadyBound(this, &ThisClass::OnCheckReloadEnd))
	{
		AnimInstance->OnCheckHit.RemoveDynamic(this, &ThisClass::OnCheckReloadEnd);
	}

	// ź�� ������ ������ Ŭ���̾�Ʈ ���� ����ȭ
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
	{ // Switch-Case ���� ������ Scope�� �����ϸ� �ش� Scope ������ ���� ������ ��������.
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
		/* Case 1. BackView���� QuarterView��
		BackView�� ��Ʈ�� �����̼��� �������� �����̼ǿ� ����ȭ �ǰ� ����.
		QuarterView�� ��Ʈ�� �����̼ǿ� �� �����̼��� ����ȭ ��.
		���� ���� ���� ���� ��Ʈ�� �����̼ǿ� ���� �����̼��� �����ص־� ��.
		*/
		GetController()->SetControlRotation(GetActorRotation());
		DestArmLength = 800.f;
		DestArmRotation = FRotator(-45.f, 0.f, 0.f);
		SetViewMode(EViewMode::QuarterView);
		break;
	case EViewMode::QuarterView:
		/* Case 2. QuarterView���� BackView��
		QuarterView�� ��Ʈ�� �����̼ǿ� �� �����̼��� ����ȭ ��.
		BackView�� ��Ʈ�� �����̼��� �������� �����̼ǿ� ����ȭ �ǰ� ����.
		���� ���� ���� ���� ��Ʈ�� �����̼ǿ� �������� �����̼��� �����ص־� ��.
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
	/*	1�� �÷����϶� ��밡��
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
	// ���� ���� ź�� ����
	if (HasAuthority())
	{
		// ���� ������ ź���� ����
		if (CurrentWeapon == "Shotgun")
		{
			ShotgunAmmo = CurrentAmmo;
		}
		else if (CurrentWeapon == "GrenadeLauncher")
		{
			GrenadeLauncherAmmo = CurrentAmmo;
		}

		// �����÷� ���� ��ȯ
		CurrentWeapon = "Rifle";
		CurrentAmmo = RifleAmmo;
		Magazine = RifleMagazine;

		SpawnWeaponInstance1_Server();
	}
	else
	{
		// Ŭ���̾�Ʈ�� ������ ��û
		SetCurrentWeapon_Server("Rifle");
	}
}

void ASPlayerCharacter::InputQuickSlot02(const FInputActionValue& InValue)
{
	//���� ���� ź�� ����
	if (HasAuthority())
	{
		// ���� ������ ź���� ����
		if (CurrentWeapon == "Rifle")
		{
			RifleAmmo = CurrentAmmo;
		}
		else if (CurrentWeapon == "GrenadeLauncher")
		{
			GrenadeLauncherAmmo = CurrentAmmo;
		}

		// �����÷� ���� ��ȯ
		CurrentWeapon = "Shotgun";
		CurrentAmmo = ShotgunAmmo;
		Magazine = ShotgunMagazine;

		SpawnWeaponInstance2_Server();
	}
	else
	{
		// Ŭ���̾�Ʈ�� ������ ��û
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
	//���� ���� ź�� ����
	if (HasAuthority())
	{
		// ���� ������ ź���� ����
		if (CurrentWeapon == "Rifle")
		{
			RifleAmmo = CurrentAmmo;
		}
		else if (CurrentWeapon == "Shotgun")
		{
			ShotgunAmmo = CurrentAmmo;
		}

		// �����÷� ���� ��ȯ
		CurrentWeapon = "GrenadeLauncher";
		CurrentAmmo = GrenadeLauncherAmmo;
		Magazine = GrenadeLauncherMagazine;

		SpawnWeaponInstance3_Server();
	}
	else
	{
		// Ŭ���̾�Ʈ�� ������ ��û
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
		return;		//PlayerController or WeaponInstance�� ������ �ߴ�.
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
			FocalLocation = CameraLocation + (AimDirectionFromCamera * FocalDistance);	//character ��ġ

			FVector WeaponMuzzleLocation = WeaponInstance->GetMesh()->GetSocketLocation(TEXT("MuzzleFlash"));
			FVector FinalFocalLocation = FocalLocation + (((WeaponMuzzleLocation - FocalLocation) | AimDirectionFromCamera) * AimDirectionFromCamera);	//Muzzle��ġ���� ������ ���� ��� ��ġ

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

			// Owning Client ���� ��Ÿ�� ���
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (IsValid(AnimInstance) == true && IsValid(WeaponInstance) == true)
			{
				if (AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()) == false)
				{
					AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
				}
			}

			// Other Client ������ ����ϱ� ���� Server RPC ȣ��.
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

			int32 PelletCount = 10; // ���� �縴 ��
			float SpreadAngle = 10.0f; // ���� ��������

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

			// Owning Client ���� ��Ÿ�� ���
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (IsValid(AnimInstance) && IsValid(WeaponInstance))
			{
				if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()))
				{
					AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
				}
			}

			// Other Client ������ ����ϱ� ���� Server RPC ȣ��.
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
	if (HasAuthority())  // ���������� ����
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

				// �׷����̵带 �������� ����
				if (GrenadeClass)
				{
					UWorld* World = GetWorld();
					if (World)
					{
						AAGrenade* SpawnedGrenade = World->SpawnActor<AAGrenade>(
							GrenadeClass,
							WeaponMuzzleLocation,    // ���� ��ġ (���� �Ա� ��ġ)
							CameraRotation           // ���� ���� (ī�޶��� ����)
						);

						if (SpawnedGrenade)
						{
							FVector LaunchDirection = AimDirectionFromCamera;
							SpawnedGrenade->FireInDirection(LaunchDirection); // �׷����̵带 �߻�
							SpawnedGrenade->SetOwner(this);                    // ������ ����
						}
					}
				}

				// ź�� ���Ҵ� ���������� ó��
				--CurrentAmmo;
				GrenadeLauncherAmmo = CurrentAmmo;

				// Owning Client���� ��Ÿ�� ���
				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				if (IsValid(AnimInstance) && IsValid(WeaponInstance))
				{
					if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()))
					{
						AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
					}
				}

				// Other Client������ ����ϱ� ���� Server RPC ȣ��
				PlayAttackMontage_Server();

				// ������ ȣ��
				if (CurrentAmmo <= 0)
				{
					Reload();
				}
			}
		}
	}
	else
	{
		// Ŭ���̾�Ʈ���� ������ ��û
		TryFire_Server();
	}

	// Ŭ���̾�Ʈ���� ī�޶� ����ũ ó��
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
		return;  // PlayerController�� WeaponInstance�� ��ȿ���� ������ ���� �ߴ�
	}

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CurrentWeapon = %s"), *CurrentWeapon));

	if (HasAuthority())  // ���������� ����
	{
		if (CurrentAmmo > 0)
		{
			// ���⿡ ���� �߻� ó��
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

			// ź�� ����
			--CurrentAmmo;

			// �� ���⺰�� ź�� �� ����
			if (CurrentWeapon == "Rifle")
			{
				RifleAmmo = CurrentAmmo;  // ������ ź���� ����
			}
			else if (CurrentWeapon == "Shotgun")
			{
				ShotgunAmmo = CurrentAmmo;  // ���� ź���� ����
			}
			else if (CurrentWeapon == "GrenadeLauncher")
			{
				GrenadeLauncherAmmo = CurrentAmmo;  // �׷����̵� ��ó ź���� ����
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
			// Ŭ���̾�Ʈ���� ������ �߻� ��û
			TryFire_Server();
		}
		else
		{
			Reload();  // ź���� ���� �� ������ ȣ��
		}
	}

	// Ŭ���̾�Ʈ���� ī�޶� ����ũ ó��
	if (IsValid(FireShake) && GetOwner() == UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->ClientStartCameraShake(FireShake);
	}
}

void ASPlayerCharacter::TryFire_Server_Implementation()
{
	TryFire();  // �������� �߻� ó��
}

void ASPlayerCharacter::PerformRifleFire(APlayerController* PlayerController)
{
#pragma region Rifle Fire Logic
	// Ÿ�� ��ġ ���
	FVector WeaponMuzzleLocation = WeaponInstance->GetMesh()->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector AimDirectionFromCamera = CameraRotation.Vector().GetSafeNormal();
	FVector FocalLocation = CameraLocation + (AimDirectionFromCamera * 400.f);
	FVector FinalFocalLocation = FocalLocation + (((WeaponMuzzleLocation - FocalLocation) | AimDirectionFromCamera) * AimDirectionFromCamera);
	FTransform TargetTransform = FTransform(CameraRotation, FinalFocalLocation);

	// ���� Ʈ���̽�
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

	// Owning Client���� �ִϸ��̼� ���
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) && IsValid(WeaponInstance))
	{
		if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()))
		{
			AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
		}
	}

	// Other Client������ �ִϸ��̼� ����� ���� Server RPC ȣ��
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

	// ���� �߻� - �縴 ��, �������� ����
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

	// Owning Client���� �ִϸ��̼� ���
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) && IsValid(WeaponInstance))
	{
		if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()))
		{
			AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
		}
	}

	// Other Client������ �ִϸ��̼� ����� ���� Server RPC ȣ��
	PlayAttackMontage_Server();
#pragma endregion
}

void ASPlayerCharacter::PerformGrenadeLauncherFire(APlayerController* PlayerController)
{
	if (HasAuthority())  // ���������� ����
	{
		// ĳ���� ��ġ Ȯ��
		FVector CharacterLocation = GetActorLocation();
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Character Location: %s"), *CharacterLocation.ToString()));
		FVector WeaponMuzzleLocation;


		// WeaponInstance Ȯ��
		if (!IsValid(WeaponInstance))
		{
			UKismetSystemLibrary::PrintString(this, TEXT("WeaponInstance is invalid!"));
			return;
		}
		else
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("WeaponInstance is valid")));

			// ���� �޽��� ���� ��ġ Ȯ��
			WeaponMuzzleLocation = WeaponInstance->GetMesh()->GetSocketLocation(TEXT("MuzzleFlash"));
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Weapon Muzzle Location: %s"), *WeaponMuzzleLocation.ToString()));

			WeaponMuzzleLocation.Z += 100.f;

			// �߰��� WeaponInstance�� �ùٸ��� �����Ǿ����� Ȯ��
			FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
			WeaponInstance->AttachToComponent(GetMesh(), AttachRules, TEXT("WeaponSocket"));
		}

		// ī�޶� ��ġ �� �߻� ���� ���
		FVector CameraLocation;
		FRotator CameraRotation;
		PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

		FVector AimDirectionFromCamera = CameraRotation.Vector().GetSafeNormal();

		// GetWorld()�� ��ȿ���� Ȯ��
		if (!GetWorld())
		{
			UKismetSystemLibrary::PrintString(this, TEXT("GetWorld() is invalid!"));
			return;
		}


		// �׷����̵� ����
		if (GrenadeClass)
		{
			UWorld* World = GetWorld();
			AAGrenade* SpawnedGrenade = World->SpawnActor<AAGrenade>(
				GrenadeClass,
				WeaponMuzzleLocation,  // �ѱ� ��ġ���� ����
				CameraRotation         // ī�޶� ȸ�� �������� ����
			);

			if (SpawnedGrenade)
			{
				//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade Spawned on Server")));
				SpawnedGrenade->SetReplicates(true);
				SpawnedGrenade->SetReplicateMovement(true);
				SpawnedGrenade->FireInDirection(AimDirectionFromCamera);  // �߻� ���� ����
				SpawnedGrenade->SetOwner(this);
				FVector GrenadeLocation = SpawnedGrenade->GetActorLocation();
				//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade Location: %s"), *GrenadeLocation.ToString()));


				FTimerHandle TimerHandle;
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, SpawnedGrenade]() {
					SpawnGrenade_NetMulticast(SpawnedGrenade);
					}, 0.2f, false);  // 0.2�� �Ŀ� NetMulticast ȣ��
			}
		}
	}

	// Owning Client���� �ִϸ��̼� ���
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) && IsValid(WeaponInstance))
	{
		if (!AnimInstance->Montage_IsPlaying(WeaponInstance->GetRifleFireAnimMontage()))
		{
			AnimInstance->Montage_Play(WeaponInstance->GetRifleFireAnimMontage());
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade FireAnim")));

		}
	}

	// Other Client������ �ִϸ��̼� ����� ���� Server RPC ȣ��
	PlayAttackMontage_Server();
#pragma endregion
}

void ASPlayerCharacter::SpawnGrenade_NetMulticast_Implementation(AAGrenade* SpawnedGrenade)
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("NetMulticast function executed")));

	if (SpawnedGrenade)
	{
		FVector ServerLocation = SpawnedGrenade->GetActorLocation();  // ���������� �׷����̵� ��ġ

		// **Ŭ���̾�Ʈ���� ��ġ ���� ����ȭ**
		if (!HasAuthority())  // Ŭ���̾�Ʈ������ ����
		{
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade Location on Server: %s"), *ServerLocation.ToString()));

			// ���� ��ġ�� Ŭ���̾�Ʈ���� ���� ����ȭ
			SpawnedGrenade->SetActorLocation(ServerLocation);

			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade Location on Client after sync: %s"), *SpawnedGrenade->GetActorLocation().ToString()));
		}

		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Grenade is valid on Client")));
	}
}

void ASPlayerCharacter::Reload()
{
	// ���� �ν��Ͻ��� ��ȿ�ϰ� ������ ���� �ƴ� ��쿡�� ����
	if (IsValid(WeaponInstance) && !IsReloading)
	{
		// �� ���⺰�� �������� �ʿ��� ��� ó��
		if ((CurrentWeapon == "Rifle" && CurrentAmmo < RifleMagazine) ||
			(CurrentWeapon == "Shotgun" && CurrentAmmo < ShotgunMagazine) ||
			(CurrentWeapon == "GrenadeLauncher" && CurrentAmmo < GrenadeLauncherMagazine))
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (IsValid(AnimInstance) && IsValid(WeaponInstance))
			{
				// ���� ������ �´� ������ �ִϸ��̼� ����
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

			// �������� ������ ��Ÿ�� ���� ��û
			PlayReloadMontage_Server();

			// ������ �� ���� ����
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

bool ASPlayerCharacter::SpawnLandMine_Server_Validate()	//���� �Լ�
{
	return true;
}

void ASPlayerCharacter::SpawnLandMine_Server_Implementation()	//���� �Լ�
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
		return;  // HitResult�� ���� ��� �Լ� ����
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

			// Grenade ���� ��ġ�� Ÿ���� ��ġ ���� �Ÿ� ���
			FVector ExplosionLocation = GetActorLocation(); // �׷����̵尡 ��ġ�� ���� ���
			float Distance = FVector::Dist(ExplosionLocation, HitResult.GetActor()->GetActorLocation());

			// �Ÿ����� ������ ����
			float MaxDamage = 100.f;
			float MinDamage = 20.f;
			float MaxDistance = 500.f; // �ִ� �������� ����Ǵ� �Ÿ�
			float MinDistance = 1000.f; // �ּ� �������� ����Ǵ� �Ÿ�

			float DamageToApply = FMath::GetMappedRangeValueClamped(FVector2D(MaxDistance, MinDistance), FVector2D(MaxDamage, MinDamage), Distance);

			// ���� ������ ����
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
		//float BlendWeight = 1.f;	//���� ��� ġ�������Բ� �ϴ� ����ġ.
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
	// �������� Ŭ���̾�Ʈ�� ź�� �� ����ȭ
	SyncAmmo(NewAmmo);
}

void ASPlayerCharacter::SyncAmmo_Implementation(int32 NewAmmo)
{
	CurrentAmmo = NewAmmo;  // Ŭ���̾�Ʈ���� ź�� �� ������Ʈ
}

