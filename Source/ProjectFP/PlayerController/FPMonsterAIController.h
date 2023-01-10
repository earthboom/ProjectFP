// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FPMonsterAIController.generated.h"

/**
 * 
 */

class UMonsterOverheadWidget;

UCLASS()
class PROJECTFP_API AFPMonsterAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AFPMonsterAIController();
	virtual void OnPossess(APawn* InPawn) override;

	static const FName HomePosKey;
	static const FName PatrolPosKey;
	static const FName TargetKey;

	void RunAI();
	void StopAI();

private:
	UMonsterOverheadWidget* MonsterOverheadWidget;

	UPROPERTY()
	class UBehaviorTree* BTAsset;

	UPROPERTY()
	class UBlackboardData* BBASset;
};
