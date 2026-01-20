#include "AI/BTService_UpdateDistance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_UpdateDistance::UBTService_UpdateDistance()
{
	NodeName = "Update Distance to Target";
	// 允许执行 Tick
	bNotifyTick = true;
	// 设置默认每 0.1s 执行一次，不需要每帧执行，省开销
	Interval = 0.1f;
}

void UBTService_UpdateDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIC = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (AIC && BB)
	{
		APawn* Boss = AIC->GetPawn();
		// 1. 获取目标 (玩家)
		AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));

		if (Boss && Target)
		{
			// 2. 计算距离
			float Dist = FVector::Dist(Boss->GetActorLocation(), Target->GetActorLocation());
            
			// 3. 更新黑板 (这是行为树判断的依据)
			BB->SetValueAsFloat(DistanceKey.SelectedKeyName, Dist);

			// 4. 屏幕输出：蓝色代表正常运行
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Cyan, 
					FString::Printf(TEXT("ash: 目标: %s | 当前距离: %.2f"), *Target->GetName(), Dist));
			}
		}
		else
		{
			// 5. 错误提示：红色代表没找到玩家
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(2, 0.1f, FColor::Red, TEXT("ash: 错误 - 无法获取 TargetActor，请检查黑板 Key 绑定"));
			}
		}
	}
}