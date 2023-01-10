
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "FPMonster.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnAttackEndDelegate);

UCLASS()
class PROJECTFP_API AFPMonster : public ACharacter
{
	GENERATED_BODY()

public:
	AFPMonster();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	
private:
	void AttackCheck();
	void UpdateHUDHealth();

	UFUNCTION()
	void OnRep_MonsterHealth();

	void ElimTimerFinished();

	UFUNCTION()
	void UpdateDissolveMaterial(float dissolveValue);
	void StartDissolve();

public:
	class AFPMonsterAIController* FPMonsterController;

	TArray<FName> AttackMontageNames;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* AttackMontage;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAttack();

	void PlayAttackMontage();

	FOnAttackEndDelegate OnAttackEnd;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* montage, bool bInterrupted);

	class AFPCharacter* TargetCharacter{ nullptr };

private:
	UPROPERTY(VisibleAnywhere, Category = "Monster Properties")
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(EditAnywhere, Category = "Monster State")
	float MaxHealth{ 100.0f };

	UPROPERTY(ReplicatedUsing = OnRep_MonsterHealth, VisibleAnywhere, Category = "Monster State")
	float Health{ 100.0f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float AttackRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float AttackDamage{ 20.0f };

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay{ 3.0f };

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	bool bAttack{ false };

public:
	FORCEINLINE bool IsAttack() const { return bAttack; }
};
