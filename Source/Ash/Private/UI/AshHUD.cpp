#include "UI/AshHUD.h"
#include "UI/PlayerHUDWidget.h"
#include "Blueprint/UserWidget.h"


void AAshHUD::InitBossHealthBar()
{
	// 检查是否已经指定了蓝图类，且实例还没创建
	if (BossHealthClass && !BossHealthWidget)
	{
		BossHealthWidget = CreateWidget<UPlayerHUDWidget>(GetOwningPlayerController(), BossHealthClass);
		if (BossHealthWidget)
		{
			BossHealthWidget->AddToViewport();
		}
	}
}


void AAshHUD::BeginPlay()
{
	Super::BeginPlay();

	// 检查是否在编辑器里指定了蓝图类
	if (PlayerHUDClass)
	{
		// 创建 UI 实例
		PlayerHUD = CreateWidget<UPlayerHUDWidget>(GetWorld(), PlayerHUDClass);
        
		if (PlayerHUD)
		{
			// 将 UI 添加到玩家屏幕上
			PlayerHUD->AddToViewport();
		}
	}
}
