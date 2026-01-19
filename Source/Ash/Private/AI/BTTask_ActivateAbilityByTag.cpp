#include "AI/BTTask_ActivateAbilityByTag.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

UBTTask_ActivateAbilityByTag::UBTTask_ActivateAbilityByTag()
{
	NodeName = "Activate Ability By Tag";
}

EBTNodeResult::Type UBTTask_ActivateAbilityByTag::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ash"));
	AAIController* MyController = OwnerComp.GetAIOwner();
	if (!MyController) return EBTNodeResult::Failed;

	APawn* MyPawn = MyController->GetPawn();
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(MyPawn);
	
	if (ASI)
	{
		UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
		// 通过标签尝试激活火球技能
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(AbilityTag);

		if (ASC && ASC->TryActivateAbilitiesByTag(TagContainer))
		{
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}