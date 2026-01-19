#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Boss_FireballStorm.generated.h"

UCLASS()
class ASH_API UGA_Boss_FireballStorm : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Boss_FireballStorm();

	// 基础属性
	UPROPERTY(EditDefaultsOnly, Category = "Fireball")
	TSubclassOf<AActor> FireballClass;

	UPROPERTY(EditDefaultsOnly, Category = "Fireball")
	UAnimMontage* CastMontage;

protected:
	// 能力激活入口
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 当接收到 Gameplay Event (即蒙太奇里的 AN_GameplayEvent) 时触发
	UFUNCTION()
	void OnFireballEventReceived(FGameplayEventData Payload);

	// 能力结束清理
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;


	// 内部发射函数
	void ExecuteSingleFireball();

	// 连发逻辑变量
	int32 FireballsShotCount = 0;
	const int32 TotalFireballsToShoot = 3;
	const float InterShotDelay = 0.1f;
	FTimerHandle FireballTimerHandle;

};