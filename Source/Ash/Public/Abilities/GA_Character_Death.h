#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Character_Death.generated.h"

UCLASS()
class ASH_API UGA_Character_Death : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Character_Death();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
protected:
	// 改为数组，并在蓝图里填充这 8 个动画
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death")
	TArray<TObjectPtr<UAnimMontage>> DeathMontages;

	// 内部引用的函数不需要改动
	UFUNCTION()
	void OnDeathAnimationFinished();

};