// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/EchoBoss/GA_Boss_Death.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/AshHUD.h"

UGA_Boss_Death::UGA_Boss_Death()
{
	// 死亡能力通常不需要被其他能力打断
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Boss_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 1. 获取 Boss 引用
	ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Boss) return;

	// 2. 停止 AI 逻辑 (让它变木桩，不再寻找玩家)
	if (AAIController* AIC = Cast<AAIController>(Boss->GetController()))
	{
		if (AIC->GetBrainComponent())
		{
			AIC->GetBrainComponent()->StopLogic("Boss Died");
		}
	}

	// 3. 禁用胶囊体碰撞 (防止挡住玩家走位)
	Boss->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// 4. 播放死亡蒙太奇
	if (DeathMontage)
	{
		// 使用 GAS 标准 Task 播放蒙太奇并等待结束
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, DeathMontage, 1.0f, NAME_None, true);
		
		MontageTask->OnCompleted.AddDynamic(this, &UGA_Boss_Death::OnDeathMontageFinished);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Boss_Death::OnDeathMontageFinished);
		MontageTask->ReadyForActivation();
	}
}

void UGA_Boss_Death::OnDeathMontageFinished()
{
	// 蒙太奇播完了，执行我们在玩家死亡时学的“定格”逻辑
	ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Boss && Boss->GetMesh())
	{
		// 暴力定格最后一帧
		Boss->GetMesh()->bPauseAnims = true;
	}

	

	// 结束能力
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}