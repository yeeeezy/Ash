#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

UCLASS()
class ASH_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 供外部（如 Character 或 HUD）调用的 C++ 接口 */
	void RefreshHealth(float NewHealth, float MaxHealth);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS|UI")
	void OnHealthPercentChanged(float Percent);
};