#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AshWeapon.generated.h"

UCLASS()
class ASH_API AAshWeapon : public AActor
{
	GENERATED_BODY()
    
public:	
	AAshWeapon();

protected:
	// 专门用于显示“自负之剑”模型的组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* WeaponMesh;

public:
	// 获取网格体，方便后续做溶解效果的材质操作
	FORCEINLINE UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
};