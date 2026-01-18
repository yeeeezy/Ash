// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Boss_Death.generated.h"

UCLASS()
class ASH_API UGA_Boss_Death : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Boss_Death();

	// 当能力被激活时触发
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 蒙太奇结束的回调
	UFUNCTION()
	void OnDeathMontageFinished();

	UPROPERTY(EditAnywhere, Category = "Death")
	UAnimMontage* DeathMontage;
};