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
};