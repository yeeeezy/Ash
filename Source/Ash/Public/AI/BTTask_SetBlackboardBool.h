#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetBlackboardBool.generated.h"

UCLASS()
class ASH_API UBTTask_SetBlackboardBool : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetBlackboardBool();

	// ash: 只需要重写执行逻辑，因为这是一个瞬时完成的 Task
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// 在编辑器里选择要修改哪个黑板变量
	UPROPERTY(EditAnywhere, Category = "ash|Blackboard")
	FBlackboardKeySelector BoolKey;

	// 想要设置成 True 还是 False
	UPROPERTY(EditAnywhere, Category = "ash|Blackboard")
	bool bValueToSet = false;
};