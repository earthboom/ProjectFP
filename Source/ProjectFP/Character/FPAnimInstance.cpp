// Fill out your copyright notice in the Description page of Project Settings.


#include "FPAnimInstance.h"
#include "FPCharacter.h"
#include "ProjectFP/Weapon/Weapon.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UFPAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	FPCharacter = Cast<AFPCharacter>(TryGetPawnOwner());
}

void UFPAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (FPCharacter == nullptr)
		FPCharacter = Cast<AFPCharacter>(TryGetPawnOwner());

	if (FPCharacter == nullptr)
		return;

	FVector velocity = FPCharacter->GetVelocity();
	velocity.Z = 0.0f;
	Speed = velocity.Size();

	bIsInAir = FPCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = FPCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f ? true : false;

	bWeaponEquipped = FPCharacter->IsWeaponEquipped();

	EquippedWeapon = FPCharacter->GetEquippedWeapon();

	bIsCrouch = FPCharacter->bIsCrouched;

	bAiming = FPCharacter->IsAiming();

	TurningInPlace = FPCharacter->GetTuringInPlace();

	bRotateRootBone = FPCharacter->ShouldRotateRootBone();

	bElimmed = FPCharacter->IsElimmed();

	// Yaw Offset for Strafing
	FRotator aimRotation = FPCharacter->GetBaseAimRotation();
	FRotator movementRotation = UKismetMathLibrary::MakeRotFromX(FPCharacter->GetVelocity());
	FRotator deltaRot = UKismetMathLibrary::NormalizedDeltaRotator(movementRotation, aimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, deltaRot, DeltaTime, 6.0f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = FPCharacter->GetActorRotation();
	const FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float target = delta.Yaw / DeltaTime;
	const float interp = FMath::FInterpTo(Lean, target, DeltaTime, 6.0f);
	Lean = FMath::Clamp(interp, -90.0f, 90.0f);

	AO_Yaw = FPCharacter->GetAO_Yaw();
	AO_Pitch = FPCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && FPCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector outPosition;
		FRotator outRotation;
		FPCharacter->GetMesh()->TransformToBoneSpace(FName("Hand_R"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, outPosition, outRotation);
		LeftHandTransform.SetLocation(outPosition);
		LeftHandTransform.SetRotation(FQuat(outRotation));

		if (FPCharacter->IsLocallyControlled())
		{
			bLocallyController = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			//const FVector RightHandLocation = RightHandTransform.GetLocation();
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - FPCharacter->GetHitTarget()));
			LookAtRotation.Roll -= 180.0f;
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 20.0f);
		}

		//FTransform muzzleTipTrasnform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//FVector muzzleX(FRotationMatrix(muzzleTipTrasnform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		//DrawDebugLine(GetWorld(), muzzleTipTrasnform.GetLocation(), muzzleTipTrasnform.GetLocation() + muzzleX * 1000.0f, FColor::Red);
		//DrawDebugLine(GetWorld(), muzzleTipTrasnform.GetLocation(), FPCharacter->GetHitTarget(), FColor::Orange);
	}
}
