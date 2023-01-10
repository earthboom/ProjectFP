// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_TurnToTarget.h"
#include "ProjectFP/PlayerController/FPMonsterAIController.h"
#include "ProjectFP/Character/FPCharacter.h"
#include "ProjectFP/Character/FPMonster.h"

#include "BehaviorTree/BlackboardComponent.h"


UBTTask_TurnToTarget::UBTTask_TurnToTarget()
{
	NodeName = TEXT("Turn");
}

EBTNodeResult::Type UBTTask_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type result = Super::ExecuteTask(OwnerComp, NodeMemory);

	auto fpMonster = Cast<AFPMonster>(OwnerComp.GetAIOwner()->GetPawn());
	if (fpMonster == nullptr)
		return EBTNodeResult::Failed;

	auto target = Cast<AFPCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AFPMonsterAIController::TargetKey));
	if (target == nullptr)
		return EBTNodeResult::Failed;

	FVector lookVector = target->GetActorLocation() - fpMonster->GetActorLocation();
	lookVector.Z = 0.0f;
	FRotator targetRot = FRotationMatrix::MakeFromX(lookVector).Rotator();
	fpMonster->SetActorRotation(FMath::RInterpTo(fpMonster->GetActorRotation(), targetRot, GetWorld()->GetDeltaSeconds(), 2.0f));

	return EBTNodeResult::Succeeded;
}
