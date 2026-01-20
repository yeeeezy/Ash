#include "AI/BTService_RandomSelect.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_RandomSelect::UBTService_RandomSelect()
{
	NodeName = "ash Random Selection Service";
	// 禁用周期性 Tick，我们只需要在进入时算一次，节省开销
	bNotifyBecomeRelevant = true;
	bNotifyTick = false; 
}

void UBTService_RandomSelect::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	if (MyBlackboard)
	{
		// 生成随机数并存入黑板
		int32 RandomVal = FMath::RandRange(0, MaxIndex);
		MyBlackboard->SetValueAsInt(ChoiceIndexKey.SelectedKeyName, RandomVal);
	}
}