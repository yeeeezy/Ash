#include "AI/BTTask_SetBlackboardBool.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetBlackboardBool::UBTTask_SetBlackboardBool()
{
	NodeName = "Set Blackboard Bool";
}

EBTNodeResult::Type UBTTask_SetBlackboardBool::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	
	if (MyBlackboard)
	{
		// 直接修改黑板值
		MyBlackboard->SetValueAsBool(BoolKey.SelectedKeyName, bValueToSet);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}