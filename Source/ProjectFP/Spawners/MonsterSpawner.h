// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawner.generated.h"

UCLASS()
class PROJECTFP_API AMonsterSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AMonsterSpawner();

	virtual void Tick(float DeltaTime) override;

	void SpawnMonster();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Spawner Option")
	class UBoxComponent* SpawnArea;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<class AFPMonster> FPMonster;

	const FVector GetRandomPoint();
};
