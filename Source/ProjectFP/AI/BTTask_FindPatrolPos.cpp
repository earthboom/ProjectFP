// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindPatrolPos.h"
#include "ProjectFP/PlayerController/FPMonsterAIController.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindPatrolPos::UBTTask_FindPatrolPos()
{
    NodeName = TEXT("FindPatrolPos");
}

EBTNodeResult::Type UBTTask_FindPatrolPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

    auto controllingPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (controllingPawn == nullptr)
        return EBTNodeResult::Failed;

    UNavigationSystemV1* navSystem = UNavigationSystemV1::GetNavigationSystem(controllingPawn->GetWorld());
    if (navSystem == nullptr)
        return EBTNodeResult::Failed;

    FVector origin = OwnerComp.GetBlackboardComponent()->GetValueAsVector(AFPMonsterAIController::HomePosKey);
    FNavLocation nextPatrol;

    if (navSystem->GetRandomPointInNavigableRadius(origin, 1000.0f, nextPatrol))
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(AFPMonsterAIController::PatrolPosKey, nextPatrol.Location);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
