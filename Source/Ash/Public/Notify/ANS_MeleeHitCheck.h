#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "ANS_MeleeHitCheck.generated.h"

UCLASS()
class ASH_API UANS_MeleeHitCheck : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_MeleeHitCheck();

	// 检测半径（球体大小）
	UPROPERTY(EditAnywhere, Category = "Combat")
	float TraceRadius = 20.0f;

	// 剑柄位置的 Socket 名字
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName StartSocketName = TEXT("Socket_Sword_Start");

	// 剑尖位置的 Socket 名字
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName EndSocketName = TEXT("Socket_Sword_End");

	// 发送给 GA 的标签，例如 Event.Melee.Hit
	UPROPERTY(EditAnywhere, Category = "Combat")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DamageMultiplier = 1.0f; // 默认倍率为 1
	
	// 每一帧都会执行的检测函数
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};