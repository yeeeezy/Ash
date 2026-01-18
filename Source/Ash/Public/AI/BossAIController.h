// BossAIController.h
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BossAIController.generated.h"

UCLASS()
class ASH_API ABossAIController : public AAIController
{
	GENERATED_BODY()

public:
	ABossAIController();


	void OnPlayerEnteredArena(AActor* Player);

protected:
	// 当 AI 附身到 Boss 身上时触发
	virtual void OnPossess(APawn* InPawn) override;

	// 两个核心组件的指针
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UBlackboardComponent* BlackboardComponent;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	class UBehaviorTree* BehaviorTree;
};