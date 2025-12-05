// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ABCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ABCharacterControlData.h"
#include "UI/ABHUDWidget.h"
#include "CharacterStat/ABCharacterStatComponent.h"
#include "Interface/ABGameInterface.h"
#include "ArenaBattle.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "Engine/DamageEvents.h"
#include "Physics/ABCollision.h"

#include "Net/UnrealNetwork.h"

#include "GameFramework/GameStateBase.h"

AABCharacterPlayer::AABCharacterPlayer()
{
	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Input
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionJumpRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Jump.IA_Jump'"));
	if (nullptr != InputActionJumpRef.Object)
	{
		JumpAction = InputActionJumpRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputChangeActionControlRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ChangeControl.IA_ChangeControl'"));
	if (nullptr != InputChangeActionControlRef.Object)
	{
		ChangeControlAction = InputChangeActionControlRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ShoulderMove.IA_ShoulderMove'"));
	if (nullptr != InputActionShoulderMoveRef.Object)
	{
		ShoulderMoveAction = InputActionShoulderMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionShoulderLookRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_ShoulderLook.IA_ShoulderLook'"));
	if (nullptr != InputActionShoulderLookRef.Object)
	{
		ShoulderLookAction = InputActionShoulderLookRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionQuaterMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_QuaterMove.IA_QuaterMove'"));
	if (nullptr != InputActionQuaterMoveRef.Object)
	{
		QuaterMoveAction = InputActionQuaterMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionAttackRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Attack.IA_Attack'"));
	if (nullptr != InputActionAttackRef.Object)
	{
		AttackAction = InputActionAttackRef.Object;
	}

	CurrentCharacterControlType = ECharacterControlType::Quater;

	// 시작할 때는 공격 가능으로 설정.
	bCanAttack = true;

	// 리플리케이션 활성화.
	bReplicates = true;
}

void AABCharacterPlayer::PostInitializeComponents()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::PostInitializeComponents();

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::BeginPlay()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		EnableInput(PlayerController);
	}

	SetCharacterControl(CurrentCharacterControlType);
}

void AABCharacterPlayer::SetDead()
{
	Super::SetDead();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		DisableInput(PlayerController);
	}
}

void AABCharacterPlayer::PossessedBy(AController* NewController)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// PossessedBy 호출되기 전 액터의 소유 확인.
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		AB_LOG(LogABNetwork, Log, TEXT("Owner: %s"), *OwnerActor->GetName());
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No Owner"));
	}

	Super::PossessedBy(NewController);

	// PossessedBy 호출된 후 액터의 소유 확인.
	OwnerActor = GetOwner();
	if (OwnerActor)
	{
		AB_LOG(LogABNetwork, Log, TEXT("Owner: %s"), *OwnerActor->GetName());
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No Owner"));
	}

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::OnRep_Owner()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::OnRep_Owner();

	// OnRep_Owner 함수 호출 후 오너 확인.
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		AB_LOG(LogABNetwork, Log, TEXT("Owner: %s"), *OwnerActor->GetName());
	}
	else
	{
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No Owner"));
	}

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::PostNetInit()
{
	AB_LOG(LogABNetwork, Log, TEXT("%s %s"), TEXT("Begin"), *GetName());

	Super::PostNetInit();

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABCharacterPlayer::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInputComponent->BindAction(ChangeControlAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ChangeCharacterControl);
	EnhancedInputComponent->BindAction(ShoulderMoveAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ShoulderMove);
	EnhancedInputComponent->BindAction(ShoulderLookAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::ShoulderLook);
	EnhancedInputComponent->BindAction(QuaterMoveAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::QuaterMove);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::Attack);
}

void AABCharacterPlayer::ChangeCharacterControl()
{
	if (CurrentCharacterControlType == ECharacterControlType::Quater)
	{
		SetCharacterControl(ECharacterControlType::Shoulder);
	}
	else if (CurrentCharacterControlType == ECharacterControlType::Shoulder)
	{
		SetCharacterControl(ECharacterControlType::Quater);
	}
}

void AABCharacterPlayer::SetCharacterControl(ECharacterControlType NewCharacterControlType)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	UABCharacterControlData* NewCharacterControl = CharacterControlManager[NewCharacterControlType];
	check(NewCharacterControl);

	SetCharacterControlData(NewCharacterControl);

	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		UInputMappingContext* NewMappingContext = NewCharacterControl->InputMappingContext;
		if (NewMappingContext)
		{
			Subsystem->AddMappingContext(NewMappingContext, 0);
		}
	}

	CurrentCharacterControlType = NewCharacterControlType;
}

void AABCharacterPlayer::SetCharacterControlData(const UABCharacterControlData* CharacterControlData)
{
	Super::SetCharacterControlData(CharacterControlData);

	CameraBoom->TargetArmLength = CharacterControlData->TargetArmLength;
	CameraBoom->SetRelativeRotation(CharacterControlData->RelativeRotation);
	CameraBoom->bUsePawnControlRotation = CharacterControlData->bUsePawnControlRotation;
	CameraBoom->bInheritPitch = CharacterControlData->bInheritPitch;
	CameraBoom->bInheritYaw = CharacterControlData->bInheritYaw;
	CameraBoom->bInheritRoll = CharacterControlData->bInheritRoll;
	CameraBoom->bDoCollisionTest = CharacterControlData->bDoCollisionTest;
}

