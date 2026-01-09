#include "Framework/AshPlayerController.h"

void AAshPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 这里可以设置初始的鼠标光标状态
	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}