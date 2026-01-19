#include "Actors/BossFireball.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

ABossFireball::ABossFireball()
{
    PrimaryActorTick.bCanEverTick = false;

    // 1. 初始化碰撞球
    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    SphereComponent->InitSphereRadius(20.0f);
    SphereComponent->SetCollisionProfileName(TEXT("Projectile")); // 确保设置了Projectile碰撞预设
    
    // 注册碰撞事件

    SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ABossFireball::OnOverlap);
    RootComponent = SphereComponent;

    // 2. 初始化飞行效果
    FireballEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireballEffect"));
    FireballEffect->SetupAttachment(RootComponent);

    // 3. 初始化移动组件
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = SphereComponent;
    ProjectileMovement->InitialSpeed = 2000.f;
    ProjectileMovement->MaxSpeed = 2000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.f; // 火球通常不受重力影响
}

void ABossFireball::BeginPlay()
{
    Super::BeginPlay();
    // 可以在这里设置初始寿命，防止飞出世界后一直存在
    SetLifeSpan(5.0f);
}


//
// void ABossFireball::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
//     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
// {
//     if (OtherActor == GetInstigator()) return;
//
//     UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
//     UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
//
//     if (SourceASC && TargetASC)
//     {
//         // ash: 打印一下 Class，确认蓝图里的值真的传进 C++ 了
//         FString ClassName = DamageEffectClass ? DamageEffectClass->GetName() : TEXT("NONE");
//         GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("ash: Using GE Class: %s"), *ClassName));
//
//         // 尝试手动创建 Context
//         FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
//         
//         // --- 核心测试：尝试创建 Spec ---
//         // 我们把 Level 固定设为 1.0f 试试
//         FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, ContextHandle);
//
//         if (SpecHandle.IsValid())
//         {
//             FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.FinalDamage"));
//             UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageTag, 20.0f);
//             
//             SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
//             GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("ash: Spec is VALID and Sent!"));
//         }
//         else
//         {
//             // 如果走到这里，说明 SourceASC 是有的，但它“拒绝”产生这个 GE
//             GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ash: Spec Creation FAILED! Check GE CDO."));
//             
//             // 额外排查：检查这个 GE 类是否被正确实例化了
//             if (DamageEffectClass)
//             {
//                 const UGameplayEffect* GE_CDO = DamageEffectClass->GetDefaultObject<UGameplayEffect>();
//                 if (!GE_CDO) {
//                     GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ash: GE CDO is NULL!"));
//                 }
//             }
//         }
//     }
// }
void ABossFireball::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                   bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor == GetInstigator()) return; // 不伤害 Boss 自己

    
    // 1. 播放特效
    if (ExplosionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
    }

    // 2. GAS 伤害逻辑
    UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

    if (SourceASC && TargetASC)
    {
        FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
        
        // --- ash 重点：即使是 Overlap 也可以这样传信息 ---
        if (bFromSweep)
        {
            ContextHandle.AddHitResult(SweepResult); // 如果是扫掠发生的重叠，直接用 SweepResult
        }
        else
        {
            // 如果没有 SweepResult，手动创建一个简单的 HitResult 传进去
            FHitResult DummyHit;
            DummyHit.ImpactPoint = GetActorLocation();
            ContextHandle.AddHitResult(DummyHit);
        }

        FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, ContextHandle);
        if (SpecHandle.IsValid())
        {
            // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ash: GE Failed to Apply! (Check Tags/Immunity)"));
            // 注入你图片里的 Data.FinalDamage 标签
            FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.FinalDamage"));
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageTag, -10.0f);

            SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
        }
    }

    Destroy(); // 撞到东西就销毁
}
