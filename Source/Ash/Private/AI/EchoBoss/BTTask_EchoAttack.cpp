// 注意：包含路径必须匹配你的 Public 文件夹层级
#include "AI/EchoBoss/BTTask_EchoAttack.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
// 不再需要包含特定的 Boss 头文件，因为我们使用接口
// #include "Character/Boss/BaseBoss.h" 

UBTTask_EchoAttack::UBTTask_EchoAttack()
{
	NodeName = "Echo Melee Attack Task";
    
	// 初始化默认攻击标签
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Boss.Melee"));
}

EBTNodeResult::Type UBTTask_EchoAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 1. 获取 AI 控制器和控制的角色
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	// 2. 通过接口获取 Ability System Component (不再需要 Cast 成特定 Boss 类)
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(ControlledPawn);
    
	if (ASCInterface)
	{
		UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
        
		if (ASC)
		{
			// 3. 尝试激活能力
			bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));

			// 4. 打印调试信息（绿字代表成功，红字代表标签没对上）
			

			// 只有成功激活能力才返回 Succeeded
			return bActivated ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
		}
	}

	// 如果运行到这里，说明 Pawn 没实现接口或没有 ASC
	return EBTNodeResult::Failed;
}