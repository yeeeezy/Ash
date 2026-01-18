#include "Character/Boss/BaseBoss.h"
#include "AbilitySystemComponent.h"
#include "AI/BossAIController.h"
#include "AttributeSet/BaseAttributeSet.h"
#include "Character/PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/AshHUD.h"
#include "UI/PlayerHUDWidget.h"

ABaseBoss::ABaseBoss()
{
    // 创建碰撞球
    LockOnZone = CreateDefaultSubobject<USphereComponent>(TEXT("LockOnZone"));
    LockOnZone->SetupAttachment(RootComponent);
    LockOnZone->SetSphereRadius(800.f); 
    
    LockOnZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    LockOnZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // 禁用控制器旋转影响，由移动组件处理转身
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true; 
    GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
}

// --- 实现接口要求的函数 ---
UAbilitySystemComponent* ABaseBoss::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ABaseBoss::ApplyDamageToSelf()
{
    if (AbilitySystemComponent && DamageGEClass)
    {
       FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
       Context.AddSourceObject(this);

       FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageGEClass, 1.0f, Context);

       if (SpecHandle.IsValid())
       {
          AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
          UE_LOG(LogTemp, Warning, TEXT("Boss took damage! Check Health Bar."));
       }
    }
}

void ABaseBoss::BeginPlay()
{
    Super::BeginPlay();
    
    LockOnZone->OnComponentBeginOverlap.AddDynamic(this, &ABaseBoss::OnLockZoneOverlap);

    // 1. 核心顺序修复：先初始化 ActorInfo，再绑定属性回调，最后发牌
    if (AbilitySystemComponent)
    {
       // 必须最先执行初始化，否则后续能力和属性操作可能失效
       AbilitySystemComponent->InitAbilityActorInfo(this, this);

       if (AttributeSet)
       {
          AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
             AttributeSet->GetHealthAttribute()).AddUObject(this, &ABaseBoss::OnHealthChanged);
       }

       // 2. 赋予默认能力 (移除了 HasAuthority 限制以便单机调试)
       for (TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
       {
          if (AbilityClass)
          {
             AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, -1, this));
             // UE_LOG(LogTemp, Warning, TEXT("Boss GAS: Successfully gave ability: %s"), *AbilityClass->GetName());
          }
       }

       // 3. 应用初始属性 GE
       if (DefaultAttributeGE)
       {
          FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
          EffectContext.AddSourceObject(this);
          FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeGE, 1.0f, EffectContext);

          if (SpecHandle.IsValid())
          {
             AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
          }
       }
    }
    
    // UI 初始化逻辑
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
       AAshHUD* AshHUD = Cast<AAshHUD>(PC->GetHUD());
       if (AshHUD && !AshHUD->BossHealthWidget)
       {
          AshHUD->InitBossHealthBar();
       }
    }
}

void ABaseBoss::OnLockZoneOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
    {
       Player->EnableTargetLock(this);
       
       if (ABossAIController* BossAI = Cast<ABossAIController>(GetController()))
       {
          BossAI->OnPlayerEnteredArena(Player);
       }
    }
}

void ABaseBoss::OnHealthChanged(const FOnAttributeChangeData& Data)
{
   float NewHealth = Data.NewValue;
   UE_LOG(LogTemp, Warning, TEXT("GAS Debug: OnHealthChanged Triggered! New Value: %f"), NewHealth);

   // --- 1. 获取 PlayerController (全局共用一次获取即可) ---
   APlayerController* PC = GetWorld()->GetFirstPlayerController();
   
   // --- 2. 更新 UI 逻辑 ---
   if (PC)
   {
      AAshHUD* AshHUD = Cast<AAshHUD>(PC->GetHUD());
      if (AshHUD)
      {
         if (!AshHUD->BossHealthWidget) AshHUD->InitBossHealthBar();
         if (AshHUD->BossHealthWidget)
         {
            AshHUD->BossHealthWidget->RefreshHealth(NewHealth, AttributeSet->GetMaxHealth());
         }
      }
   }

   // --- 3. 死亡逻辑判断 ---
   if (NewHealth <= 0.f)
   {
      if (AbilitySystemComponent)
      {
         // 注意：这里的标签需要和你 GA 蓝图中的 Ability Tags 保持一致
         FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(FName("Event.Boss.Dead"));
         
         // 尝试激活死亡能力
         bool bAbilityActivated = AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(DeathTag));
            
         if (bAbilityActivated)
         {
            UE_LOG(LogTemp, Warning, TEXT("Boss Death Ability Activated!"));

            // --- 4. 强制让锁定我的玩家解锁 ---
            // 这里直接复用上面已经获取过的 PC，解决了变量重复声明的报错
            if (PC)
            {
               APlayerCharacter* Player = Cast<APlayerCharacter>(PC->GetPawn());
               
               // 如果玩家存在，则执行解锁逻辑
               // 建议增加一个 Player->GetCurrentLockedTarget() == this 的判断（如果你已在 Player 类中实现该 Getter）
               if (Player)
               {
                  Player->DisableTargetLock();
                  UE_LOG(LogTemp, Warning, TEXT("Boss is dead, forced Player to unlock camera."));
               }
            }
         }
      }
   }
}