#include "Character/Boss/EchoBoss.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet/BaseAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UI/AshHUD.h"
#include "UI/PlayerHUDWidget.h"

AEchoBoss::AEchoBoss()
{
	// 1. 将体型缩放改为 2.0 倍
	if (GetMesh())
	{
		GetMesh()->SetRelativeScale3D(FVector(2.0f));
        
		// 2. 注意：体型变大后，需要进一步向下偏移，否则脚会悬空
		// 假设原始偏移是 -90，1.5倍是 -135，那么 2.0倍通常需要设为 -180 左右
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -180.f));
	}

	// 3. 必须同步放大胶囊体，否则 Boss 会穿模或主角无法正确碰撞
	if (GetCapsuleComponent())
	{
		// 原始大小如果是 (34, 88)，2.0倍建议设为 (68, 180) 左右
		GetCapsuleComponent()->SetCapsuleSize(70.f, 180.f);
	}

	// 2. 武器挂载到指定插槽
	BossWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossWeapon"));
	// "Bone_weaponSocket" 必须与你骨骼中的插槽名完全一致
	BossWeapon->SetupAttachment(GetMesh(), TEXT("Bone_weaponSocket"));

	// 3. 创建并挂载所有分件
	BossArmor = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BossArmor"));
	BossArmor->SetupAttachment(GetMesh());

	BossSleeves = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BossSleeves"));
	BossSleeves->SetupAttachment(GetMesh());

	BossShoulders = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BossShoulders"));
	BossShoulders->SetupAttachment(GetMesh());

	BossHelm = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BossHelm"));
	BossHelm->SetupAttachment(GetMesh());

	BossCap = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BossCap"));
	BossCap->SetupAttachment(GetMesh());

	BossGloves = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BossGloves"));
	BossGloves->SetupAttachment(GetMesh());

	// 4. 调整碰撞胶囊体大小以匹配体型
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCapsuleSize(60.f, 135.f);
	}

	
}



void AEchoBoss::BeginPlay()
{
	Super::BeginPlay();

	// 5. 核心：强行同步分件动作，解决换材质或模型后的脱节问题
	if (GetMesh())
	{
		BossArmor->SetLeaderPoseComponent(GetMesh());
		BossSleeves->SetLeaderPoseComponent(GetMesh());
		BossShoulders->SetLeaderPoseComponent(GetMesh());
		BossHelm->SetLeaderPoseComponent(GetMesh());
		BossCap->SetLeaderPoseComponent(GetMesh());
		BossGloves->SetLeaderPoseComponent(GetMesh());
	}

	

	if (AbilitySystemComponent && AttributeSet)
	{
		// 核心绑定代码：监听 Health 属性的变化
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetHealthAttribute()).AddUObject(this, &AEchoBoss::OnHealthChanged);
	}
	//test
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;
	
	AAshHUD* AshHUD = Cast<AAshHUD>(PC->GetHUD());
	if (!AshHUD) return;

	// 2. 检查 Boss 血条是否已创建，如果没有则初始化 (InitBossHealthBar 是我们在 AshHUD 里写的函数)
	if (!AshHUD->BossHealthWidget)
	{
		AshHUD->InitBossHealthBar();

	}

	// 确保 ASC 存在且已经指定了 GE 类
	if (AbilitySystemComponent && DefaultAttributeGE)
	{
		// 创建一个 Effect 上下文
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		// 创建一个具体的 Effect 句柄
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeGE, 1.0f, EffectContext);

		if (SpecHandle.IsValid())
		{
			// 真正把 1000 血量“贴”到 Boss 身上
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			
		}
	}
}

