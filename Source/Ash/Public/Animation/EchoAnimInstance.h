#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EchoAnimInstance.generated.h"

/**
 * Echo Boss 的专属动画实例
 */
UCLASS()
class ASH_API UEchoAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// 相当于蓝图中的逻辑初始化
	virtual void NativeInitializeAnimation() override;

	// 相当于蓝图中的每帧更新
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	/** 给蓝图使用的速度值 */
	UPROPERTY(BlueprintReadOnly, Category = "Echo|Movement")
	float Speed;

	/** 是否正在移动 */
	UPROPERTY(BlueprintReadOnly, Category = "Echo|Movement")
	bool bIsMoving;

	/** 引用 Echo 本身 */
	UPROPERTY(BlueprintReadOnly, Category = "Echo|References")
	class ABaseCharacter* EchoCharacter;
};