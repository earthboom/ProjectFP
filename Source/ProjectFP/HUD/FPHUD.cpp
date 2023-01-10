// Fill out your copyright notice in the Description page of Project Settings.


#include "FPHUD.h"
#include "CharacterOverlay.h"

#include "GameFramework/PlayerController.h"

void AFPHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
}

void AFPHUD::AddCharacterOverlay()
{
	APlayerController* playerController = GetOwningPlayerController();
	if (playerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(playerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void AFPHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D viewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(viewportSize);
		const FVector2D viewportCenter(viewportSize.X / 2.0f, viewportSize.Y / 2.0f);

		float spreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D spread(0.0f, 0.0f);
			DrawCrosshairs(HUDPackage.CrosshairsCenter, viewportCenter, spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D spread(-spreadScaled, 0.0f);
			DrawCrosshairs(HUDPackage.CrosshairsLeft, viewportCenter, spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsRight)
		{
			FVector2D spread(spreadScaled, 0.0f);
			DrawCrosshairs(HUDPackage.CrosshairsRight, viewportCenter, spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsTop)
		{
			FVector2D spread(0.0f, -spreadScaled);
			DrawCrosshairs(HUDPackage.CrosshairsTop, viewportCenter, spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D spread(0.0f, spreadScaled);
			DrawCrosshairs(HUDPackage.CrosshairsBottom, viewportCenter, spread, HUDPackage.CrosshairsColor);
		}
	}
}

void AFPHUD::DrawCrosshairs(UTexture2D* texture, FVector2D viewportCenter, FVector2D spread, FLinearColor crosshairsColor)
{
	const float textureWidth = texture->GetSizeX();
	const float textureHeight = texture->GetSizeY();
	const FVector2D textureDrawPoint(
		viewportCenter.X - (textureWidth / 2.0f) + spread.X, 
		viewportCenter.Y - (textureHeight / 2.0f) + spread.Y
	);

	DrawTexture(
		texture,
		textureDrawPoint.X, textureDrawPoint.Y,
		textureWidth, textureHeight,
		0.0f, 0.0f,
		1.0f, 1.0f,
		crosshairsColor
	);
}
