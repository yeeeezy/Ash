#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AshPlayerController.generated.h"

UCLASS()
class ASH_API AAshPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};