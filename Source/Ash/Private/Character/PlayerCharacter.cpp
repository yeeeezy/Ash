#include "Character/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AshAnimInstance.h"
#include "AttributeSet/BaseAttributeSet.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/Boss/EchoBoss.h"
#include "UI/AshHUD.h"
#include "UI/PlayerHUDWidget.h"

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
    GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

    //Jump
    JumpMaxCount = 1;

    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

    AttributeSet = CreateDefaultSubobject<UBaseAttributeSet>(TEXT("AttributeSet"));

    //武器状态设置为未装备
    WeaponState = EWeaponState::Unequipped;
}

void APlayerCharacter::EquipSword()
{
    if (EquipMontage)
    {
        PlayAnimMontage(EquipMontage);

        // --- 核心改动：一键封锁所有游戏内输入 ---
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            // 切换到 UIOnly 模式，不处理任何游戏操作
            FInputModeUIOnly InputMode;
            // 设置锁定鼠标到视口（可选，防止鼠标滑出屏幕）
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            
            PC->SetInputMode(InputMode);

            // 1.5s 后恢复 GameOnly 模式
            FTimerHandle TimerHandle;
            GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([PC]()
            {
                if (PC)
                {
                    // 恢复成正常的“仅游戏”模式，按键和鼠标点击重新回到角色逻辑
                    FInputModeGameOnly GameMode;
                    PC->SetInputMode(GameMode);
                }
            }), 1.5f, false);
        }
    }

    SprintSpeed = 400.f;
    WalkSpeed = 150.f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APlayerCharacter::UnequipSword()
{

    SprintSpeed = 600.f;
    WalkSpeed = 250.f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    WeaponState = EWeaponState::Unequipped;
    if (UAshAnimInstance* AnimInst = Cast<UAshAnimInstance>(GetMesh()->GetAnimInstance()))
    {
        AnimInst->UpdateWeaponState(EWeaponState::Unequipped);
    }
}

void APlayerCharacter::HandleWeaponAttachment()
{

    
    if (SwordClass && !EquippedSword)
    {
        FActorSpawnParameters SpawnParams;
        EquippedSword = GetWorld()->SpawnActor<AActor>(SwordClass, GetActorLocation(), GetActorRotation(), SpawnParams);
        
        if (EquippedSword)
        {
            EquippedSword->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("sword_equipped"));
        }
    }
    WeaponState = EWeaponState::SwordEquipped;
    // 更新枚举状态以切换到持剑混合空间
    if (UAshAnimInstance* AnimInst = Cast<UAshAnimInstance>(GetMesh()->GetAnimInstance()))
    {
        AnimInst->UpdateWeaponState(EWeaponState::SwordEquipped);
    }
    
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
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            UBaseAttributeSet::GetHealthAttribute()).AddUObject(this, &APlayerCharacter::HealthChanged);
        
    }


    EquipSword();

    if (GetAbilitySystemComponent() && DefaultMeleeAbility)
    {
        // 赋予能力。注意：最后一个参数是 InputID，我们假设设为 1
        FGameplayAbilitySpec AttackSpec(DefaultMeleeAbility, 1, 1); 
        GetAbilitySystemComponent()->GiveAbility(AttackSpec);
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
   
        EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerCharacter::Input_AttackPressed);
    }
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APlayerCharacter::HealthChanged(const FOnAttributeChangeData& Data)
{

    OnHealthChanged.Broadcast(Data.NewValue, AttributeSet->GetMaxHealth());

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        // 获取我们在第二步创建的自定义 HUD
        AAshHUD* AshHUD = Cast<AAshHUD>(PC->GetHUD());
        
        // 确保 HUD 存在且其中的 PlayerHUD 实例已创建
        if (AshHUD && AshHUD->PlayerHUD)
        {
            // 直接调用你刚刚在 UI 的 cpp 中实现的计算逻辑
            AshHUD->PlayerHUD->RefreshHealth(Data.NewValue, AttributeSet->GetMaxHealth());
        }
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

void APlayerCharacter::Input_AttackPressed()
{
    
    // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Input: Left Mouse Pressed!"));
    if (GetAbilitySystemComponent())
    {
        /** * 通知 ASC 按下了 InputID 为 1 的按键。
         * 这会触发你在 GiveAbility 时指定的 InputID 为 1 的能力，
         * 或者是触发正在运行的能力中的 WaitInputPress 任务。
         */
        GetAbilitySystemComponent()->AbilityLocalInputPressed(1);
    }
    
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

void APlayerCharacter::PlayRollMontage()
{

    // if (GetCharacterMovement()->IsFalling()) return;
    //
    // UAshAnimInstance* AnimInst = Cast<UAshAnimInstance>(GetMesh()->GetAnimInstance());
    // if (AnimInst)
    // {
    //     // 逻辑清晰：直接根据条件播不同的文件
    //     if (AnimInst->GroundSpeed < 10.f || AnimInst->Direction <= 0.f)
    //     {
    //         PlayAnimMontage(RollLeftMontage);
    //     }
    //     else
    //     {
    //         PlayAnimMontage(RollRightMontage);
    //     }
    // }

    if (GetCharacterMovement()->IsFalling()) return;

    UAshAnimInstance* AnimInst = Cast<UAshAnimInstance>(GetMesh()->GetAnimInstance());
    if (AnimInst)
    {
        // 2. 状态检查：如果当前正在播放左闪或右闪蒙太奇，直接返回，不执行后续逻辑
        // 这样在一次翻滚动作结束前，按任何键都不会触发新的翻滚
        if (AnimInst->Montage_IsPlaying(RollLeftMontage) || 
            AnimInst->Montage_IsPlaying(RollRightMontage))
        {
            return;
        }
        // --- 1. 定义闪避的速度（固定值） ---
        // 数值越大，闪避距离越远。通常 1200-1800 比较合适
        float RollImpulse = 1500.f; 
        FVector LaunchDirection = FVector::ZeroVector;

        // --- 2. 播放动画并确定发射方向 ---
        // 原地 (GroundSpeed < 10) 或向左 (Direction <= 0) -> 向左闪
        if (AnimInst->GroundSpeed < 10.f || AnimInst->Direction <= 0.f)
        {
            if (RollLeftMontage)
            {
                PlayAnimMontage(RollLeftMontage);
                // 获取角色右向量的反方向，即左方向
                LaunchDirection = -GetActorRightVector();
            }
        }
        else // 其他情况 -> 向右闪
        {
            if (RollRightMontage)
            {
                PlayAnimMontage(RollRightMontage);
                // 获取角色右向量
                LaunchDirection = GetActorRightVector();
            }
        }

        // --- 3. 执行发射 ---
        if (!LaunchDirection.IsNearlyZero())
        {
            // LaunchCharacter(速度向量, 是否覆盖水平速度, 是否覆盖垂直速度)
            // 两个 true 确保了无论你之前在做什么，闪避瞬间的速度都是这个固定值
            LaunchCharacter(LaunchDirection * RollImpulse, true, true);
        }
    }
}

void APlayerCharacter::OnJumpedStarted()
{
    



    if (WeaponState == EWeaponState::Unequipped)
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
    else if (WeaponState == EWeaponState::SwordEquipped)
    {
        // 已装备武器：执行翻滚
        PlayRollMontage();
    }
}
