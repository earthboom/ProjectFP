// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnSystem.generated.h"

UCLASS()
class PROJECTFP_API ASpawnSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpawnSystem();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:	
	static const uint32 MonsterCountMax{ 20 };
	static uint32 MonsterCounter;

private:
	TArray<class AMonsterSpawner* > MonsterSpawners;

	float SpawnTimer{ 2.0f };
	float MonsterSpawnTimer{ 0.0f };	
};
