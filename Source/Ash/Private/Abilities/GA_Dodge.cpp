#include "Abilities/GA_Dodge.h"
#include "Character/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AshAnimInstance.h"

UGA_Dodge::UGA_Dodge()
{
    // 设置能力标签，防止在被控制或死亡时释放
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Action.Dodge")));
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    APlayerCharacter* Character = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!Character) return;

    UAnimMontage* SelectedMontage = nullptr;
    FVector LaunchDirection = FVector::ZeroVector;

    // 1. 获取玩家当前的输入加速度方向（WASD 的最终合力方向）
    FVector CurrentInput = Character->GetCharacterMovement()->GetCurrentAcceleration().GetSafeNormal();

    // 2. 如果没有按任何方向键（原地），默认执行后撤步/后滚
    if (CurrentInput.IsNearlyZero())
    {
        SelectedMontage = DodgeBackwardMontage;
        LaunchDirection = -Character->GetActorRightVector();
    }
    else
    {
        // 3. 将全局输入向量转换为角色的局部空间
        // 这能让我们知道输入是相对于角色的前、后、左还是右
        FVector LocalInput = Character->GetActorQuat().UnrotateVector(CurrentInput);

        // 4. 根据 LocalInput 的 X 和 Y 值判断具体方向
        // X > 0.5 是前, X < -0.5 是后, Y > 0.5 是右, Y < -0.5 是左
        if (LocalInput.X > 0.5f) 
        {
            SelectedMontage = DodgeForwardMontage;
            LaunchDirection = Character->GetActorForwardVector();
        }
        else if (LocalInput.X < -0.5f)
        {
            SelectedMontage = DodgeBackwardMontage;
            LaunchDirection = -Character->GetActorForwardVector();
        }
        else if (LocalInput.Y > 0.5f)
        {
            SelectedMontage = DodgeRightMontage;
            LaunchDirection = Character->GetActorRightVector();
        }
        else // LocalInput.Y < -0.5f
        {
            SelectedMontage = DodgeLeftMontage;
            LaunchDirection = -Character->GetActorRightVector();
        }
    }

    // 5. 执行 Launch
    Character->LaunchCharacter(LaunchDirection * DodgeImpulse, true, true);

    // 6. 播放对应的动画
    if (SelectedMontage)
    {
        Character->PlayAnimMontage(SelectedMontage);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}