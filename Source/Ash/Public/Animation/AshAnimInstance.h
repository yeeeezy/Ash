#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AshAnimInstance.generated.h"

// 定义武器状态枚举
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Unequipped    UMETA(DisplayName = "Unequipped"),
	SwordEquipped UMETA(DisplayName = "Sword Equipped")
};

UCLASS()
class ASH_API UAshAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// 每一帧更新数据，类似 Tick
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "State")
	void UpdateWeaponState(EWeaponState NewState) { WeaponState = NewState; }

	UFUNCTION()
	void AnimNotify_AttachWeapon(); // 名称必须与 Notify 名字一致

	// 对应之前在蓝图中创建的变量
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Direction; // 用于驱动混合空间的横轴

	UPROPERTY(BlueprintReadOnly, Category = "States")
	bool bIsDead = false;
	
protected:

	
	
	// 设置为 BlueprintReadOnly，方便在动画蓝图中使用 Blend Poses by Enum
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	EWeaponState WeaponState;




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