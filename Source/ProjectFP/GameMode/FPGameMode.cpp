// Fill out your copyright notice in the Description page of Project Settings.


#include "FPGameMode.h"
#include "ProjectFP/Character/FPCharacter.h"
#include "ProjectFP/Character/FPMonster.h"
#include "ProjectFP/PlayerController/FPPlayerController.h"
#include "ProjectFP/PlayerController/FPMonsterAIController.h"
#include "ProjectFP/PlayerState/FPPlayerState.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void AFPGameMode::PlayerEliminated(AFPCharacter* elimmedCharacter, AFPPlayerController* victimController, AFPPlayerController* attackerController)
{
	AFPPlayerState* attackerPlayerState = attackerController ? Cast<AFPPlayerState>(attackerController->PlayerState) : nullptr;
	AFPPlayerState* victimPlayerState = victimController ? Cast<AFPPlayerState>(victimController->PlayerState) : nullptr;
	if (attackerPlayerState && attackerPlayerState != victimPlayerState)
	{
		attackerPlayerState->AddToScore(1.0f);
	}

	if (elimmedCharacter)
		elimmedCharacter->Elim();
}

void AFPGameMode::RequestRespawn(ACharacter* elimmedCharacter, AController* elimmedController)
{
	if (elimmedCharacter)
	{
		elimmedCharacter->Reset();
		elimmedCharacter->Destroy();
	}

	if (elimmedController)
	{
		TArray<AActor*> playerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), playerStarts);
		int32 selection = FMath::RandRange(0, playerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(elimmedController, playerStarts[selection]);
	}
}

void AFPGameMode::MonsterEliminated(AFPMonster* elimmedMonster, AFPPlayerController* attackerController)
{
	AFPPlayerState* attackerPlayerState = attackerController ? Cast<AFPPlayerState>(attackerController->PlayerState) : nullptr;
	if (attackerPlayerState)
		attackerPlayerState->AddToScore(10.0f);

	if (elimmedMonster)
		elimmedMonster->Elim();
}
