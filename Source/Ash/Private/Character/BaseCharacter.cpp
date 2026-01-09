#include "Character/BaseCharacter.h" // 确保包含路径匹配你的文件夹结构

ABaseCharacter::ABaseCharacter()
{
	// 默认每帧执行 Tick，ARPG 基础类通常保留
	PrimaryActorTick.bCanEverTick = true;

	// 初始化默认数值
	MaxHealth = 100.f;
	Health = MaxHealth;
	bIsDead = false;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	// 这里可以放置初始化逻辑，例如确保血量不超过上限
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::HandleTakeDamage(float DamageAmount)
{
	if (bIsDead) return;

	// 扣除生命值并限制在 0 以上
	Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);

	// 检查是否死亡
	if (Health <= 0.f)
	{
		bIsDead = true;
		// 可以在这里触发死亡动画或逻辑
		UE_LOG(LogTemp, Warning, TEXT("%s has died!"), *GetName());
	}
}