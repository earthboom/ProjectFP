// Fill out your copyright notice in the Description page of Project Settings.


#include "FPPlayerState.h"
#include "ProjectFP/Character/FPCharacter.h"
#include "ProjectFP/PlayerController/FPPlayerController.h"

void AFPPlayerState::AddToScore(float scoreAmount)
{
	Score += scoreAmount;
	Character = (Character == nullptr) ? Cast<AFPCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = (Controller == nullptr) ? Cast<AFPPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}
}

void AFPPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = (Character == nullptr) ? Cast<AFPCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = (Controller == nullptr) ? Cast<AFPPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}
}
