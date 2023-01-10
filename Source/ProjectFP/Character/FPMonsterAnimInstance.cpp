// Fill out your copyright notice in the Description page of Project Settings.


#include "FPMonsterAnimInstance.h"
#include "FPMonster.h"

#include "GameFramework/CharacterMovementComponent.h"

void UFPMonsterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	FPMonster = Cast<AFPMonster>(TryGetPawnOwner());
}

void UFPMonsterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (FPMonster == nullptr)
		FPMonster = Cast<AFPMonster>(TryGetPawnOwner());

	if (FPMonster == nullptr)
		return;

	FVector velocity = FPMonster->GetVelocity();
	velocity.Z = 0.0f;
	Speed = velocity.Size();

	bIsAccelerating = FPMonster->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f ? true : false;
}

void UFPMonsterAnimInstance::AnimNotify_Check_BH_Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("Notify - BH_ATtack"));
	OnAttackHitCheck.Broadcast();
}

void UFPMonsterAnimInstance::AnimNotify_Check_LH_Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("Notify - LH_ATtack"));
	OnAttackHitCheck.Broadcast();
}

void UFPMonsterAnimInstance::AnimNotify_Check_RH_Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("Notify - RH_ATtack"));
	OnAttackHitCheck.Broadcast();
}
