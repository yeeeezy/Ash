#include "Notify/ANS_EchoMeleeHitCheck.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "GameplayTagContainer.h"

UANS_EchoMeleeHitCheck::UANS_EchoMeleeHitCheck()
{
    EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Boss.Melee.Hit"));
}

void UANS_EchoMeleeHitCheck::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
    // 明确清空列表
    HitActors.Empty();
}

void UANS_EchoMeleeHitCheck::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    if (!MeshComp || !MeshComp->GetOwner()) return;
    AActor* OwnerActor = MeshComp->GetOwner();

    // 1. 寻找武器组件
    UStaticMeshComponent* WeaponMesh = nullptr;
    TArray<UStaticMeshComponent*> StaticMeshComps;
    OwnerActor->GetComponents<UStaticMeshComponent>(StaticMeshComps);

    for (UStaticMeshComponent* Comp : StaticMeshComps)
    {
        if (Comp && (Comp->GetName().Contains(TEXT("BossWeapon")) || Comp->ComponentHasTag(TEXT("BossWeapon"))))
        {
            WeaponMesh = Comp;
            break;
        }
    }
    if (!WeaponMesh) WeaponMesh = OwnerActor->FindComponentByClass<UStaticMeshComponent>();
    if (!WeaponMesh) return;

    // 2. 射线检测起止点
    FVector Start = WeaponMesh->GetSocketLocation(StartSocketName);
    FVector End = WeaponMesh->GetSocketLocation(EndSocketName);

    // 3. 执行对象检测（只扫 Pawn）
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(OwnerActor);
    TArray<FHitResult> OutHits;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn)); 

    bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
       OwnerActor, Start, End, TraceRadius, ObjectTypes, false, 
       ActorsToIgnore, DebugTraceType, OutHits, true
    );

    if (bHit)
    {
        for (const FHitResult& Hit : OutHits)
        {
            AActor* Victim = Hit.GetActor();
            if (Victim && !HitActors.Contains(Victim))
            {
                HitActors.Add(Victim);

                FGameplayEventData Payload;
                Payload.Instigator = OwnerActor;
                Payload.Target = Victim;
                Payload.EventMagnitude = DamageMultiplier; 
                Payload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);

                // 发送信号
                UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
                
                // if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, FString::Printf(TEXT("ANS Hit: %s"), *Victim->GetName()));
            }
        }
    }
}

void UANS_EchoMeleeHitCheck::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);
    // 结束时再次清空，确保下一段攻击干净
    HitActors.Empty();
}