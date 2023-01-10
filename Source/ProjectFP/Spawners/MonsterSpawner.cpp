
#include "MonsterSpawner.h"
#include "SpawnSystem.h"
#include "ProjectFP/Character/FPMonster.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"

AMonsterSpawner::AMonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SpawnArea->SetupAttachment(RootComponent);
	SpawnArea->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();
	
}
void AMonsterSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMonsterSpawner::SpawnMonster()
{
	FActorSpawnParameters spawnParam;
	spawnParam.Owner = this;
	spawnParam.Instigator = GetInstigator();
	
	FVector location = GetRandomPoint();
	FRotator rotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

	UWorld* world = GetWorld();
	if (world)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnMonster MonsterCounter %d"), ASpawnSystem::MonsterCounter);
		if (ASpawnSystem::MonsterCounter < ASpawnSystem::MonsterCountMax)
		{
			world->SpawnActor<AFPMonster>(FPMonster, location, rotation, spawnParam);
			++ASpawnSystem::MonsterCounter;
		}
	}
}

const FVector AMonsterSpawner::GetRandomPoint()
{
	const FVector center = SpawnArea->Bounds.Origin;
	const FVector halfSize = SpawnArea->Bounds.BoxExtent;

	return UKismetMathLibrary::RandomPointInBoundingBox(center, halfSize);
}
