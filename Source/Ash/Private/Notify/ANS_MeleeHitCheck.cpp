#include "Notify/ANS_MeleeHitCheck.h" // 或者是你项目实际的目录结构

#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Kismet/KismetSystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"


UANS_MeleeHitCheck::UANS_MeleeHitCheck()
{

	EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Melee.Hit"));
}

void UANS_MeleeHitCheck::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	// 安全检查 1: 确保 MeshComp 存在且有 Owner
	if (!MeshComp || !MeshComp->GetOwner()) return;
	AActor* OwnerActor = MeshComp->GetOwner();

	// 安全检查 2: 如果是在编辑器预览窗口（没有真正的 World），跳过逻辑防止闪退
	if (!OwnerActor->GetWorld() || OwnerActor->GetWorld()->WorldType == EWorldType::EditorPreview) return;

	// 1. 寻找挂载的武器 Actor
	TArray<AActor*> AttachedActors;
	OwnerActor->GetAttachedActors(AttachedActors);
	AActor* WeaponActor = nullptr;

	for (AActor* Actor : AttachedActors)
	{
		if (Actor && Actor->GetRootComponent() && Actor->GetRootComponent()->GetAttachSocketName() == TEXT("sword_equipped"))
		{
			WeaponActor = Actor;
			break;
		}
	}

	// 安全检查 3: 如果还没拿到武器，不执行后续逻辑
	if (!WeaponActor) return;

	// 2. 查找武器蓝图中的静态网格体组件
	UStaticMeshComponent* SwordMesh = WeaponActor->FindComponentByClass<UStaticMeshComponent>();
	if (!SwordMesh) return;

	// 3. 获取 Socket 位置
	FVector Start = SwordMesh->GetSocketLocation(StartSocketName);
	FVector End = SwordMesh->GetSocketLocation(EndSocketName);

	// 4. 执行碰撞检测
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnerActor);
	ActorsToIgnore.Add(WeaponActor);

	TArray<FHitResult> OutHits;
	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		OwnerActor, Start, End, TraceRadius,
		UEngineTypes::ConvertToTraceType(ECC_Pawn), 
		false, ActorsToIgnore, 
		EDrawDebugTrace::ForOneFrame, 
		OutHits, true
	);

	// 5. 安全检查 4: 只有在标签合法时才发送事件
	if (bHit && EventTag.IsValid())
	{
		// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("[SUCCESS] Damage applied to: %s"));

		for (const FHitResult& Hit : OutHits)
		{
			FGameplayEventData Payload;
			Payload.Instigator = OwnerActor;
			Payload.Target = Hit.GetActor();
			Payload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);
			Payload.EventMagnitude = DamageMultiplier;
			
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
		}
	}
}