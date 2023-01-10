// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTFP_API AFPPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float health, float maxHealth);
	void SetHUDScore(float score);
	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;

private:
	class AFPHUD* FPHUD;
};
