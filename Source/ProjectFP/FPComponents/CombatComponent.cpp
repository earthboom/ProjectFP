

#include "CombatComponent.h"
#include "ProjectFP/Weapon/Weapon.h"
#include "ProjectFP/Character/FPCharacter.h"
#include "ProjectFP/PlayerController/FPPlayerController.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	AimWalkSpeed = 350.0f;
	BaseWalkSpeed = 600.0f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled()) {
		FHitResult hitResult;
		TraceUnderCrosshairs(hitResult);
		HitTarget = hitResult.ImpactPoint;
	
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
		Fire();
}

void UCombatComponent::Fire()
{
	if (bCanFire)
	{
		bCanFire = false;
		ServerFire(HitTarget);

		if (EquippedWeapon)
			CrosshairShootingFactor = 1.0f;

		StartFireTimer();
	}	
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr)
		return;

	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr)
		return;

	bCanFire = true;

	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
		Fire();
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& tracerHitTarget)
{
	MulticastFire(tracerHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& tracerHitTarget)
{
	if (EquippedWeapon == nullptr)
		return;

	if (Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(tracerHitTarget);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* weaponToEquip)
{
	if (Character == nullptr || weaponToEquip == nullptr)
		return;

	EquippedWeapon = weaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	// Attach weapon to RightHandSocket
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());

	// Character가 Weapon의 소유권을 가지도록
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		// Attach weapon to RightHandSocket
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& traceHitResult)
{
	FVector2D viewportSize;
	if (GEngine && GEngine->GameViewport)
		GEngine->GameViewport->GetViewportSize(viewportSize);

	FVector2D crosshairLocation(viewportSize.X / 2.0f, viewportSize.Y / 2.0f);	// Center of viewport
	FVector crosshairWorldPosition;
	FVector crosshairWorldDirection;
	// ScreenSpace -> WorldSpace
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		crosshairLocation,
		crosshairWorldPosition,
		crosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector start = crosshairWorldPosition;

		if (Character)
		{
			float distanceToCahracter = (Character->GetActorLocation() - start).Size();
			start += (crosshairWorldDirection * (distanceToCahracter + 100.0f));
			//DrawDebugSphere(GetWorld(), start, 16.0f, 12, FColor::Red, false);
		}

		FVector end = start + crosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(traceHitResult, start, end, ECollisionChannel::ECC_Visibility);

		if (traceHitResult.GetActor() && traceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		else
			HUDPackage.CrosshairsColor = FLinearColor::White;

		//if (!traceHitResult.bBlockingHit)
		//	traceHitResult.ImpactPoint = end;
		//else
		//	DrawDebugSphere(GetWorld(), traceHitResult.ImpactPoint, 10.0f, 10.0f, FColor::Red);
	}
}

void UCombatComponent::SetHUDCrosshairs(float deltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr)
		return;

	Controller = (Controller == nullptr) ? Cast<AFPPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = (HUD == nullptr) ? Cast<AFPHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}

			// 조준선 퍼짐 계산
			// (0, 600) -> (0, 1)
			FVector2D walkSpeedRange(0.0f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D velocityMultiplayerRage(0.0f, 1.0f);
			FVector velocity = Character->GetVelocity();
			velocity.Z = 0.0f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(walkSpeedRange, velocityMultiplayerRage, velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, deltaTime, 2.25f);
			else
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.0f, deltaTime, 30.0f);

			if (bAiming)
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, CROSSHAIR_AIM_GAP, deltaTime, 30.0f);
			else
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.0f, deltaTime, 30.0f);

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, deltaTime, 10.0f);

			HUDPackage.CrosshairSpread = CROSSHAIT_GAP_DEFAULT + (CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairShootingFactor) - CrosshairAimFactor;
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float deltaTime)
{
	if (EquippedWeapon == nullptr)
		return;

	if (bAiming)
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), deltaTime, EquippedWeapon->GetZoomInterpSpeed());
	else
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, deltaTime, EquippedWeapon->GetZoomInterpSpeed());

	if (Character && Character->GetFollowCamera())
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAming(bIsAiming);

	if (Character)
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
}

void UCombatComponent::ServerSetAming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	if (Character)
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
}