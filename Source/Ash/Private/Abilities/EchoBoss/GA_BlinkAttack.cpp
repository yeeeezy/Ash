#include "Abilities/EchoBoss/GA_BlinkAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"       
#include "GameplayEffect.h"

UGA_BlinkAttack::UGA_BlinkAttack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_BlinkAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
       FinishAbility();
       return;
    }

    // 1. 【核心】开启全局监听
    // 既然两段伤害都在后面，我们从一开始就竖起耳朵听，且 bOnlyTriggerOnce = false (允许听到多次伤害)
    FGameplayTag HitTag = FGameplayTag::RequestGameplayTag(FName("Event.Boss.Melee.Hit"));
    UAbilityTask_WaitGameplayEvent* HitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, HitTag, nullptr, false, true);
    
    if (HitTask)
    {
        HitTask->EventReceived.AddDynamic(this, &UGA_BlinkAttack::OnMeleeHitReceived);
        HitTask->ReadyForActivation();
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("ash: GA Listener START - Ready for Multi-Hit"));
    }

    // 2. 开启位移监听
    FGameplayTag DashTag = FGameplayTag::RequestGameplayTag(FName("Event.Boss.DashStart"));
    UAbilityTask_WaitGameplayEvent* DashEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, DashTag, nullptr, true, true);
    DashEventTask->EventReceived.AddDynamic(this, &UGA_BlinkAttack::OnDashEventReceived);
    DashEventTask->ReadyForActivation();

    // 3. 播放第一段蒙太奇 (DashStart)
    UAbilityTask_PlayMontageAndWait* StartTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("StartTask"), DashStartMontage);
    if (StartTask)
    {
        // ash重点：这里绝对不要绑定 FinishAbility！
        // 因为当第二段动画开始播放时，第一段会被 Interrupted。
        // 如果绑定了，技能就自杀了。我们这里什么都不绑，让位移事件去驱动流程。
        StartTask->ReadyForActivation();
    }
}

void UGA_BlinkAttack::OnDashEventReceived(FGameplayEventData Payload)
{
    // ... 这里保持原本的位移计算逻辑不变 ...
    AActor* Boss = GetAvatarActorFromActorInfo();
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    if (Boss && Player)
    {
        FVector PlayerLoc = Player->GetActorLocation();
        FVector BossLoc = Boss->GetActorLocation();
        FVector DirToPlayer = (PlayerLoc - BossLoc).GetSafeNormal();
        DirToPlayer.Z = 0.f;

        FVector TargetLoc = PlayerLoc - (DirToPlayer * 150.f);
        TargetLoc.Z = BossLoc.Z; 

        FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(BossLoc, PlayerLoc);
        TargetRot.Pitch = 0.f; TargetRot.Roll = 0.f;

        FLatentActionInfo LatentInfo;
        LatentInfo.CallbackTarget = this;
        LatentInfo.ExecutionFunction = FName("OnDashFinished"); 
        LatentInfo.UUID = FMath::Rand(); 
        LatentInfo.Linkage = 0;

        UKismetSystemLibrary::MoveComponentTo(Boss->GetRootComponent(), TargetLoc, TargetRot, true, true, 0.15f, false, EMoveComponentAction::Move, LatentInfo);
    }
}

void UGA_BlinkAttack::OnDashFinished()
{
    // 位移结束，播放第二段包含攻击的动画
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("ash: Playing Second Montage (Attacks inside)"));

    UAbilityTask_PlayMontageAndWait* AttackTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("AttackTask"), DashHitMontage, 1.0f, FName("DashHit"));
    
    if (AttackTask)
    {
        // 只有这第二段动画播完，才是真的结束
        AttackTask->OnCompleted.AddDynamic(this, &UGA_BlinkAttack::FinishAbility);
        AttackTask->OnInterrupted.AddDynamic(this, &UGA_BlinkAttack::FinishAbility);
        AttackTask->OnCancelled.AddDynamic(this, &UGA_BlinkAttack::FinishAbility);
        AttackTask->OnBlendOut.AddDynamic(this, &UGA_BlinkAttack::FinishAbility);
        AttackTask->ReadyForActivation();
    }
    else 
    { 
        FinishAbility(); 
    }
}

void UGA_BlinkAttack::OnMeleeHitReceived(FGameplayEventData Payload)
{
    // ash: 只要进这里，说明伤害触发了
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("ash: Hit Received!"));

    if (DamageEffectClass)
    {
        // 1. 完全复刻 GA_Echo_MeleeAttack 的写法
        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
       
        if (SpecHandle.IsValid())
        {
            float Multiplier = (Payload.EventMagnitude != 0.f) ? Payload.EventMagnitude : 1.0f; 
            float TotalDamage = 15.f * Multiplier; // 你的基础伤害

            FGameplayTag DataTag = FGameplayTag::RequestGameplayTag(FName("Data.FinalDamage"));
            SpecHandle.Data.Get()->SetSetByCallerMagnitude(DataTag, -TotalDamage);

            // 2. 直接应用到 TargetData (自动处理批量目标)
            ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle, Payload.TargetData);

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("ash: Damage Applied: %.1f"), TotalDamage));
        }
    }
}

void UGA_BlinkAttack::FinishAbility()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}