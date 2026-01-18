#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Echo_MeleeAttack.generated.h"

/**
 * Echo Boss 近战能力
 */
UCLASS()
class ASH_API UGA_Echo_MeleeAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Echo_MeleeAttack();

	// 在编辑器里指定死神的攻击蒙太奇
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Echo|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 动画播放完毕后的回调
	UFUNCTION()
	void OnAttackFinished();

	// 动画被取消（如被玩家打断/眩晕）的回调
	UFUNCTION()
	void OnAttackCancelled();

	// 在蓝图子类里指定你的 GE_Damage_Basic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Echo|Abilities")
	TSubclassOf<class UGameplayEffect> DamageGEClass;

	// 回调函数必须标记为 UFUNCTION
	UFUNCTION()
	void HandleHitEvent(FGameplayEventData Payload);
};