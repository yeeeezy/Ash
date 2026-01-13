#include "Animation/AshAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "Character/PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAshAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 1. 获取拥有此动画实例的角色
	ACharacter* Character = Cast<ACharacter>(TryGetPawnOwner());
	if (Character)
	{
		// 获取角色的速度向量
		FVector Velocity = Character->GetVelocity();
		// 获取角色的旋转量
		FRotator Rotation = Character->GetActorRotation();



		// 2. 计算 Direction (-180 到 180)
		// 这个函数会自动根据角色朝向和移动方向计算出偏移角度
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation);
		GroundSpeed = Character->GetVelocity().Size();

		// 检查是否有玩家输入（加速度）
		bool bHasInput = Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;

		if (bHasInput)
		{
			bIsAccelerating = true; 
			bIsStopping = false;
		}
		else
		{
			bIsAccelerating = false;
          
			// 只有当“没有输入”且“还有余速”时，才判定为正在收步
			// 如果速度已经接近 0，则 bIsStopping 也要关掉，否则会卡在 End 动画
			bIsStopping = (GroundSpeed > 5.f); 
		}

		float MaxSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
		bIsRunning = (MaxSpeed > 250.f); 
		
	}
}

void UAshAnimInstance::AnimNotify_AttachWeapon()
{

	APlayerCharacter* Character = Cast<APlayerCharacter>(TryGetPawnOwner());
	if (Character)
	{
		Character->HandleWeaponAttachment(); 
	}
	
}
