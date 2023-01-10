// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_Detect.h"
#include "ProjectFP/PlayerController/FPMonsterAIController.h"
#include "ProjectFP/Character/FPCharacter.h"
#include "ProjectFP/Character/FPMonster.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"

UBTService_Detect::UBTService_Detect()
{
	NodeName = TEXT("Detect");
	Interval = 1.0f;
}

void UBTService_Detect::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	//APawn* controllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	AFPMonster* controllingPawn = Cast<AFPMonster>(OwnerComp.GetAIOwner()->GetPawn());
	if (controllingPawn == nullptr)
		return;

	UWorld* world = controllingPawn->GetWorld();
	if (world == nullptr)
		return;

	FVector center = controllingPawn->GetActorLocation();
	float detectRadius = 4000.0f;

	TArray<FOverlapResult> overlapResults;
	FCollisionQueryParams collisionQueryParam(NAME_None, false, controllingPawn);
	bool bResult = world->OverlapMultiByChannel(
		overlapResults,
		center,
		FQuat::Identity,
		//ECollisionChannel::ECC_GameTraceChannel2,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeSphere(detectRadius),
		collisionQueryParam
	);

	if (bResult)
	{
		float previousDist = 0.0f;
		if(controllingPawn->TargetCharacter)
			previousDist = controllingPawn->GetDistanceTo(controllingPawn->TargetCharacter);

		//UE_LOG(LogTemp, Warning, TEXT("previous Distance : %f"), previousDist);

		float currentDist = 0.0f;
		for (auto const& overlapResult : overlapResults)
		{
			AFPCharacter* fpCharacter = Cast<AFPCharacter>(overlapResult.GetActor());						
			if (fpCharacter && fpCharacter->GetController()->IsPlayerController())
			{
				if (controllingPawn->TargetCharacter && controllingPawn->TargetCharacter != fpCharacter)
				{
					currentDist = controllingPawn->GetDistanceTo(fpCharacter);
					//UE_LOG(LogTemp, Warning, TEXT("current Distance : %f"), currentDist);
					if (currentDist >= previousDist)
						continue;
				}

				OwnerComp.GetBlackboardComponent()->SetValueAsObject(AFPMonsterAIController::TargetKey, fpCharacter);
				controllingPawn->TargetCharacter = fpCharacter;

				DrawDebugSphere(world, center, detectRadius, 16, FColor::Green, false, 0.2f);
				//DrawDebugPoint(world, fpCharacter->GetActorLocation(), 10.0f, FColor::Blue, false, 0.2f);
				//DrawDebugLine(world, controllingPawn->GetActorLocation(), fpCharacter->GetActorLocation(), FColor::Blue, false, 0.27f);
				return;
			}
		}
	}

	OwnerComp.GetBlackboardComponent()->SetValueAsObject(AFPMonsterAIController::TargetKey, nullptr);
	//DrawDebugSphere(world, center, detectRadius, 16, FColor::Red, false, 0.2f);
}
