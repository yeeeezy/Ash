#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_BlinkAttack.generated.h"

UCLASS()
class ASH_API UGA_BlinkAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_BlinkAttack();

protected:
	// ash: 核心激活函数
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// --- 回调函数 ---
	UFUNCTION()
	void OnDashEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMeleeHitReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnDashFinished();

	// ash: 手动结束能力包装，确保同步行为树
	UFUNCTION()
	void FinishAbility();

	// --- 资源变量 ---
	UPROPERTY(EditAnywhere, Category = "DashAttack")
	UAnimMontage* DashStartMontage;

	UPROPERTY(EditAnywhere, Category = "DashAttack")
	UAnimMontage* DashHitMontage;

	UPROPERTY(EditAnywhere, Category = "DashAttack")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	// 伤害去重列表
	UPROPERTY()
	TArray<AActor*> HitActorsInThisAbility;
};