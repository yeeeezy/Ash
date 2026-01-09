#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

UCLASS()
class ASH_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

protected:
	/** 基础属性：生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ash|Attributes")
	float Health;

	/** 基础属性：最大生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ash|Attributes")
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Ash|Attributes")
	bool bIsDead;

	/** 通用受击处理 */
	UFUNCTION(BlueprintCallable, Category = "Ash|Combat")
	virtual void HandleTakeDamage(float DamageAmount);

	virtual void BeginPlay() override;
};