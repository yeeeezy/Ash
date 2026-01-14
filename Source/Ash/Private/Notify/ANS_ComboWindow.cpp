// Fill out your copyright notice in the Description page of Project Settings.


#include "Notify/ANS_ComboWindow.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"

// ANS_ComboWindow.cpp 核心逻辑
void UANS_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{

	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("notify"));
	// 1. 获取 ASC
	if (IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(MeshComp->GetOwner()))
	{
		UAbilitySystemComponent* ASC = Interface->GetAbilitySystemComponent();
		// 2. 给角色加上“连招窗口开启”的标签
		ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.ComboWindow"));
	}
}

void UANS_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(MeshComp->GetOwner()))
	{
		UAbilitySystemComponent* ASC = Interface->GetAbilitySystemComponent();
		// 3. 时间一结束，立刻移除标签
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.ComboWindow"));
	}
}