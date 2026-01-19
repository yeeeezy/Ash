#include "Abilities/EchoBoss/GA_Boss_FireballStorm.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_Boss_FireballStorm::UGA_Boss_FireballStorm()
{
    // 设置能力标签
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Boss.FireballStorm")));
}

void UGA_Boss_FireballStorm::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) return;

    ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Boss) return;

    // --- ash 核心参数 ---
    float LaunchVelocity = 1300.f;  // 上升冲力
    float RiseTime = 0.8f;         // 建议 0.8s-1s，3s 真的太高了
    float HoverWait = 0.2f;        // 停稳后的静止时间

    // 1. 【起飞】：纯物理上升
    Boss->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
    Boss->LaunchCharacter(FVector(0.f, 0.f, LaunchVelocity), false, true);

    // 2. 【准备监听】：监听动画中的火球发射事件
    UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, FGameplayTag::RequestGameplayTag(FName("Event.Boss.SpawnFireball")), nullptr, false, true);
    WaitEventTask->EventReceived.AddDynamic(this, &UGA_Boss_FireballStorm::OnFireballEventReceived);
    WaitEventTask->ReadyForActivation();

    // 3. 【时序控制】：等待上升结束后刹车并播蒙太奇
    FTimerHandle PlayMontageHandle;
    GetWorld()->GetTimerManager().SetTimer(PlayMontageHandle, [this, Boss, HoverWait]()
    {
        if (Boss)
        {
            // 刹车停稳
            Boss->GetCharacterMovement()->StopMovementImmediately();
            Boss->GetCharacterMovement()->Velocity = FVector::ZeroVector;

            // 停稳 0.2s 后开始播放蒙太奇
            FTimerHandle HoverTimerHandle;
            GetWorld()->GetTimerManager().SetTimer(HoverTimerHandle, [this]()
            {
                UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                    this, TEXT("CastFireball"), CastMontage);
                
                // 蒙太奇播完直接调用 EndAbility（这会自动触发落地逻辑）
                MontageTask->OnCompleted.AddDynamic(this, &UGA_Boss_FireballStorm::K2_EndAbility);
                MontageTask->OnInterrupted.AddDynamic(this, &UGA_Boss_FireballStorm::K2_EndAbility);
                MontageTask->ReadyForActivation();
            }, HoverWait, false);
        }
    }, RiseTime, false);
}

void UGA_Boss_FireballStorm::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (Boss)
    {
        // 结束时切回行走，Boss 掉回地面
        Boss->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }
    
    // 调用父类 EndAbility，此时行为树的 Task 节点会返回 Success
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Boss_FireballStorm::OnFireballEventReceived(FGameplayEventData Payload)
{
    // 重置计数并启动 0.1s 循环计时器
    FireballsShotCount = 0;
    
    // 立即发射第一发，然后开启计时器发剩下的
    ExecuteSingleFireball();

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FireballTimerHandle, 
            this, 
            &UGA_Boss_FireballStorm::ExecuteSingleFireball, 
            InterShotDelay, 
            true // 开启循环
        );
    }
    
}



void UGA_Boss_FireballStorm::ExecuteSingleFireball()
{
    ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Boss || !FireballClass) 
    {
        GetWorld()->GetTimerManager().ClearTimer(FireballTimerHandle);
        return;
    }

    // 1. 计算位置和方向
    FVector MuzzleLoc = Boss->GetMesh()->GetSocketLocation(TEXT("Muzzle_Socket"));
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    
    if (PlayerPawn)
    {
        FVector TargetLoc = PlayerPawn->GetActorLocation();
        // 直线飞行，不拐弯
        FRotator FireRot = (TargetLoc - MuzzleLoc).Rotation();

        // 2. 生成火球
        FActorSpawnParameters Params;
        Params.Owner = Boss;
        Params.Instigator = Boss;
        GetWorld()->SpawnActor<AActor>(FireballClass, MuzzleLoc, FireRot, Params);
        
        UE_LOG(LogTemp, Log, TEXT("ash: Fireball %d Shot!"), FireballsShotCount + 1);
    }

    // 3. 计数与清理
    FireballsShotCount++;
    if (FireballsShotCount >= TotalFireballsToShoot)
    {
        GetWorld()->GetTimerManager().ClearTimer(FireballTimerHandle);
    }
}
