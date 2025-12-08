// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ABCharacterMovementComponent.h"
#include "ArenaBattle.h"
#include "GameFramework/Character.h"

UABCharacterMovementComponent::UABCharacterMovementComponent()
{
	// 텔레포트 거리 값 (600cm).
	TeleportOffset = 600.0f;

	// 텔레포트 쿨타임 (3초).
	TeleportCooltime = 3.0f;

	// 불리언 값 초기화.
	bPressedTeleport = false;
	bDidTeleport = false;
}

void UABCharacterMovementComponent::SetTeleportCommand()
{
	// 텔레포트 명령이 전달되면 bPressedTeleport을 true로 설정.
	bPressedTeleport = true;
}

void UABCharacterMovementComponent::ABTeleport()
{
	// 텔레포트 수행.

	if (CharacterOwner)
	{
		AB_SUBLOG(
			LogABTeleport, 
			Log, 
			TEXT("%s"), 
			TEXT("Teleport Begin")
		);

		// 앞 방향으로 TeleportOffset 거리만큼 떨어진 위치를 목표 위치로.
		FVector TargetLocation
			= CharacterOwner->GetActorLocation()
			+ CharacterOwner->GetActorForwardVector() * TeleportOffset;

		CharacterOwner->TeleportTo(
			TargetLocation,
			CharacterOwner->GetActorRotation(),
			false,
			true
		);

		// 텔레포트 진행중으로 상태 변경.
		bDidTeleport = true;

		// 쿨타임 적용.
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda([&]()
			{
				// 쿨타임이 종료되면 다시 false로 설정.
				bDidTeleport = false;

				AB_SUBLOG(
					LogABTeleport, 
					Log, 
					TEXT("%s"), 
					TEXT("Teleport End")
				);
			})
			, TeleportCooltime, false
		);
	}
}

void UABCharacterMovementComponent::OnMovementUpdated(
	float DeltaSeconds, 
	const FVector& OldLocation, 
	const FVector& OldVelocity)
{
	// 텔레포트 실행 조건 확인 후 실행.
	if (bPressedTeleport && !bDidTeleport)
	{
		ABTeleport();
	}

	if (bPressedTeleport)
	{
		bPressedTeleport = false;
	}
}

FNetworkPredictionData_Client* UABCharacterMovementComponent::GetPredictionData_Client() const
{
	//Super::GetPredictionData_Client();

	if (ClientPredictionData == nullptr)
	{
		UABCharacterMovementComponent* MutableThis 
			= const_cast<UABCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData 
			= new FABNetworkPredictionData_Client_Character(*this);
	}

	return ClientPredictionData;
}

void UABCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	// 클라이언트에서 인코딩한 상태 값을 디코딩.
	bPressedTeleport 
		= ((Flags & FSavedMove_Character::FLAG_Custom_0) != 0);

	bDidTeleport
		= ((Flags & FSavedMove_Character::FLAG_Custom_1) != 0);

	// 조건 확인해서 텔레포트 수행 (서버 확인).
	if (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		if (bPressedTeleport && !bDidTeleport)
		{
			AB_SUBLOG(LogABTeleport, Log, TEXT("%s"), TEXT("Teleport Begin"));

			ABTeleport();
		}
	}
}

FABNetworkPredictionData_Client_Character::FABNetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FABNetworkPredictionData_Client_Character::AllocateNewMove()
{
	return FSavedMovePtr(new FABSavedMove_Character());
}

void FABSavedMove_Character::Clear()
{
	Super::Clear();

	// 프로퍼티 초기화.
	bPressedTeleport = false;
	bDidTeleport = false;
}

void FABSavedMove_Character::SetInitialPosition(ACharacter* Character)
{
	Super::SetInitialPosition(Character);

	UABCharacterMovementComponent* ABMovement = Cast<UABCharacterMovementComponent>(Character->GetCharacterMovement());
	if (ABMovement)
	{
		bPressedTeleport = ABMovement->bPressedTeleport;
		bDidTeleport = ABMovement->bDidTeleport;
	}
}

uint8 FABSavedMove_Character::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	/*
		FLAG_JumpPressed	= 0x01,	// Jump pressed
		FLAG_WantsToCrouch	= 0x02,	// Wants to crouch
		FLAG_Reserved_1		= 0x04,	// Reserved for future use
		FLAG_Reserved_2		= 0x08,	// Reserved for future use
		// Remaining bit masks are available for custom flags.
		FLAG_Custom_0		= 0x10,
		FLAG_Custom_1		= 0x20,
		FLAG_Custom_2		= 0x40,
		FLAG_Custom_3		= 0x80,
	*/

	if (bPressedTeleport)
	{
		Result |= FLAG_Custom_0;
	}

	if (bDidTeleport)
	{
		Result |= FLAG_Custom_1;
	}

	return Result;
}
