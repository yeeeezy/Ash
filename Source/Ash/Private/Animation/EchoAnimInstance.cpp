#include "Animation/EchoAnimInstance.h"

#include "Character/BaseCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UEchoAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 初始获取一次
	EchoCharacter = Cast<ABaseCharacter>(TryGetPawnOwner());
}

void UEchoAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 确保指针有效（热重载或动态生成时可能需要重新获取）
	if (!EchoCharacter)
	{
		EchoCharacter = Cast<ABaseCharacter>(TryGetPawnOwner());
	}

	if (EchoCharacter)
	{
		// 获取速度并去掉 Z 轴影响（幽灵在上下漂浮时不应该产生移动速度）
		FVector Velocity = EchoCharacter->GetVelocity();
		Velocity.Z = 0;
		Speed = Velocity.Size();

		// 判断是否有输入且速度大于阈值
		bIsMoving = Speed > 3.f && EchoCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	}
}