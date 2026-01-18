#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ANS_EchoMeleeHitCheck.generated.h"

UCLASS()
class ASH_API UANS_EchoMeleeHitCheck : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_EchoMeleeHitCheck();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	               const FAnimNotifyEventReference& EventReference);

protected:
	UPROPERTY(EditAnywhere, Category = "Echo|HitCheck")
	FGameplayTag EventTag;

	/** 每一段攻击的伤害倍率，默认 1.0 */
	UPROPERTY(EditAnywhere, Category = "Echo|HitCheck")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Echo|HitCheck")
	float TraceRadius = 20.f;

	UPROPERTY(EditAnywhere, Category = "Echo|HitCheck")
	FName StartSocketName = TEXT("Scythe_Base");

	UPROPERTY(EditAnywhere, Category = "Echo|HitCheck")
	FName EndSocketName = TEXT("Scythe_Tip");

	UPROPERTY(EditAnywhere, Category = "Echo|HitCheck")
	TEnumAsByte<EDrawDebugTrace::Type> DebugTraceType = EDrawDebugTrace::ForOneFrame;

private:
	/** 这里的 HitActors 是实例化的，每个 ANS 条都会有自己独立的列表 */
	UPROPERTY()
	TArray<AActor*> HitActors;
};