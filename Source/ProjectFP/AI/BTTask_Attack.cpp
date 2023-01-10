// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "ProjectFP/PlayerController/FPMonsterAIController.h"
#include "ProjectFP/Character/FPMonster.h"

UBTTask_Attack::UBTTask_Attack()
{
	bNotifyTick = true;
	IsAttacking = false;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type result = Super::ExecuteTask(OwnerComp, NodeMemory);

	auto fpMonster = Cast<AFPMonster>(OwnerComp.GetAIOwner()->GetPawn());
	if (fpMonster == nullptr)
		return EBTNodeResult::Failed;

	if (fpMonster->IsAttack())
		return EBTNodeResult::Failed;

	fpMonster->MulticastAttack();
	IsAttacking = true;
	fpMonster->OnAttackEnd.AddLambda([this]() {
		//UE_LOG(LogTemp, Warning, TEXT("UBTTask_Attack - OnAttackEnd.AddLambda"));
		IsAttacking = false; 
	});

	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	if (!IsAttacking)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
