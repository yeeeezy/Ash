#include "AI/BTTask_ActivateAbilityByTag.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

UBTTask_ActivateAbilityByTag::UBTTask_ActivateAbilityByTag()
{
    NodeName = "Activate Ability By Tag (Async)";
    // 删掉报错的 bNotifyAbort = true; 
    // 现代 UE 版本只要重写了 AbortTask 就会自动通知
}

EBTNodeResult::Type UBTTask_ActivateAbilityByTag::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* MyController = OwnerComp.GetAIOwner();
    APawn* MyPawn = MyController ? MyController->GetPawn() : nullptr;
    IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(MyPawn);

    if (ASI && ASI->GetAbilitySystemComponent())
    {
        UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
        CachedOwnerComp = &OwnerComp;

        // 绑定监听
        ASC->OnAbilityEnded.AddUObject(this, &UBTTask_ActivateAbilityByTag::OnAbilityEnded);

        FGameplayTagContainer TagContainer;
        TagContainer.AddTag(AbilityTag);

        if (ASC->TryActivateAbilitiesByTag(TagContainer))
        {
            return EBTNodeResult::InProgress; // 保持节点运行
        }
        
        // 激活失败清理
        ASC->OnAbilityEnded.RemoveAll(this);
    }
    return EBTNodeResult::Failed;
}

// 只要你重写了这个函数，行为树在打断任务时就会执行这里
EBTNodeResult::Type UBTTask_ActivateAbilityByTag::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* MyController = OwnerComp.GetAIOwner();
    if (APawn* MyPawn = MyController ? MyController->GetPawn() : nullptr)
    {
        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(MyPawn))
        {
            if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
            {
                ASC->OnAbilityEnded.RemoveAll(this);
            }
        }
    }
    return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_ActivateAbilityByTag::OnAbilityEnded(const FAbilityEndedData& AbilityEndedData)
{
    if (CachedOwnerComp)
    {
        // ... (清理委托的逻辑同上) ...
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}