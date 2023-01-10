// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "ProjectFP/Character/FPMonster.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherCompo, FVector NormalImpulse, const FHitResult& Hit)
{
	AFPMonster* monter = Cast<AFPMonster>(OtherActor);
	if (monter)
	{
		ACharacter* ownerCharacter = Cast<ACharacter>(GetOwner());
		if (ownerCharacter)
		{
			AController* ownerController = ownerCharacter->Controller;
			if (ownerController)
			{
				UGameplayStatics::ApplyDamage(OtherActor, Damage, ownerController, this, UDamageType::StaticClass());
			}
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherCompo, NormalImpulse, Hit);
}