void AABCharacterPlayer::ShoulderMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void AABCharacterPlayer::ShoulderLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AABCharacterPlayer::QuaterMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	float InputSizeSquared = MovementVector.SquaredLength();
	float MovementVectorSize = 1.0f;
	float MovementVectorSizeSquared = MovementVector.SquaredLength();
	if (MovementVectorSizeSquared > 1.0f)
	{
		MovementVector.Normalize();
		MovementVectorSizeSquared = 1.0f;
	}
	else
	{
		MovementVectorSize = FMath::Sqrt(MovementVectorSizeSquared);
	}

	FVector MoveDirection = FVector(MovementVector.X, MovementVector.Y, 0.0f);
	GetController()->SetControlRotation(FRotationMatrix::MakeFromX(MoveDirection).Rotator());
	AddMovementInput(MoveDirection, MovementVectorSize);
}

void AABCharacterPlayer::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 프로퍼티 등록.
	DOREPLIFETIME(AABCharacterPlayer, bCanAttack);
}

void AABCharacterPlayer::Attack()
{
	//ProcessComboCommand();

	// 공격 가능한 경우.
	if (bCanAttack)
	{
		// 클라이언트.
		if (!HasAuthority())
		{
			bCanAttack = false;

			GetCharacterMovement()->SetMovementMode(
				EMovementMode::MOVE_None
			);

			// 공격 종료 처리를 위한 타이머 설정.
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(
				Handle,
				FTimerDelegate::CreateLambda([&]()
					{
						bCanAttack = true;

						GetCharacterMovement()->SetMovementMode(
							EMovementMode::MOVE_Walking
						);
					})
				, AttackTime, false
			);

			// 애니메이션 재생.
			PlayAttackAnimation();
		}

		// 서버에 공격 시작을 알림 (Server RPC 호출).
		// 이때 서버에 공격 시작한 시간을 전달.
		//float AttackStartTime = GetWorld()->GetTimeSeconds();
		// 서버 시간을 기준으로 공격 시작 시간 보내기.
		float AttackStartTime
			= GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
		ServerRPCAttack(AttackStartTime);
	}
}

void AABCharacterPlayer::PlayAttackAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(ComboActionMontage);
}

void AABCharacterPlayer::AttackHitCheck()
{
	// 입력을 전달한 클라이언트에서 공격 판정 진행.
	if (IsLocallyControlled())
	{
		// 로그 출력.
		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

		FHitResult OutHitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

		const float AttackRange = Stat->GetTotalStat().AttackRange;
		const float AttackRadius = Stat->GetAttackRadius();
		const float AttackDamage = Stat->GetTotalStat().Attack;
		const FVector Forward = GetActorForwardVector();
		const FVector Start
			= GetActorLocation()
			+ Forward * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector End = Start + Forward * AttackRange;

		bool HitDetected = GetWorld()->SweepSingleByChannel(
			OutHitResult,
			Start,
			End,
			FQuat::Identity,
			CCHANNEL_ABACTION,
			FCollisionShape::MakeSphere(AttackRadius),
			Params
		);

		// 충돌 검증을 진행한 시간.
		float HitCheckTime
			= GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

		// 클라이언트.
		if (!HasAuthority())
		{
			// 무언가 맞았을 때.
			if (HitDetected)
			{
				ServerRPCNotifyHit(OutHitResult, HitCheckTime);
			}

			// 안 맞았을 때.
			else
			{
				ServerRPCNotifyMiss(Start, End, Forward, HitCheckTime);
			}
		}

		// 서버.
		else
		{
			if (HitDetected)
			{
				AttackHitConfirm(OutHitResult.GetActor());
			}
		}

		//		if (HitDetected)
		//		{
		//			FDamageEvent DamageEvent;
		//			OutHitResult.GetActor()->TakeDamage(
		//				AttackDamage, DamageEvent, GetController(), this
		//			);
		//		}
		//
		//#if ENABLE_DRAW_DEBUG
		//
		//		FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
		//		float CapsuleHalfHeight = AttackRange * 0.5f;
		//		FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;
		//
		//		DrawDebugCapsule(
		//			GetWorld(),
		//			CapsuleOrigin,
		//			CapsuleHalfHeight,
		//			AttackRadius,
		//			FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(),
		//			DrawColor,
		//			false,
		//			5.0f
		//		);
		//
		//#endif
	}
}

void AABCharacterPlayer::AttackHitConfirm(AActor* HitActor)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// 이 로직은 서버에서 처리.
	if (HasAuthority())
	{
		// 공격 대미지.
		const float AttackDamage = Stat->GetTotalStat().Attack;

		FDamageEvent DamageEvent;
		HitActor->TakeDamage(
			AttackDamage,
			DamageEvent,
			GetController(),
			this
		);
	}
}

