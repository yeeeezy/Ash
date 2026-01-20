#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_RandomSelect.generated.h"

UCLASS()
class ASH_API UBTService_RandomSelect : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_RandomSelect();

protected:
	// ash: 重写此函数，它在 Service 变得活跃时（进入节点分支时）执行
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 黑板变量选择器
	UPROPERTY(EditAnywhere, Category = "ash|Random")
	FBlackboardKeySelector ChoiceIndexKey;

	// 随机数最大值 (比如填 2，就会生成 0, 1, 2)
	UPROPERTY(EditAnywhere, Category = "ash|Random")
	int32 MaxIndex = 1;
};