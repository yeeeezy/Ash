#include "Abilities/GA_MeleeAttack.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "GameFramework/Character.h"

UGA_MeleeAttack::UGA_MeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 1. 基础检查
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	CurrentComboStep = 0;
	PlayComboStep(CurrentComboStep);

	// 2. 开启常驻输入监听
	UAbilityTask_WaitInputPress* InputTask = UAbilityTask_WaitInputPress::WaitInputPress(this);
	if (InputTask)
	{
		InputTask->OnPress.AddDynamic(this, &UGA_MeleeAttack::OnInputReceived);
		InputTask->ReadyForActivation();
	}
}

void UGA_MeleeAttack::PlayComboStep(int32 Step)
{


		
	if (ComboMontages.IsValidIndex(Step) && ComboMontages[Step])
	{
		
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, ComboMontages[Step], 1.0f);

		if (MontageTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &UGA_MeleeAttack::OnAttackFinished);
			MontageTask->OnInterrupted.AddDynamic(this, &UGA_MeleeAttack::OnAttackFinished);
			MontageTask->OnCancelled.AddDynamic(this, &UGA_MeleeAttack::OnAttackFinished);
			MontageTask->ReadyForActivation();
		}
	}
	else
	{
		OnAttackFinished();
	}
}



void UGA_MeleeAttack::OnInputReceived(float TimeWaited)
{
	FGameplayTag WindowTag = FGameplayTag::RequestGameplayTag(FName("State.ComboWindow"));
	if (!GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(WindowTag))
	{
		UAbilityTask_WaitInputPress* RetryTask = UAbilityTask_WaitInputPress::WaitInputPress(this);
		if (RetryTask)
		{
			RetryTask->OnPress.AddDynamic(this, &UGA_MeleeAttack::OnInputReceived);
			RetryTask->ReadyForActivation();
		}
		return;
 
	}

	if (CurrentComboStep < ComboMontages.Num() - 1)
	{
		CurrentComboStep++;
       
		// --- 修改点 1：设置跳转标记 ---
		bIsComboTransition = true; 
       
		// 停止旧动画（这会触发上一个任务的 OnInterrupted）
		GetAbilitySystemComponentFromActorInfo()->CurrentMontageStop();
       
		// 重置标记，准备播放新一段
		bIsComboTransition = false; 

		PlayComboStep(CurrentComboStep);



		UAbilityTask_WaitInputPress* NextTask = UAbilityTask_WaitInputPress::WaitInputPress(this);
		if (NextTask)
		{
			NextTask->OnPress.AddDynamic(this, &UGA_MeleeAttack::OnInputReceived);
			NextTask->ReadyForActivation();
		}
	}
}

void UGA_MeleeAttack::OnAttackFinished()
{
	// --- 修改点 3：判断是否是连招跳转 ---
	if (bIsComboTransition)
	{
		// 如果是因为连招跳转触发的中断，直接返回，不结束能力
		return; 
	}
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("endAbility"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}