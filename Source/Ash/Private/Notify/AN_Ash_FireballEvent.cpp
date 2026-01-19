#include "Notify/AN_Ash_FireballEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UAN_Ash_FireballEvent::UAN_Ash_FireballEvent()
{
	// ash, 这里默认设置成我们统一个那个标签
	EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Boss.SpawnFireball"));
}

void UAN_Ash_FireballEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ash"));
	if (MeshComp && MeshComp->GetOwner())
	{
		// 关键点：通过这个库函数直接给 Actor 发送 GameplayEvent
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, FGameplayEventData());
		
		// 调试日志，ash 你可以在 Output Log 里确认它是否触发
		// UE_LOG(LogTemp, Log, TEXT("ash: AnimNotify Sent Tag: %s"), *EventTag.ToString());
	}
}