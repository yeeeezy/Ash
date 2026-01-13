#include "UI/PlayerHUDWidget.h"


void UPlayerHUDWidget::RefreshHealth(float NewHealth, float MaxHealth)
{
	// 1. 安全检查：防止除以零导致的崩溃
	if (MaxHealth > 0.f)
	{
		// 2. 在 C++ 中计算百分比，保证逻辑一致性
		const float HealthPercent = NewHealth / MaxHealth;
    

		OnHealthPercentChanged(HealthPercent);
	}
}


