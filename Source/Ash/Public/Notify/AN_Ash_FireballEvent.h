#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_Ash_FireballEvent.generated.h"

UCLASS()
class ASH_API UAN_Ash_FireballEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 在蒙太奇里可以直接选标签
	UPROPERTY(EditAnywhere, Category = "GAS")
	FGameplayTag EventTag;

	// 构造函数设置默认标签
	UAN_Ash_FireballEvent();

	// 核心触发逻辑
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};