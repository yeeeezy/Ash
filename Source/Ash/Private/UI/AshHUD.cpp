#include "UI/AshHUD.h"
#include "UI/PlayerHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"


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

void AAshHUD::ShowGameResult(bool bIsWin)
{
	if (GameResultClass)
	{
		// 创建并显示
		GameResultWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), GameResultClass);
		if (GameResultWidget)
		{
			GameResultWidget->AddToViewport();

			// ash: 调用蓝图里的 SetupUI 函数来设置“成功”或“失败”的文本/颜色
			UFunction* Func = GameResultWidget->FindFunction(FName("SetupUI"));
			if (Func)
			{
				GameResultWidget->ProcessEvent(Func, &bIsWin);
			}

			// 3. 游戏交互处理
			APlayerController* PC = GetOwningPlayerController();
			if (PC)
			{
				// 显示鼠标并切换到 UI 模式
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(GameResultWidget->TakeWidget());
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = true;
                
				// 暂停游戏（如果需要的话）
				UGameplayStatics::SetGamePaused(GetWorld(), true);
			}
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
