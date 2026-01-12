#include "Animation/AshAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAshAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 1. 获取拥有此动画实例的角色
	ACharacter* Character = Cast<ACharacter>(TryGetPawnOwner());
	if (Character)
	{
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