#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "BTTask_EchoAttack.generated.h"

/**
 * 专门用于测试 Echo Boss 激活近战能力的行为树节点
 */
UCLASS()
class ASH_API UBTTask_EchoAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_EchoAttack();

	// 执行任务的核心函数
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	/** 要激活的能力标签，默认设为 Ability.Boss.Melee */
	UPROPERTY(EditAnywhere, Category = "GAS")
	FGameplayTag AbilityTag;
};