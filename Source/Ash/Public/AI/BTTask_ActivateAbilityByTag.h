#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h" // 必须有，否则找不到 FAbilityEndedData
#include "BTTask_ActivateAbilityByTag.generated.h"

UCLASS()
class ASH_API UBTTask_ActivateAbilityByTag : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ActivateAbilityByTag();

	// 重写这两个核心函数
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag AbilityTag;

	// 技能结束回调
	void OnAbilityEnded(const FAbilityEndedData& AbilityEndedData);

	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;
};