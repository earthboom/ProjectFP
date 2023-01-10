
#include "SpawnSystem.h"
#include "ProjectFP/Spawners/MonsterSpawner.h"

#include "Kismet/GameplayStatics.h"

uint32 ASpawnSystem::MonsterCounter = 0;

ASpawnSystem::ASpawnSystem()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASpawnSystem::BeginPlay()
{
	Super::BeginPlay();

	MonsterCounter = 0;
	
	TArray<AActor*> monsterSpawners;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMonsterSpawner::StaticClass(), monsterSpawners);

	for (auto& spawner : monsterSpawners)
	{
		MonsterSpawners.Emplace(Cast<AMonsterSpawner>(spawner));
	}
	//UE_LOG(LogTemp, Warning, TEXT("MonsterSpawners count %d"), MonsterSpawners.Num());
}

void ASpawnSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!MonsterSpawners.IsEmpty())
	{
		if (MonsterSpawnTimer += DeltaTime; MonsterSpawnTimer > SpawnTimer)
		{
			MonsterSpawnTimer = 0.0f;

			UINT num = FMath::RandRange(0, MonsterSpawners.Num() - 1);
			MonsterSpawners[num]->SpawnMonster();
		}
	}
}

