// Fill out your copyright notice in the Description page of Project Settings.


#include "FPPlayerController.h"
#include "ProjectFP/HUD/FPHUD.h"
#include "ProjectFP/HUD/CharacterOverlay.h"
#include "ProjectFP/Character/FPCharacter.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void AFPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FPHUD = Cast<AFPHUD>(GetHUD());
}

void AFPPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AFPCharacter* fpCharacter = Cast<AFPCharacter>(InPawn);
	if (fpCharacter)
		SetHUDHealth(fpCharacter->GetHealth(), fpCharacter->GetMaxHealth());
}

void AFPPlayerController::SetHUDHealth(float health, float maxHealth)
{
	FPHUD = (FPHUD == nullptr) ? Cast<AFPHUD>(GetHUD()) : FPHUD;

	bool bHUDValid = FPHUD && FPHUD->CharacterOverlay && FPHUD->CharacterOverlay->HealthBar && FPHUD->CharacterOverlay->HealthText;

	//if (FPHUD)
	//	UE_LOG(LogTemp, Warning, TEXT("FPHUD valid"));

	//if(FPHUD && FPHUD->CharacterOverlay)
	//	UE_LOG(LogTemp, Warning, TEXT("FPHUD->CharacterOverlay valid"));

	if (bHUDValid)
	{
		const float healthPercent = health / maxHealth;
		FPHUD->CharacterOverlay->HealthBar->SetPercent(healthPercent);

		FString healthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(health), FMath::CeilToInt(maxHealth));
		FPHUD->CharacterOverlay->HealthText->SetText(FText::FromString(healthText));
	}
}

void AFPPlayerController::SetHUDScore(float score)
{
	FPHUD = (FPHUD == nullptr) ? Cast<AFPHUD>(GetHUD()) : FPHUD;

	bool bHUDValid = FPHUD && FPHUD->CharacterOverlay && FPHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString scoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(score));
		FPHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(scoreText));
	}
}
