// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ABCharacterMovementComponent.generated.h"

class FABNetworkPredictionData_Client_Character
	: public FNetworkPredictionData_Client_Character
{
	// 편의를 위해서 부모 클래스 알리아싱 지정.
	using Super = FNetworkPredictionData_Client_Character;

public:
	FABNetworkPredictionData_Client_Character(
		const UCharacterMovementComponent& ClientMovement
	);

	virtual FSavedMovePtr AllocateNewMove() override;
};

// 움직임 데이터를 변경하는데 사용할 클래스.
class FABSavedMove_Character : public FSavedMove_Character
{
	using Super = FSavedMove_Character;

public:

	// 객체가 재활용 가능하도록 리셋 함수 오버라이드.
	virtual void Clear() override;

	/** Set the properties describing the position, etc. of the moved pawn at the start of the move. */
	virtual void SetInitialPosition(ACharacter* Character) override;

	/** Returns a byte containing encoded special movement information (jumping, crouching, etc.)	 */
	virtual uint8 GetCompressedFlags() const override;

	// 텔레포트 상태 값을 저장할 변수.
	uint8 bPressedTeleport : 1;
	uint8 bDidTeleport : 1;
};

/**
 * 
 */
UCLASS()
class ARENABATTLE_API UABCharacterMovementComponent 
	: public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	// 생성자.
	UABCharacterMovementComponent();

	// 받은 명령(T키 입력)을 설정하는 함수.
	void SetTeleportCommand();

protected:
	// 텔레포트 실행 함수.
	virtual void ABTeleport();

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

public:
	// 현재 텔레포트 명령이 들어왔는지 확인하는 변수.
	uint8 bPressedTeleport : 1;

	// 텔레포트 진행 여부를 판단하는데 사용할 변수.
	uint8 bDidTeleport : 1;

protected:
	// 텔레포트 거리를 지정하는 변수.
	UPROPERTY()
	float TeleportOffset;

	// 텔레포트 쿨타임.
	UPROPERTY()
	float TeleportCooltime;
};
