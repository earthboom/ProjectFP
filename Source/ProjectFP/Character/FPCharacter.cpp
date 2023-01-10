
#include "FPCharacter.h"
#include "FPAnimInstance.h"
#include "ProjectFP/ProjectFP.h"
#include "ProjectFP/Weapon/Weapon.h"
#include "ProjectFP/FPComponents/CombatComponent.h"
#include "ProjectFP/PlayerController/FPPlayerController.h"
#include "ProjectFP/GameMode/FPGameMode.h"
#include "ProjectFP/PlayerState/FPPlayerState.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

AFPCharacter::AFPCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	//SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 850.0f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void AFPCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AFPCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AFPCharacter, Health);
}

void AFPCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();

	TimeSinceLastMovementReplication = 0.0f;
}

void AFPCharacter::Elim()
{
	if (Combat && Combat->EquippedWeapon)
		Combat->EquippedWeapon->Dropped();

	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &AFPCharacter::ElimTimerFinished, ElimDelay);
}

void AFPCharacter::MulticastElim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();

	// Dissolve Effect 시작
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 100.0f);
	}

	StartDissolve();

	// Character Movement 비활성화
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (FPPlayerController)
		DisableInput(FPPlayerController);

	// Collision 비활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFPCharacter::ElimTimerFinished()
{
	AFPGameMode* fpGameMode = GetWorld()->GetAuthGameMode<AFPGameMode>();
	if (fpGameMode)
		fpGameMode->RequestRespawn(this, Controller);
}

void AFPCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AFPCharacter::ReceiveDamage);
	}
}

void AFPCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
		AimOffset(DeltaTime);
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
			OnRep_ReplicateMovement();

		CalculateAO_Pitch();
	}

	HideCameraIfCharacterClose();
	PollInit();
}

void AFPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFPCharacter::Jump);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AFPCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AFPCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AFPCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AFPCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AFPCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AFPCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AFPCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AFPCharacter::AimButtonRelease);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPCharacter::FireButtonRelease);
}

void AFPCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
		Combat->Character = this;
}

void AFPCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
		return;

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && FireWeaponMontage)
	{
		animInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		animInstance->Montage_JumpToSection(SectionName);
	}
}

void AFPCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
		return;

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && HitReactMontage)
	{
		animInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		animInstance->Montage_JumpToSection(SectionName);
	}
}

void AFPCharacter::PlayElimMontage()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && ElimMontage)
		animInstance->Montage_Play(ElimMontage);
}

void AFPCharacter::MoveForward(float value)
{
	if (Controller != nullptr && value != 0.0f)
	{
		const FRotator yawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector direction(FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(direction, value);
	}
}

void AFPCharacter::MoveRight(float value)
{
	if (Controller != nullptr && value != 0.0f)
	{
		const FRotator yawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector direction(FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(direction, value);
	}
}

void AFPCharacter::Turn(float value)
{
	AddControllerYawInput(value);
}

void AFPCharacter::LookUp(float value)
{
	AddControllerPitchInput(value);
}

void AFPCharacter::EquipButtonPressed()
{
	if (Combat)
	{
		if (HasAuthority())
			Combat->EquipWeapon(OverlappingWeapon);
		else
			ServerEquipButtonPressed();
	}
}

void AFPCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
		Combat->EquipWeapon(OverlappingWeapon);
}

void AFPCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
		UnCrouch();
	else
		Crouch();
}

void AFPCharacter::AimButtonPressed()
{
	if (Combat)
		Combat->SetAiming(true);
}

void AFPCharacter::AimButtonRelease()
{
	if (Combat)
		Combat->SetAiming(false);
}

float AFPCharacter::CalculateSpeed()
{
	FVector velocity = GetVelocity();
	velocity.Z = 0.0f;
	return velocity.Size();
}

void AFPCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr)
		return;

	float speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (speed == 0.0f && !bIsInAir)	
	{
		bRotateRootbone = true;

		FRotator currentAimRotation = FRotator(0.0, GetBaseAimRotation().Yaw, 0.0f);
		FRotator deltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(currentAimRotation, StartingAimRotation);
		AO_Yaw = deltaAimRotation.Yaw;

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
			InterpAO_Yaw = AO_Yaw;

		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}

	if (speed > 0.0f || bIsInAir)
	{
		bRotateRootbone = false;

		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.0f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void AFPCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.0f && !IsLocallyControlled())
	{
		// (270, 360) ~ (-90, 0)
		FVector2D inRange(270.0f, 360.0f);
		FVector2D outRange(-90.0f, 0.0f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(inRange, outRange, AO_Pitch);
	}
}

void AFPCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
		return;

	bRotateRootbone = false;

	float speed = CalculateSpeed();
	if(speed > 0.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	// UE_LOG(LogTemp, Warning, TEXT("ProxyYaw : %f"), ProxyYaw);

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
			TurningInPlace = ETurningInPlace::ETIP_Right;
		else if(ProxyYaw < -TurnThreshold)
			TurningInPlace = ETurningInPlace::ETIP_Left;
		else 
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;

		return;
	}

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void AFPCharacter::Jump()
{
	if (bIsCrouched)
		UnCrouch();
	else
		Super::Jump();
}

void AFPCharacter::FireButtonPressed()
{
	if (Combat)
		Combat->FireButtonPressed(true);
}

void AFPCharacter::FireButtonRelease()
{
	if (Combat)
		Combat->FireButtonPressed(false);
}

void AFPCharacter::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCause)
{
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health <= 0.0f)
	{
		AFPGameMode* fpGameMode = GetWorld()->GetAuthGameMode<AFPGameMode>();
		if (fpGameMode)
		{
			FPPlayerController = (FPPlayerController == nullptr) ? Cast<AFPPlayerController>(Controller) : FPPlayerController;
			AFPPlayerController* attackterController = Cast<AFPPlayerController>(InstigatorController);
			fpGameMode->PlayerEliminated(this, FPPlayerController, attackterController);
		}
	}	
}

void AFPCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.0f)
		TurningInPlace = ETurningInPlace::ETIP_Right;
	else if (AO_Yaw < -90.0f)
		TurningInPlace = ETurningInPlace::ETIP_Left;

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.0f, DeltaTime, 4.0f);
		AO_Yaw = InterpAO_Yaw;

		if (FMath::Abs(AO_Yaw) < 15.0f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}

void AFPCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled())
		return;

	float distanceCamToChar = (FollowCamera->GetComponentLocation() - GetActorLocation()).Size();
	if (distanceCamToChar < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
	}
}

void AFPCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void AFPCharacter::UpdateHUDHealth()
{
	FPPlayerController = (FPPlayerController == nullptr) ? Cast<AFPPlayerController>(Controller) : FPPlayerController;
	if (FPPlayerController)
		FPPlayerController->SetHUDHealth(Health, MaxHealth);
}

void AFPCharacter::PollInit()
{
	if (FPPlayerState == nullptr)
	{
		FPPlayerState = GetPlayerState<AFPPlayerState>();
		if (FPPlayerState)
			FPPlayerState->AddToScore(0.0f);
	}
}

void AFPCharacter::UpdateDissolveMaterial(float dissolveValue)
{
	if (DynamicDissolveMaterialInstance)
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), dissolveValue);
}

void AFPCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AFPCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void AFPCharacter::SetOverlappingWeapon(AWeapon* weapon)
{
	if (OverlappingWeapon)
		OverlappingWeapon->ShowPickupWidget(false);

	OverlappingWeapon = weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
			OverlappingWeapon->ShowPickupWidget(true);
	}
}

void AFPCharacter::OnRep_OverlappingWeapon(AWeapon* lastWeapon)
{
	if (OverlappingWeapon)
		OverlappingWeapon->ShowPickupWidget(true);

	if (lastWeapon)
		lastWeapon->ShowPickupWidget(false);
}

bool AFPCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AFPCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* AFPCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr)
		return nullptr;

	return Combat->EquippedWeapon;
}

FVector AFPCharacter::GetHitTarget() const
{
	if (Combat == nullptr)
		return FVector();

	return Combat->HitTarget;
}
