// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterOverheadWidget.h"

void UMonsterOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
