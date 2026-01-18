#include "Abilities/GA_Character_Death.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"


UGA_Character_Death::UGA_Character_Death()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // 绑定死亡事件 Tag
    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Character.Dead"));
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(TriggerData);

    // 死亡时拥有的标签
    ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
}

void UGA_Character_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // 1. 先获取当前角色 (Player)
    APlayerCharacter* Character = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
    
    if (Character)
    {
        Character->SetIsDead(true);

        // 2. 获取动画实例并直接修改它的变量
        UAshAnimInstance* AnimInst = Cast<UAshAnimInstance>(Character->GetMesh()->GetAnimInstance());
        if (AnimInst)
        {
            // 你需要在 AshAnimInstance 里把 bIsDead 设为 public 或者给个 SetIsDead
            AnimInst->bIsDead = true; 
        }
        // 停止当前所有动画并禁用移动
        Character->StopAnimMontage();
        Character->GetCharacterMovement()->DisableMovement();
        Character->GetCharacterMovement()->StopMovementImmediately();
        
        // 忽略所有碰撞（让 Boss 砍不到尸体，且尸体不卡位）
        Character->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

        // 2. 找到所有 AI，让他们“忘了”这个已经死掉的角色
        TArray<AActor*> FoundAI;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAIController::StaticClass(), FoundAI);

        for (AActor* Actor : FoundAI)
        {
            AAIController* AIC = Cast<AAIController>(Actor);
            if (AIC && AIC->GetBlackboardComponent())
            {
                // 获取 AI 黑板上的目标
                UObject* CurrentTarget = AIC->GetBlackboardComponent()->GetValueAsObject(FName("TargetActor"));
                
                // 如果 AI 盯着的是刚死的这个角色
                if (CurrentTarget == Character)
                {
                    // 清空目标：这会触发行为树装饰器打断攻击
                    AIC->GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), nullptr);
                    
                    // 强制停止 AI 当前的移动
                    if (AIC)
                    {
                        // 1. 清空黑板目标（触发行为树打断）
                        AIC->GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), nullptr);
    
                        // 2. 停止当前所有移动逻辑
                        AIC->StopMovement(); // 这个函数内部会自动处理 PathFollowingComponent 的中止
                    }
                }
            }
        }
    }

    // --- 随机播放死亡动画逻辑 ---
    UAnimMontage* SelectedMontage = nullptr;

    if (DeathMontages.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, DeathMontages.Num() - 1);
        SelectedMontage = DeathMontages[RandomIndex];
    }

    if (SelectedMontage)
    {
        UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SelectedMontage);
        MontageTask->OnCompleted.AddDynamic(this, &UGA_Character_Death::OnDeathAnimationFinished);
        MontageTask->OnInterrupted.AddDynamic(this, &UGA_Character_Death::OnDeathAnimationFinished);
        MontageTask->ReadyForActivation();
    }
    else
    {
        // 如果没动画，直接结束能力
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

void UGA_Character_Death::OnDeathAnimationFinished()
{
  
    
    // 4. 正式结束能力
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}