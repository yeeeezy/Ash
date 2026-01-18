#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Dodge.generated.h"

UCLASS()
class ASH_API UGA_Dodge : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Dodge();

	/** 能力激活时的核心逻辑 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** 闪避的力度 */
	UPROPERTY(EditAnywhere, Category = "Dodge")
	float DodgeImpulse = 1500.f;

	/** 闪避时使用的蒙太奇 (可以在蓝图里配) */
	UPROPERTY(EditAnywhere, Category = "Dodge")
	UAnimMontage* DodgeLeftMontage;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	UAnimMontage* DodgeRightMontage;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	UAnimMontage* DodgeForwardMontage;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	UAnimMontage* DodgeBackwardMontage;
};