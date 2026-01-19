#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "Animation/AshAnimInstance.h"
#include "PlayerCharacter.generated.h"

/** 前向声明 */
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

// --- 在这里定义枚举 ---
UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
    None,
    Confirm,
    Cancel,
    Attack,  // 对应你的轻击
    Dodge    // 对应你的闪避
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, NewHealth, float, MaxHealth);
UCLASS()
class ASH_API APlayerCharacter : public ABaseCharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    APlayerCharacter();
    UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
    FOnHealthChangedSignature OnHealthChanged;
    /** * 执行装备剑的逻辑
     * BlueprintCallable 方便你绑定到按键输入（如键盘 F 键）
     */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EquipSword();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UnequipSword();

    // 供 AnimInstance 调用的挂载函数
    void HandleWeaponAttachment();

    void EnableTargetLock(AActor* NewTarget);
    void DisableTargetLock();

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    AActor* BossActor;

protected:
    

    // 仰角限制：比如最低俯视 -30度，最高仰视 10度
    UPROPERTY(EditAnywhere, Category = "Camera")
    float MinLockPitch = -50.0f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float MaxLockPitch = 20.0f;
    
    UPROPERTY(BlueprintReadOnly)
    AActor* CurrentLockedTarget;

    bool bIsAutoLocking = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    EWeaponState WeaponState;

    // 存储生成的武器实例
    UPROPERTY(VisibleAnywhere, Category = "Combat")
    AActor* EquippedSword;

    // 在蓝图中指定要生成的武器类
    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<AActor> SwordClass;
    
    UPROPERTY(EditAnywhere, Category = "Combat")
    UAnimMontage* EquipMontage;
    
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

    /** 攻击输入动作 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ash|Input")
    TObjectPtr<UInputAction> AttackAction;

    /** 速度变量 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ash|Attributes")
    float WalkSpeed = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ash|Attributes")
    float SprintSpeed = 600.f;

    /** 奔跑逻辑函数 */
    void OnSprintStarted();
    void OnSprintEnded();

    /** 处理攻击输入按下后的逻辑 */
    void Input_AttackPressed();

    /** 输入回调 */
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    
    
    void OnJumpedStarted();
    

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    // ASC 组件是 GAS 的核心
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    class UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    TSubclassOf<class UGameplayAbility> DefaultMeleeAbility;

    // 闪避技能类 (在蓝图里指定 GA_Dodge)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    TSubclassOf<class UGameplayAbility> DodgeAbilityClass;

    // PlayerCharacter.h 中
    UPROPERTY(EditAnywhere, Category = "Abilities")
    TSubclassOf<class UGameplayAbility> DeathAbilityClass;
    
    // 属性集合
    UPROPERTY()
    class UBaseAttributeSet* AttributeSet;

    // 实现接口函数
    virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

protected:



    virtual void Tick(float DeltaTime) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* JumpMontage;

    //监听Health变化
    virtual void HealthChanged(const FOnAttributeChangeData& Data);
};