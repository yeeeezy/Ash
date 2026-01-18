#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Character/BaseCharacter.h" // 或者是你的 AMyBaseCharacter
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "BaseBoss.generated.h"

UCLASS()
class ASH_API ABaseBoss : public ABaseCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// 构造函数：处理组件创建和体型缩放
	ABaseBoss();

	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<UGameplayEffect> DamageGEClass;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	// 一个可以被蓝图调用的测试函数
	UFUNCTION(BlueprintCallable, Category = "GAS")
	void ApplyDamageToSelf();

	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<class UGameplayEffect> DefaultAttributeGE;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	class UBehaviorTree* BehaviorTree;
	
protected:
	// BeginPlay：处理运行时骨骼同步
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* LockOnZone;

	// 声明 Overlap 回调
	UFUNCTION()
	void OnLockZoneOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 1. 声明 AbilitySystemComponent (ASC)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAbilitySystemComponent> AbilitySystemComponent;

	// 2. 声明之前写好的 AttributeSetBase
	UPROPERTY()
	TObjectPtr<class UBaseAttributeSet> AttributeSet;

	// 3. 声明健康变化的回调函数
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);

	// 允许在蓝图里配置初始能力（比如近战、俯冲等）
	UPROPERTY(EditAnywhere, Category = "GAS|Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;
};