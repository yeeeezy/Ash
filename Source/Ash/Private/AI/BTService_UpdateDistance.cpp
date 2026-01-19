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

	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Update Distance");
	AAIController* AIC = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (AIC && BB)
	{
		APawn* Boss = AIC->GetPawn();
		// 从黑板获取目标 Actor (玩家)
		AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));

		if (Boss && Target)
		{
			// 计算距离
			float Dist = FVector::Dist(Boss->GetActorLocation(), Target->GetActorLocation());
            
			// 更新黑板变量
			BB->SetValueAsFloat(DistanceKey.SelectedKeyName, Dist);

			// 调试语句
			// GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Cyan, FString::Printf(TEXT("ash: Current Distance: %f"), Dist));
		}
	}
}