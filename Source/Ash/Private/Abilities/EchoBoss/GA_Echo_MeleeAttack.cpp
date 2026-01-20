#include "Abilities/EchoBoss/GA_Echo_MeleeAttack.h" 
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UGA_Echo_MeleeAttack::UGA_Echo_MeleeAttack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Echo_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
       EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
       return;
    }

    if (!AttackMontage)
    {
       EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
       return;
    }

    // 1. 持续监听信号
    FGameplayTag HitEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Boss.Melee.Hit"));
    UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, HitEventTag);
    WaitEventTask->EventReceived.AddDynamic(this, &UGA_Echo_MeleeAttack::HandleHitEvent);
    WaitEventTask->ReadyForActivation();

    // 2. 播放动作
    UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, 1.f, NAME_None, false);
    Task->OnCompleted.AddDynamic(this, &UGA_Echo_MeleeAttack::OnAttackFinished);
    Task->OnInterrupted.AddDynamic(this, &UGA_Echo_MeleeAttack::OnAttackCancelled);
    Task->OnCancelled.AddDynamic(this, &UGA_Echo_MeleeAttack::OnAttackCancelled);
    Task->ReadyForActivation();
}

void UGA_Echo_MeleeAttack::HandleHitEvent(FGameplayEventData Payload)
{
    if (DamageGEClass)
    {
       FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageGEClass, GetAbilityLevel());
       if (SpecHandle.IsValid())
       {
          float Multiplier = (Payload.EventMagnitude != 0.f) ? Payload.EventMagnitude : 1.0f; 
          float TotalDamage = BaseDamage * Multiplier;

          FGameplayTag DataTag = FGameplayTag::RequestGameplayTag(FName("Data.FinalDamage"));
          SpecHandle.Data.Get()->SetSetByCallerMagnitude(DataTag, -TotalDamage);

          // 标准应用方法：对 TargetData 指向的所有目标应用
          ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle, Payload.TargetData);

          // if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("GA Dealt %.2f Damage!"), TotalDamage));
       }
    }
}

void UGA_Echo_MeleeAttack::OnAttackFinished() { EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false); }

void UGA_Echo_MeleeAttack::OnAttackCancelled() 
{ 
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("GA CANCELLED/INTERRUPTED!"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true); 
}