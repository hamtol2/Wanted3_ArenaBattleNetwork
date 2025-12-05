// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ABCharacterBase.h"
#include "InputActionValue.h"
#include "Interface/ABCharacterHUDInterface.h"
#include "ABCharacterPlayer.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABCharacterPlayer : public AABCharacterBase, public IABCharacterHUDInterface
{
	GENERATED_BODY()
	
public:
	AABCharacterPlayer();

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void SetDead() override;
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void OnRep_Owner() override;
	virtual void PostNetInit() override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

// Character Control Section
protected:
	void ChangeCharacterControl();
	void SetCharacterControl(ECharacterControlType NewCharacterControlType);
	virtual void SetCharacterControlData(const class UABCharacterControlData* CharacterControlData) override;

// Camera Section
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

// Input Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ChangeControlAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShoulderMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShoulderLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuaterMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;

	void ShoulderMove(const FInputActionValue& Value);
	void ShoulderLook(const FInputActionValue& Value);

	void QuaterMove(const FInputActionValue& Value);

	ECharacterControlType CurrentCharacterControlType;

	// 프로퍼티를 리플리케이션에 등록.
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Attack();

	// 공격 애니메이션 재생 함수 (재사용 가능하도록).
	void PlayAttackAnimation();

	// ABCharacterBAse에 있는 AttackHitCheck 오버라이드.
	virtual void AttackHitCheck() override;

	// 공격 판정 확인 함수.
	void AttackHitConfirm(AActor* HitActor);

	// 충돌 감지 영역을 그릴 때 사용할 디버깅 함수.
	void DrawDebugAttackRange(
		const FColor& DrawColor, 
		FVector TraceStart,
		FVector TraceEnd,
		FVector Forward
	);

	// Client -> Server 공격 명령 처리 요청에 사용되는 Server RPC.
	// 클라이언트에서 공격을 시작한 시간을 보내도록 함수 업데이트.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCAttack(float AttackStartTime);

	// 클라이언트(서버 포함)에 공격 명령 전달을 위한 Multicast RPC.
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCAttack();

	UFUNCTION(Client, Unreliable)
	void ClientRPCPlayAnimation(AABCharacterPlayer* CharacterToPlay);

	// 클라이언트에서 충돌 판정을 한 뒤에 무언가 맞았을 때 호출하는 함수.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCNotifyHit(
		const FHitResult& HitResult, float HitCheckTime
	);

	// 클라이언트에서 충돌 판정을 한 뒤에 안 맞았을 때 호출하는 함수.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCNotifyMiss(
		FVector_NetQuantizeNormal TraceStart,
		FVector_NetQuantizeNormal TraceEnd,
		FVector_NetQuantizeNormal TraceDir,
		float HitCheckTime
	);

	UFUNCTION()
	void OnRep_CanAttack();

	// 현재 공격 중인지 판단하는 변수.
	UPROPERTY(ReplicatedUsing = OnRep_CanAttack)
	uint8 bCanAttack : 1;

	// 첫 번째 공격 애니메이션 재생 길이(단위: 초).
	float AttackTime = 1.4667f;

	// 이전에 공격 시작한 시간 기록용.
	float LastAttackStartTime = 0.0f;

	// 클라이언트와 서버의 시간 차이 계산용 변수.
	float AttackTimeDifference = 0.0f;

	// 공격 판정에 사용할 거리 값(3미터).
	float AcceptCheckDistance = 300.0f;

	// 공격 판정은 애니메이션 몽타주의 노티파이로 진행.
	// 이 시간이 대략 0.233초 정도.
	float AcceptMinCheckTime = 0.15f;

// UI Section
protected:
	virtual void SetupHUDWidget(class UABHUDWidget* InHUDWidget) override;
};
