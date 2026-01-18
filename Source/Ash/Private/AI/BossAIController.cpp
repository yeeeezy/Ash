// BossAIController.cpp
#include "AI/BossAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Character/Boss/BaseBoss.h" // 包含你的 Boss 类

ABossAIController::ABossAIController()
{
	// 1. 初始化黑板和行为树组件
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
}

void ABossAIController::OnPlayerEnteredArena(AActor* Player)
{
	
	if (BlackboardComponent && Player)
	{
		// 直接告诉黑板：目标就是这个玩家
		BlackboardComponent->SetValueAsObject(FName("TargetActor"), Player);
        
		
	}
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 2. 获取 Boss 身上关联的行为树资源
	if (ABaseBoss* Boss = Cast<ABaseBoss>(InPawn))
	{
		// 确保 Boss 蓝图里已经选好了 BehaviorTree
		if (Boss->BehaviorTree && Boss->BehaviorTree->BlackboardAsset)
		{
			// 3. 初始化黑板数据
			BlackboardComponent->InitializeBlackboard(*(Boss->BehaviorTree->BlackboardAsset));
			// 4. 启动行为树
			RunBehaviorTree(Boss->BehaviorTree);
		}
	}
}
