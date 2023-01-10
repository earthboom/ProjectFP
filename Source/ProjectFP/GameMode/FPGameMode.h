// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "FPGameMode.generated.h"

/**
 * 
 */

class AFPCharacter;
class AFPMonster;
class AFPPlayerController;

UCLASS()
class PROJECTFP_API AFPGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PlayerEliminated(AFPCharacter* elimmedCharacter, AFPPlayerController* victimController, AFPPlayerController* attackerController);
	virtual void RequestRespawn(ACharacter* elimmedCharacter, AController* elimmedController);
	virtual void MonsterEliminated(AFPMonster* elimmedMonster, AFPPlayerController* attackerController);
};
