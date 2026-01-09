#include "Character/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

APlayerCharacter::APlayerCharacter()
{
	// 初始化摇臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// 初始化相机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 1. 距离与位置偏移 (居中偏上)
	CameraBoom->TargetArmLength = 450.0f; // 设置相机距离，ARPG通常在400-500之间
	// SocketOffset 是关键：让相机焦点在角色头部上方，而不是腰部，避免遮挡视野
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 75.f); 

	// 2. 启用摄像机滞后 (Lag) - 实现那种流畅的跟随感
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.0f; // 数值越小越“粘”，建议 8-12 之间
    
	// 3. 启用旋转滞后 - 转动视角时会有轻微的缓冲感
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 15.0f; // 旋转滞后通常比移动滞后快一点

	// 4. 旋转控制
	CameraBoom->bUsePawnControlRotation = true; // 允许玩家通过鼠标旋转
	CameraBoom->bInheritPitch = true;
	CameraBoom->bInheritYaw = true;
	CameraBoom->bInheritRoll = false; // 侧倾通常关掉

	// ARPG 常用移动设置
	GetCharacterMovement()->bOrientRotationToMovement = true; 
	bUseControllerRotationYaw = false;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 注入增强输入映射
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// 绑定动作
	if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		// 绑定按下 Shift
		EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &APlayerCharacter::OnSprintStarted);
		// 绑定松开 Shift
		EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::OnSprintEnded);
	}
	
}

void APlayerCharacter::OnSprintStarted()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void APlayerCharacter::OnSprintEnded()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.X);
		AddMovementInput(RightDirection, MovementVector.Y);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(-LookAxisVector.Y);
	}
}