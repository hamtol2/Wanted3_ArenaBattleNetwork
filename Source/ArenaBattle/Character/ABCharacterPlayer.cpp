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
#include "EngineUtils.h"
#include "ABCharacterMovementComponent.h"

AABCharacterPlayer::AABCharacterPlayer(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UABCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
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

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionTeleportRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ArenaBattle/Input/Actions/IA_Teleport.IA_Teleport'"));
	if (nullptr != InputActionTeleportRef.Object)
	{
		TeleportAction = InputActionTeleportRef.Object;
	}

	CurrentCharacterControlType = ECharacterControlType::Quater;

	// ������ ���� ���� �������� ����.
	bCanAttack = true;

	// ���ø����̼� Ȱ��ȭ.
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

	// PossessedBy ȣ��Ǳ� �� ������ ���� Ȯ��.
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

	// PossessedBy ȣ��� �� ������ ���� Ȯ��.
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

	// OnRep_Owner �Լ� ȣ�� �� ���� Ȯ��.
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
	EnhancedInputComponent->BindAction(TeleportAction, ETriggerEvent::Triggered, this, &AABCharacterPlayer::Teleport);
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
	// ���� �߿��� �̵� ���ϵ��� ó��.
	if (!bCanAttack)
	{
		return;
	}

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
	// ���� �߿��� �̵� ���ϵ��� ó��.
	if (!bCanAttack)
	{
		return;
	}

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

	// ������Ƽ ���.
	DOREPLIFETIME(AABCharacterPlayer, bCanAttack);
}

