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

void APlayerCharacter::EnableTargetLock(AActor* NewTarget)
{
    CurrentLockedTarget = NewTarget;
    bIsAutoLocking = true;
    
    // 锁定时的移动配置：不自动朝向移动方向，而是允许侧移
    GetCharacterMovement()->bOrientRotationToMovement = false;
}

void APlayerCharacter::DisableTargetLock()
{

    CurrentLockedTarget = nullptr;
    bIsAutoLocking = false;
    
    // 恢复自由移动朝向
    GetCharacterMovement()->bOrientRotationToMovement = true;
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
        FGameplayAbilitySpec AttackSpec(DefaultMeleeAbility, 1, static_cast<int32>(EAbilityInputID::Attack)); 
        GetAbilitySystemComponent()->GiveAbility(AttackSpec);
    }

    if (AbilitySystemComponent && DodgeAbilityClass)
    {
        AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(
            DodgeAbilityClass, 1, static_cast<int32>(EAbilityInputID::Dodge)));


    }


    if (AbilitySystemComponent && DeathAbilityClass)
    {
        // 死亡能力不设 InputID，传 0 或 -1
        FGameplayAbilitySpec DeathSpec(DeathAbilityClass, 1);
        AbilitySystemComponent->GiveAbility(DeathSpec);
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
    
    // 如果开启了锁定，且有目标（这里的 CurrentLockedTarget 就是你的 Boss）
    if (bIsAutoLocking && CurrentLockedTarget)
    {
        // 1. 获取位置信息
        FVector PlayerLoc = GetActorLocation();
        // 看向 Boss 中心（稍微抬高一点）
        FVector TargetLoc = CurrentLockedTarget->GetActorLocation() + FVector(0.f, 0.f, 50.f);
        
        // 2. 计算基础朝向旋转
        FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(PlayerLoc, TargetLoc);
        
        // --- ash 核心逻辑：动态 Pitch 调整 ---
        // 3. 计算高度差：Boss 高度 - 玩家高度
        float HeightDiff = TargetLoc.Z - PlayerLoc.Z;
        
        // 映射逻辑：高度差越大，Pitch 越往正值走（仰视）
        // 假设高度差在 0 到 1000 之间波动
        // 我们把这个差值映射到 [MinLockPitch, MaxLockPitch]
        float DynamicPitch = FMath::GetMappedRangeValueClamped(
            FVector2D(0.f, 300.f), 
            FVector2D(MinLockPitch, MaxLockPitch), 
            HeightDiff
        );

        // 4. 构造最终旋转：使用动态计算的 Pitch
        FRotator FinalLockRot = FRotator(DynamicPitch, LookAtRot.Yaw, 0.f);

        // 5. 平滑设置控制器旋转（加入插值让镜头不那么抖）
        if (GetController())
        {
            FRotator CurrentRot = GetController()->GetControlRotation();
            FRotator SmoothedRot = FMath::RInterpTo(CurrentRot, FinalLockRot, DeltaTime, 10.0f);
            GetController()->SetControlRotation(SmoothedRot);
        }

        // 6. 强制身体朝向同步 (Yaw)
        SetActorRotation(FRotator(0.f, LookAtRot.Yaw, 0.f));
    }
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

    // 2. 新增：死亡判定并发送 Gameplay Event
    if (Data.NewValue <= 0.0f)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Health is ZERO! Sending Event..."));
        FGameplayTag DeathEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Character.Dead"));
        
        FGameplayEventData Payload;
        Payload.EventTag = DeathEventTag;
        Payload.Instigator = this; // 这里的 Instigator 可以根据需要设置为伤害来源
        Payload.Target = this;

        // 发送事件以激活 GA_Character_Death
        if (GetAbilitySystemComponent())
        {
            GetAbilitySystemComponent()->HandleGameplayEvent(DeathEventTag, &Payload);
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
    

    if (GetAbilitySystemComponent())
    {
        /** * 通知 ASC 按下了 InputID 为 1 的按键。
         * 这会触发你在 GiveAbility 时指定的 InputID 为 1 的能力，
         * 或者是触发正在运行的能力中的 WaitInputPress 任务。
         */
        GetAbilitySystemComponent()->AbilityLocalInputPressed(static_cast<int32>(EAbilityInputID::Attack));
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
        // PlayRollMontage();
        if (AbilitySystemComponent)
        {
            // 仅仅发送信号，剩下的交给 GA_Dodge 
            AbilitySystemComponent->AbilityLocalInputPressed(static_cast<int32>(EAbilityInputID::Dodge));
        }
    }
    
}
