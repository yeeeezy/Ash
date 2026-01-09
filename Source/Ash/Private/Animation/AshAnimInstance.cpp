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
		// 2. 直接计算速度长度并赋值给 GroundSpeed
		GroundSpeed = Character->GetVelocity().Size();
        
		// 之后你可以从你的 Character 类里获取 bIsTargeting 等变量
	}
}