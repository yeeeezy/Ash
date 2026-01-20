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
    // if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Magenta, TEXT("activate"));

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
       FinishAbility();
       return;
    }

    // 1. 【核心】开启全局监听
    FGameplayTag HitTag = FGameplayTag::RequestGameplayTag(FName("Event.Boss.Melee.Hit"));
    UAbilityTask_WaitGameplayEvent* HitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, HitTag, nullptr, false, true);
    
    if (HitTask)
    {
        HitTask->EventReceived.AddDynamic(this, &UGA_BlinkAttack::OnMeleeHitReceived);
        HitTask->ReadyForActivation();
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
        StartTask->ReadyForActivation();
    }
}

void UGA_BlinkAttack::OnDashEventReceived(FGameplayEventData Payload)
{
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
    UAbilityTask_PlayMontageAndWait* AttackTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("AttackTask"), DashHitMontage, 1.0f, FName("DashHit"));
    
    if (AttackTask)
    {
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
    if (DamageEffectClass)
    {
        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
       
        if (SpecHandle.IsValid())
        {
            float Multiplier = (Payload.EventMagnitude != 0.f) ? Payload.EventMagnitude : 1.0f; 
            float TotalDamage = BaseDamage * Multiplier;

            FGameplayTag DataTag = FGameplayTag::RequestGameplayTag(FName("Data.FinalDamage"));
            SpecHandle.Data.Get()->SetSetByCallerMagnitude(DataTag, -TotalDamage);

            ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle, Payload.TargetData);
        }
    }
}

void UGA_BlinkAttack::FinishAbility()
{
    // if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Magenta, TEXT("ash: BlinkAttack TRULY ENDED"));
    if (GetAvatarActorFromActorInfo())
    {
        if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
        {
            Character->GetMesh()->GetAnimInstance()->Montage_Stop(0.2f);
        }
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UGA_BlinkAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}