void AABCharacterPlayer::Attack()
{
	//ProcessComboCommand();

	// ���� ������ ���.
	if (bCanAttack)
	{
		// Ŭ���̾�Ʈ.
		if (!HasAuthority())
		{
			bCanAttack = false;

			GetCharacterMovement()->SetMovementMode(
				EMovementMode::MOVE_None
			);

			// ���� ���� ó���� ���� Ÿ�̸� ����.
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

			// �ִϸ��̼� ���.
			PlayAttackAnimation();
		}

		// ������ ���� ������ �˸� (Server RPC ȣ��).
		// �̶� ������ ���� ������ �ð��� ����.
		//float AttackStartTime = GetWorld()->GetTimeSeconds();
		// ���� �ð��� �������� ���� ���� �ð� ������.
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
	// �Է��� ������ Ŭ���̾�Ʈ���� ���� ���� ����.
	if (IsLocallyControlled())
	{
		// �α� ���.
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

		// �浹 ������ ������ �ð�.
		float HitCheckTime
			= GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

		// Ŭ���̾�Ʈ.
		if (!HasAuthority())
		{
			// ���� �¾��� ��.
			if (HitDetected)
			{
				ServerRPCNotifyHit(OutHitResult, HitCheckTime);
			}

			// �� �¾��� ��.
			else
			{
				ServerRPCNotifyMiss(Start, End, Forward, HitCheckTime);
			}
		}

		// ����.
		else
		{
			FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;
			DrawDebugAttackRange(
				DrawColor,
				Start,
				End,
				Forward
			);

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

	// �� ������ �������� ó��.
	if (HasAuthority())
	{
		// ���� �����.
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

void AABCharacterPlayer::DrawDebugAttackRange(const FColor& DrawColor, FVector TraceStart, FVector TraceEnd, FVector Forward)
{
#if ENABLE_DRAW_DEBUG

	const float AttackRange = Stat->GetTotalStat().AttackRange;
	const float AttackRadius = Stat->GetAttackRadius();

	FVector CapsuleOrigin = TraceStart + (TraceEnd - TraceStart) * 0.5f;
	float CapsuleHalfHeight = AttackRange * 0.5f;

	DrawDebugCapsule(
		GetWorld(),
		CapsuleOrigin,
		CapsuleHalfHeight,
		AttackRadius,
		FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(),
		DrawColor,
		false,
		5.0f
	);

#endif
}

void AABCharacterPlayer::ClientRPCPlayAnimation_Implementation(
	AABCharacterPlayer* CharacterToPlay)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	if (CharacterToPlay)
	{
		CharacterToPlay->PlayAttackAnimation();
	}
}

void AABCharacterPlayer::ServerRPCAttack_Implementation(float AttackStartTime)
{
	// ���� ���� ó��.
	bCanAttack = false;
	OnRep_CanAttack();

	// ����-Ŭ���̾�Ʈ�� �ð� ����.
	AttackTimeDifference = GetWorld()->GetTimeSeconds() - AttackStartTime;

	// �α� ���.
	AB_LOG(LogABNetwork, Log, TEXT("LagTime: %f"), AttackTimeDifference);

	// �ð� �� ����. Ÿ�̸Ӱ� ����� �� �ֵ��� �ణ�� ������ ����.
	AttackTimeDifference
		= FMath::Clamp(AttackTimeDifference, 0.0f, AttackTime - 0.01f);

	// ���� ���� Ÿ�̸� ����.
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateLambda([&]()
			{
				bCanAttack = true;
				OnRep_CanAttack();
			})
		, AttackTime - AttackTimeDifference, false
	);

	// ���� ó�� �ð� ���.
	LastAttackStartTime = AttackStartTime;

	// �ִϸ��̼� ���.
	PlayAttackAnimation();

	// Ŭ���̾�Ʈ�κ��� ��û ���� ���� ������
	// �ٽ� Ŭ���̾�Ʈ�� ���� (���� ����).
	//MulticastRPCAttack();

	// PlayerController ��ȸ.
	for (auto PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		// ���� �Ÿ���.
		if (PlayerController && GetController() != PlayerController)
		{
			if (!PlayerController->IsLocalController())
			{
				// SimulatedProxy���� �޽��� ����.
				AABCharacterPlayer* OtherPlayer 
					= Cast<AABCharacterPlayer>(PlayerController->GetPawn());
				if (OtherPlayer)
				{
					OtherPlayer->ClientRPCPlayAnimation(this);
				}
			}
		}
	}
}

bool AABCharacterPlayer::ServerRPCAttack_Validate(float AttackStartTime)
{
	// ���� Ÿ�ֿ̹� ���� ���� �߰�.
	// �ʹ� ª�� �ð��� ������ �ݺ����� �ʾҴ����� Ȯ��.
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	// ���� ���� ������ �ð��� ������ �����ߴ� �ð��� ���̰�
	// ���� �ִϸ��̼� ���̺��� ũ�� ����.
	return (AttackStartTime - LastAttackStartTime) > AttackTime;
}

void AABCharacterPlayer::MulticastRPCAttack_Implementation()
{
	//// �α� ���.
	//AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	//// ���� ����.
	//if (HasAuthority())
	//{
	//	// ���� ���̶�� ����.
	//	bCanAttack = false;

	//	// ���������� OnRep_ �Լ� ȣ���� �ȵǱ� ������ ���� ȣ��.
	//	OnRep_CanAttack();

	//	// ���� ���� ó���� ���� Ÿ�̸� ���.
	//	FTimerHandle Handle;
	//	GetWorld()->GetTimerManager().SetTimer(
	//		Handle,
	//		FTimerDelegate::CreateLambda([&]()
	//			{
	//				// �ٽ� ���� ������ ���·� ����.
	//				bCanAttack = true;

	//				// ���������� OnRep_ �Լ� ȣ���� �ȵǱ� ������ ���� ȣ��.
	//				OnRep_CanAttack();
	//			}),
	//		AttackTime, false
	//	);
	//}

	// ���� Ŭ��� ������ �ƴ� �ٸ� Ŭ���̾�Ʈ������ �ִϸ��̼� ���.
	if (!IsLocallyControlled())
	{
		PlayAttackAnimation();

	}
}

void AABCharacterPlayer::ServerRPCNotifyHit_Implementation(
	const FHitResult& HitResult, float HitCheckTime)
{
	// ���� ����.
	AActor* HitActor = HitResult.GetActor();
	if (HitActor)
	{
		// ���� ���� ������ Ȱ���� ���� ����.
		const FVector HitLocation = HitResult.Location;

		// ���� ĳ������ �ٿ�� �ڽ� ����.
		const FBox HitBox = HitActor->GetComponentsBoundingBox();

		// �ٿ�� �ڽ� ��� ��.
		const FVector ActorBoxCenter = HitBox.GetCenter();

		// ������ �ִ��� ����.
		if (FVector::DistSquared(HitLocation, ActorBoxCenter)
			<= AcceptCheckDistance * AcceptCheckDistance)
		{
			// ���� ���.
			AttackHitConfirm(HitActor);
		}
	}
}

bool AABCharacterPlayer::ServerRPCNotifyHit_Validate(
	const FHitResult& HitResult, float HitCheckTime)
{
	// ������ ������ ���� ���ٸ� ���� ����.
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	// ���� ������ �Ŀ� ���� �������� �ɸ� �ð� ����
	// ������ ������ Ȯ��.
	return (HitCheckTime - LastAttackStartTime) > AcceptMinCheckTime;
}

void AABCharacterPlayer::ServerRPCNotifyMiss_Implementation(
	FVector_NetQuantizeNormal TraceStart,
	FVector_NetQuantizeNormal TraceEnd,
	FVector_NetQuantizeNormal TraceDir,
	float HitCheckTime)
{
}

bool AABCharacterPlayer::ServerRPCNotifyMiss_Validate(
	FVector_NetQuantizeNormal TraceStart,
	FVector_NetQuantizeNormal TraceEnd,
	FVector_NetQuantizeNormal TraceDir,
	float HitCheckTime)
{
	// ������ ������ ���� ���ٸ� ���� ����.
	if (LastAttackStartTime == 0.0f)
	{
		return true;
	}

	// ���� ������ �Ŀ� ���� �������� �ɸ� �ð� ����
	// ������ ������ Ȯ��.
	return (HitCheckTime - LastAttackStartTime) > AcceptMinCheckTime;
}

void AABCharacterPlayer::OnRep_CanAttack()
{
	if (!bCanAttack)
	{
		// ���� �ÿ� �̵����� �ʵ��� ó��.
		GetCharacterMovement()->SetMovementMode(
			EMovementMode::MOVE_None
		);
	}
	else
	{
		// ������ ����Ǹ� �ٽ� �̵� �����ϵ��� ��� ����.
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

void AABCharacterPlayer::Teleport()
{
	AB_LOG(LogABTeleport, Log, TEXT("%s"), TEXT("Begin"));

	UABCharacterMovementComponent* ABMovement
		= Cast<UABCharacterMovementComponent>(GetCharacterMovement());
	if (ABMovement)
	{
		ABMovement->SetTeleportCommand();
	}
}
