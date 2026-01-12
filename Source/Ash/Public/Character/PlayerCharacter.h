#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

/** 前向声明 */
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

    /** 跳跃输入动作 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ash|Input")
    TObjectPtr<UInputAction> JumpAction;

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
    void OnJumpedStarted();
    

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    // 锁定范围
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LockOnRange = 1000.f;

    // 旋转平滑速度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RotationSmoothSpeed = 5.f;

    // 当前锁定的 Boss 目标
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    class AEchoBoss* TargetBoss;

    virtual void Tick(float DeltaTime) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* JumpMontage;
    
};