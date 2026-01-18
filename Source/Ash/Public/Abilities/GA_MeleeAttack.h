#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MeleeAttack.generated.h"

UCLASS()
class ASH_API UGA_MeleeAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_MeleeAttack();

	/** 存放三段连招的蒙太奇数组，在蓝图中按顺序放入 3 个动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<TObjectPtr<UAnimMontage>> ComboMontages;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** 必须带 float 参数以匹配 WaitInputPress 委托 */
	UFUNCTION()
	void OnInputReceived(float TimeWaited);

	/** 动画播放完成后的回调 */
	UFUNCTION()
	void OnAttackFinished();

	/** 播放指定索引的连招动画 */
	void PlayComboStep(int32 Step);

	// 标记当前是否正在从一段攻击切换到下一段
	bool bIsComboTransition = false;

	int32 CurrentComboStep = 0;

	/** 1. 核心回调函数：当 ANS 发来信号时，这个函数会被调用 */
	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);

	/** 2. 击中黑名单：确保这一刀挥过去，同一个怪只会被扣一次血 */
	UPROPERTY()
	TArray<AActor*> HitActors;

	/** 3. 伤害效果：在蓝图 Details 面板里指定具体的 GameplayEffect (如 GE_Damage) */
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float BaseDamage = 10000.0f; // 基础伤害设为 20
};