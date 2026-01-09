#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

/** 前向声明，减少头文件依赖以加快编译速度 */
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class ASH_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	/** 摄像机摇臂 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ash|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** 摄像机 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ash|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	/** 增强输入配置 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ash|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ash|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ash|Input")
	TObjectPtr<UInputAction> LookAction;

	/** 奔跑输入动作 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ash|Input")
	TObjectPtr<UInputAction> SprintAction;

	/** 速度变量 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ash|Attributes")
	float WalkSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ash|Attributes")
	float SprintSpeed = 600.f;

	/** 奔跑逻辑函数 */
	void OnSprintStarted();
	void OnSprintEnded();

	/** 输入回调 */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};