#pragma once

#include "CoreMinimal.h"
#include "BaseBoss.h"
#include "Character/BaseCharacter.h" // 或者是你的 AMyBaseCharacter
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "EchoBoss.generated.h"

UCLASS()
class ASH_API AEchoBoss : public ABaseBoss
{
	GENERATED_BODY()

public:
	// 构造函数：处理组件创建和体型缩放
	AEchoBoss();
	//
	// UPROPERTY(EditAnywhere, Category = "GAS")
	// TSubclassOf<UGameplayEffect> DamageGEClass;
	//
	// // 一个可以被蓝图调用的测试函数
	// UFUNCTION(BlueprintCallable, Category = "GAS")
	// void ApplyDamageToSelf();

	// UPROPERTY(EditAnywhere, Category = "GAS")
	// TSubclassOf<class UGameplayEffect> DefaultAttributeGE;

protected:
	// BeginPlay：处理运行时骨骼同步
	virtual void BeginPlay() override;

	// 武器组件 (静态网格体)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossAppearance")
	class UStaticMeshComponent* BossWeapon;

	// 骨骼分件槽位
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossAppearance")
	class USkeletalMeshComponent* BossArmor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossAppearance")
	class USkeletalMeshComponent* BossSleeves;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossAppearance")
	class USkeletalMeshComponent* BossShoulders;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossAppearance")
	class USkeletalMeshComponent* BossHelm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossAppearance")
	class USkeletalMeshComponent* BossCap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossAppearance")
	class USkeletalMeshComponent* BossGloves;

	
};