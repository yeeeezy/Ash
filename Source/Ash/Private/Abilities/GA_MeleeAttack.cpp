#include "Abilities/GA_MeleeAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayTasksComponent.h"
#include "GameplayTask.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"


#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

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

	// 【1. 准备】每一刀开始前，把名单清空
	HitActors.Empty();

	// 【2. 支起天线】创建一个异步任务，专门监听 Event.Melee.Hit 频道
	UAbilityTask_WaitGameplayEvent* WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, 
		FGameplayTag::RequestGameplayTag(FName("Event.Melee.Hit"))
	);

	// 【3. 连线】告诉任务：一旦收到信号，就去运行 OnHitEventReceived 函数
	WaitTask->EventReceived.AddDynamic(this, &UGA_MeleeAttack::OnHitEventReceived);
    
	// 【4. 启动任务】
	// 正确写法
	WaitTask->ReadyForActivation();
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

	HitActors.Empty();
		
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

void UGA_MeleeAttack::OnHitEventReceived(FGameplayEventData Payload)
{
    // 1. 获取目标 Actor
    AActor* Victim = const_cast<AActor*>(Cast<AActor>(Payload.Target));

    // 2. 基础检查：防空、防自残、防重复攻击
    if (!Victim || Victim == GetAvatarActorFromActorInfo() || HitActors.Contains(Victim))
    {
       return;
    }

    // 3. 尝试直接获取受害者的 ASC 组件
    UAbilitySystemComponent* TargetASC = Victim->FindComponentByClass<UAbilitySystemComponent>();
    
    // 如果受害者身上有 ASC，我们才继续执行伤害逻辑
    if (TargetASC)
    {


       // 4. 正式把受害者加入命中名单
       HitActors.Add(Victim);

       // 5. 检查伤害类是否已经在蓝图中配置
       if (DamageEffectClass)
       {
          // 创建 GE 句柄 (Spec)
          FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
        
          if (SpecHandle.IsValid())
          {
             // 获取从 ANS 传过来的倍率 (EventMagnitude)
             float MultiplierFromANS = Payload.EventMagnitude; 

             // 计算总伤害：基础伤害值 * 动画传来的倍率
             // 注意：BaseDamage 需要在你的 .h 文件中初始化（例如 float BaseDamage = 20.f;）
             float DamageToApply = BaseDamage * MultiplierFromANS;

             // 获取对应的 DataTag
             FGameplayTag DataTag = FGameplayTag::RequestGameplayTag(FName("Data.FinalDamage"));

             // 核心步骤：将计算好的伤害值塞进 Spec，并贴上标签
             // 这里使用 -DamageToApply，因为 Health + (-Value) 才是扣血
             SpecHandle.Data.Get()->SetSetByCallerMagnitude(DataTag, -DamageToApply);

             // 6. 最终应用伤害
             ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle, Payload.TargetData);
          }
       }
       else
       {
          // 调试提醒：如果你忘了在 GA 蓝图里选 GE_Damage，会走这里
          if (GEngine)
          {
             GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error: DamageEffectClass is NULL!"));
          }
       }
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