#include "Character/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AttributeSet/BaseAttributeSet.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/Boss/EchoBoss.h"


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

    // 1. 距离与位置偏移
    CameraBoom->TargetArmLength = 450.0f; 
    CameraBoom->SocketOffset = FVector(0.f, 0.f, 100.f); 

    // 3. 设置俯仰角 (Pitch)
    // FRotator(Pitch, Yaw, Roll)。-20 到 -30 度是比较舒服的俯视感
    CameraBoom->SetRelativeRotation(FRotator(-25.f, 0.f, 0.f));
    // 2. 启用摄像机滞后
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 10.0f; 
    
    // 3. 启用旋转滞后
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraRotationLagSpeed = 15.0f; 

    // 4. 旋转控制
    CameraBoom->bUsePawnControlRotation = true; 
    CameraBoom->bInheritPitch = true;
    CameraBoom->bInheritYaw = true;
    CameraBoom->bInheritRoll = false; 

    // ARPG 常用移动设置
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 3000.f, 0.f);

    //Jump
    JumpMaxCount = 1;

    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

    AttributeSet = CreateDefaultSubobject<UBaseAttributeSet>(TEXT("AttributeSet"));
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
       if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
       {
          Subsystem->AddMappingContext(DefaultMappingContext, 0);
       }
    }

    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    if (AbilitySystemComponent && AttributeSet)
    {
        // 设置初始血量为 100
        AttributeSet->InitHealth(100.f);
        AttributeSet->InitMaxHealth(100.f);
        
    }
}


void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
       EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
       EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
       EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &APlayerCharacter::OnSprintStarted);
       EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::OnSprintEnded);
        EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::OnJumpedStarted);
    }
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
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

void APlayerCharacter::OnJumpedStarted()
{
    
    Jump();

    // 1. 检查当前是否已经正在播放跳跃蒙太奇
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    bool bIsPlaying = AnimInstance && AnimInstance->Montage_IsPlaying(JumpMontage);

    // 2. 只有没在播放时，才允许触发
    if (!bIsPlaying)
    {
        Jump(); 
        if (JumpMontage)
        {
            PlayAnimMontage(JumpMontage, 1.0f);
        }
    }
}
