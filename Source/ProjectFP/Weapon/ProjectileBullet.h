// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTFP_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherCompo, FVector NormalImpulse, const FHitResult& Hit) override;
};
