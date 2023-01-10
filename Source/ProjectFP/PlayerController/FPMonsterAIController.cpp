// Fill out your copyright notice in the Description page of Project Settings.


#include "FPMonsterAIController.h"
#include "ProjectFP/HUD/MonsterOverheadWidget.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

const FName AFPMonsterAIController::HomePosKey(TEXT("HomePos"));
const FName AFPMonsterAIController::PatrolPosKey(TEXT("PatrolPos"));
const FName AFPMonsterAIController::TargetKey(TEXT("Target"));

AFPMonsterAIController::AFPMonsterAIController()
{
	static ConstructorHelpers::FObjectFinder<UBlackboardData> bbObject(TEXT("/Game/Blueprints/Character/AI/BB_FPMonster.BB_FPMonster"));
	if (bbObject.Succeeded())
		BBASset = bbObject.Object;


	static ConstructorHelpers::FObjectFinder<UBehaviorTree> btObject(TEXT("/Game/Blueprints/Character/AI/BT_FPMonster.BT_FPMonster"));
	if (btObject.Succeeded())
		BTAsset = btObject.Object;
}

void AFPMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UBlackboardComponent* blackboardComp = Blackboard;
	if (UseBlackboard(BBASset, blackboardComp))
	{
		blackboardComp->SetValueAsVector(HomePosKey, GetPawn()->GetActorLocation());
		if (!RunBehaviorTree(BTAsset))
			UE_LOG(LogTemp, Warning, TEXT("AIController couldn't run behavior tree"));
	}
}

void AFPMonsterAIController::RunAI()
{
	UBlackboardComponent* blackboardComp = Blackboard;
	if (UseBlackboard(BBASset, blackboardComp))
	{
		blackboardComp->SetValueAsVector(HomePosKey, GetPawn()->GetActorLocation());
		if (!RunBehaviorTree(BTAsset))
			UE_LOG(LogTemp, Warning, TEXT("AIController couldn't run behavior tree"));
	}
}

void AFPMonsterAIController::StopAI()
{
}
