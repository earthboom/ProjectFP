// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectFP/HUD/FPHUD.h"
#include "CombatComponent.generated.h"

class AWeapon;

const float TRACE_LENGTH = 80000.0f;
const float CROSSHAIT_GAP_DEFAULT = 0.5f;
const float CROSSHAIR_AIM_GAP = 0.58f;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTFP_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AFPCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* weaponToEquip);

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& tracerHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& tracerHitTarget);

	void TraceUnderCrosshairs(FHitResult& traceHitResult);

	void SetHUDCrosshairs(float deltaTime);

private:
	class AFPCharacter* Character;
	class AFPPlayerController* Controller;
	class AFPHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	//HUD, Crosshairs =========
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;
	FHUDPackage HUDPackage;
	// =======================
	
	// Aiming, FOV ===========
	// 조준하지 않을 때의 시야 - 카메라 기본 시야 또는 FMV로 설정하고 게임을 시작
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV{ 45.0f };

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed{ 20.0f };

	void InterpFOV(float deltaTime);
	//========================

	// Automatic fire ========	
	FTimerHandle FireTimer;
	bool bCanFire{ true };

	void StartFireTimer();
	void FireTimerFinished();
	// =======================
};