void AABCharacterPlayer::ServerRPCAttack_Implementation(float AttackStartTime)
{
	// 공격 시작 처리.
	bCanAttack = false;
	OnRep_CanAttack();

	// 서버-클라이언트의 시간 차이.
	AttackTimeDifference = GetWorld()->GetTimeSeconds() - AttackStartTime;

	// 로그 출력.
	AB_LOG(LogABNetwork, Log, TEXT("LagTime: %f"), AttackTimeDifference);

	// 시간 값 보정. 타이머가 실행될 수 있도록 약간의 오프셋 적용.
	AttackTimeDifference
		= FMath::Clamp(AttackTimeDifference, 0.0f, AttackTime - 0.01f);

	// 공격 종료 타이머 설정.
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateLambda([&]()
			{
				bCanAttack = false;
				OnRep_CanAttack();
			})
		, AttackTime - AttackTimeDifference, false
	);

	// 공격 처리 시간 기록.
	LastAttackStartTime = AttackStartTime;

	// 애니메이션 재생.
	PlayAttackAnimation();

	// 클라이언트로부터 요청 받은 공격 명령을
	// 다시 클라이언트에 전파 (서버 포함).
	MulticastRPCAttack();
}

bool AABCharacterPlayer::ServerRPCAttack_Validate(float AttackStartTime)
{
	// 공격 타이밍에 대한 검증 추가.
	// 너무 짧은 시간에 공격이 반복되지 않았는지를 확인.
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	// 현재 공격 시작한 시간과 이전에 공격했던 시간의 차이가
	// 공격 애니메이션 길이보다 크면 인정.
	return (AttackStartTime - LastAttackStartTime) > AttackTime;
}

void AABCharacterPlayer::MulticastRPCAttack_Implementation()
{
	//// 로그 출력.
	//AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	//// 서버 로직.
	//if (HasAuthority())
	//{
	//	// 공격 중이라고 설정.
	//	bCanAttack = false;

	//	// 서버에서는 OnRep_ 함수 호출이 안되기 때문에 직접 호출.
	//	OnRep_CanAttack();

	//	// 공격 종료 처리를 위해 타이머 사용.
	//	FTimerHandle Handle;
	//	GetWorld()->GetTimerManager().SetTimer(
	//		Handle,
	//		FTimerDelegate::CreateLambda([&]()
	//			{
	//				// 다시 공격 가능한 상태로 설정.
	//				bCanAttack = true;

	//				// 서버에서는 OnRep_ 함수 호출이 안되기 때문에 직접 호출.
	//				OnRep_CanAttack();
	//			}),
	//		AttackTime, false
	//	);
	//}

	// 본인 클라와 서버가 아닌 다른 클라이언트에서는 애니메이션 재생.
	if (!IsLocallyControlled())
	{
		PlayAttackAnimation();

	}
}

void AABCharacterPlayer::ServerRPCNotifyHit_Implementation(
	const FHitResult& HitResult, float HitCheckTime)
{
	// 맞은 액터.
	AActor* HitActor = HitResult.GetActor();
	if (HitActor)
	{
		// 맞은 곳의 정보를 활용해 로직 검증.
		const FVector HitLocation = HitResult.Location;

		// 현재 캐릭터의 바운딩 박스 정보.
		const FBox HitBox = HitActor->GetComponentsBoundingBox();

		// 바운딩 박스 가운데 값.
		const FVector ActorBoxCenter = HitBox.GetCenter();

		// 문제가 있는지 검증.
		if (FVector::DistSquared(HitLocation, ActorBoxCenter)
			<= )
	}
}

bool AABCharacterPlayer::ServerRPCNotifyHit_Validate(
	const FHitResult& HitResult, float HitCheckTime)
{
	return true;
}

void AABCharacterPlayer::ServerRPCNotifyMiss_Implementation(
	FVector TraceStart,
	FVector TraceEnd,
	FVector TraceDir,
	float HitCheckTime)
{
}

bool AABCharacterPlayer::ServerRPCNotifyMiss_Validate(
	FVector TraceStart,
	FVector TraceEnd,
	FVector TraceDir,
	float HitCheckTime)
{
	return true;
}

void AABCharacterPlayer::OnRep_CanAttack()
{
	if (!bCanAttack)
	{
		// 공격 시에 이동하지 않도록 처리.
		GetCharacterMovement()->SetMovementMode(
			EMovementMode::MOVE_None
		);
	}
	else
	{
		// 공격이 종료되면 다시 이동 가능하도록 모드 설정.
		GetCharacterMovement()->SetMovementMode(
			EMovementMode::MOVE_Walking
		);
	}
}

void AABCharacterPlayer::SetupHUDWidget(UABHUDWidget* InHUDWidget)
{
	if (InHUDWidget)
	{
		InHUDWidget->UpdateStat(Stat->GetBaseStat(), Stat->GetModifierStat());
		InHUDWidget->UpdateHpBar(Stat->GetCurrentHp());

		Stat->OnStatChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateStat);
		Stat->OnHpChanged.AddUObject(InHUDWidget, &UABHUDWidget::UpdateHpBar);
	}
}
