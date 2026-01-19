#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_SendGameplayEvent.generated.h"

UCLASS()
class ASH_API UAN_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 允许在动画编辑器里设置我们要发送的 Tag
	UPROPERTY(EditAnywhere, Category = "GAS")
	FGameplayTag EventTag;

	// 重写 Notify 函数
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};