// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FPPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTFP_API AFPPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void OnRep_Score() override;
	void AddToScore(float scoreAmount);

private:
	class AFPCharacter* Character;
	class AFPPlayerController* Controller;
};
