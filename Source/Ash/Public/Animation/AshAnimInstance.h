#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AshAnimInstance.generated.h"

UCLASS()
class ASH_API UAshAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// 每一帧更新数据，类似 Tick
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 对应之前在蓝图中创建的变量
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsTargeting;

	// 新增：判断是否正在加速（按住 Shift）
	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bIsStopping;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bIsRunning;
};