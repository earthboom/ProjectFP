// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FPMonsterAnimInstance.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnAttackHitCheckDelegate);

/**
 * 
 */
UCLASS()
class PROJECTFP_API UFPMonsterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	UFUNCTION()
	void AnimNotify_Check_BH_Attack();
	
	UFUNCTION()
	void AnimNotify_Check_LH_Attack();
	
	UFUNCTION()
	void AnimNotify_Check_RH_Attack();

public:
	FOnAttackHitCheckDelegate OnAttackHitCheck;

private:
	UPROPERTY(BlueprintReadOnly, Category = "Monster", meta = (AllowPrivateAccess = "true"))
	class AFPMonster* FPMonster;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;
};
