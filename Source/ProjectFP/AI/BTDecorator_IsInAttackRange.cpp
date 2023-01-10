// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_IsInAttackRange.h"
#include "ProjectFP/PlayerController/FPMonsterAIController.h"
#include "ProjectFP/Character/FPCharacter.h"
#include "ProjectFP/Character/FPMonster.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsInAttackRange::UBTDecorator_IsInAttackRange()
{
	NodeName = TEXT("CanAttack");
}

bool UBTDecorator_IsInAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	auto controllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (controllingPawn == nullptr)
		return false;
	
	auto target = Cast<AFPCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AFPMonsterAIController::TargetKey));
	if (target == nullptr)
		return false;

	bResult = (target->GetDistanceTo(controllingPawn) <= 300.0f);
	return bResult;
}
