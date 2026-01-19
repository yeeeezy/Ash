#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameFramework/Actor.h"
#include "BossFireball.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;

UCLASS()
class ASH_API ABossFireball : public AActor
{
	GENERATED_BODY()
    
public:

	ABossFireball();

protected:
	virtual void BeginPlay() override;

	// 碰撞组件
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComponent;

	// 移动组件
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// 飞行时的特效
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UParticleSystemComponent* FireballEffect;

	// 爆炸时的特效 (在编辑器里指定)
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	// 碰撞回调
	

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
				   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
				   bool bFromSweep, const FHitResult& SweepResult);
public:
	// 初始化飞行速度
	void Launch(float Speed);
};