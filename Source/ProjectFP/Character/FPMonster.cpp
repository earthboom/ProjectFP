

#include "FPMonster.h"
#include "FPMonsterAnimInstance.h"
#include "ProjectFP/ProjectFP.h"
#include "ProjectFP/PlayerController/FPMonsterAIController.h"
#include "ProjectFP/PlayerController/FPPlayerController.h"
#include "ProjectFP/HUD/MonsteroverheadWidget.h"
#include "ProjectFP/GameMode/FPGameMode.h"
#include "ProjectFP/Spawners/SpawnSystem.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/ProgressBar.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

AFPMonster::AFPMonster()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	GetMesh()->SetEnableGravity(true);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 720.0f);
	GetCharacterMovement()->MaxWalkSpeed = 480.0f;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	AIControllerClass = AFPMonsterAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	FPMonsterController = Cast<AFPMonsterAIController>(Controller);

	AttackMontageNames = {FName("BH_Attack"), FName("LH_Attack"), FName("RH_Attack")};

	AttackRange = 200.0f;
	AttackRadius = 50.0f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void AFPMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFPMonster::Destroyed()
{
	--ASpawnSystem::MonsterCounter;
	UE_LOG(LogTemp, Warning, TEXT("ElimTimerFinished : %d"), ASpawnSystem::MonsterCounter);
}

void AFPMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPMonster, Health);
}

void AFPMonster::Elim()
{
	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &AFPMonster::ElimTimerFinished, ElimDelay);
}

void AFPMonster::MulticastElim_Implementation()
{
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 50.0f);
	}

	StartDissolve();

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFPMonster::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AFPMonster::ReceiveDamage);
	}

	if (FPMonsterController)
	{
		FPMonsterController->RunAI();
	}

	UFPMonsterAnimInstance* animInstance = Cast<UFPMonsterAnimInstance>(GetMesh()->GetAnimInstance());
	if (animInstance)
	{
		animInstance->OnMontageEnded.AddDynamic(this, &AFPMonster::OnAttackMontageEnded);
		animInstance->OnAttackHitCheck.AddUObject(this, &AFPMonster::AttackCheck);
	}
}

void AFPMonster::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{	
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	UpdateHUDHealth();

	if (Health <= 0.0f)
	{
		AFPGameMode* fpGameMode = GetWorld()->GetAuthGameMode<AFPGameMode>();
		if (fpGameMode)
		{
			AFPPlayerController* attackerController = Cast<AFPPlayerController>(InstigatorController);
			fpGameMode->MonsterEliminated(this, attackerController);
		}
	}
}

void AFPMonster::OnRep_MonsterHealth()
{
	UpdateHUDHealth();
}

void AFPMonster::ElimTimerFinished()
{
	Reset();
	Destroy();
}

void AFPMonster::UpdateDissolveMaterial(float dissolveValue)
{
	if (DynamicDissolveMaterialInstance)
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), dissolveValue);
}

void AFPMonster::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AFPMonster::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void AFPMonster::UpdateHUDHealth()
{
	FPMonsterController = (FPMonsterController == nullptr) ? Cast<AFPMonsterAIController>(Controller) : FPMonsterController;
	if (FPMonsterController)
	{
		UMonsterOverheadWidget* widget = Cast<UMonsterOverheadWidget>(OverheadWidget->GetUserWidgetObject());
		if (widget)
		{
			const float healthPercent = Health / MaxHealth;
			widget->HealthBar->SetPercent(healthPercent);
		}
	}
}

void AFPMonster::AttackCheck()
{
	FHitResult hitResult;
	FCollisionQueryParams params(NAME_None, false, this);
	bool bResult = GetWorld()->SweepSingleByChannel(
		hitResult,
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * AttackRange,
		FQuat::Identity,
		ECC_SkeletalMesh,
		FCollisionShape::MakeSphere(AttackRadius),
		params
	);

	//FVector traceVec = GetActorForwardVector() * AttackRange;
	//FVector center = GetActorLocation() + traceVec * 0.5f;
	//FColor drawColor = bResult ? FColor::Green : FColor::Red;

	//DrawDebugSphere(
	//	GetWorld(),
	//	center,
	//	AttackRadius,
	//	16,
	//	drawColor,
	//	false,
	//	1.0f
	//);

	if (bResult)
	{
		if (hitResult.GetActor())
		{
			UGameplayStatics::ApplyDamage(hitResult.GetActor(), AttackDamage, FPMonsterController, this, UDamageType::StaticClass());
		}
	}
}

void AFPMonster::MulticastAttack_Implementation()
{
	bAttack = true;
	PlayAttackMontage();
}

void AFPMonster::PlayAttackMontage()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && AttackMontage)
	{
		animInstance->Montage_Play(AttackMontage);

		int8 select = FMath::RandRange(0, AttackMontageNames.Num() - 1);
		animInstance->Montage_JumpToSection(AttackMontageNames[select]);
	}
}

void AFPMonster::OnAttackMontageEnded(UAnimMontage* montage, bool bInterrupted)
{
	bAttack = false;
	OnAttackEnd.Broadcast();
}

