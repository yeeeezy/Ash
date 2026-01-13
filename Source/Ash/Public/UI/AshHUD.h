#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AshHUD.generated.h"

/** 前向声明，减少头文件依赖 */
class UPlayerHUDWidget;

UCLASS()
class ASH_API AAshHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** * 在编辑器里指定的 UI 蓝图类 (WBP_PlayerHUD)
	 */
	UPROPERTY(EditAnywhere, Category = "GAS|UI")
	TSubclassOf<UPlayerHUDWidget> PlayerHUDClass;

	/** * 运行时生成的 UI 实例
	 */
	UPROPERTY()
	TObjectPtr<UPlayerHUDWidget> PlayerHUD;

	/** Boss 血条的蓝图类 (用于在编辑器里指定 WBP_BossHealthBar) */
	UPROPERTY(EditAnywhere, Category = "GAS|UI")
	TSubclassOf<class UPlayerHUDWidget> BossHealthClass;

	/** 运行时生成的 Boss UI 实例指针 */
	UPROPERTY()
	TObjectPtr<class UPlayerHUDWidget> BossHealthWidget;

	/** 初始化 Boss 血条的函数声明 */
	UFUNCTION(BlueprintCallable, Category = "GAS|UI")
	void InitBossHealthBar();

protected:
	/** 游戏开始时创建 UI */
	virtual void BeginPlay() override;
};