// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Interface/ABGameInterface.h"
#include "ABGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABGameMode 
	: public AGameMode, 
	public IABGameInterface
{
	GENERATED_BODY()
	
public:
	AABGameMode();

	//virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	//virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	//virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void StartPlay() override;

	virtual FTransform GetRandomStartTransform() const override;

	virtual void OnPlayerKilled(
		AController* Killer,
		AController* KilledPlayer,
		APawn* KilledPawn
	) override;

protected:
	// 액터가 준비되면 호출되는 (게임 플레이와 무관) 이벤트 함수.
	virtual void PostInitializeComponents() override;

	// 타이머에 사용할 함수.
	virtual void DefaultGameTimer();

	// 경기를 종료할 때 호출할 함수.
	virtual void FinishMatch();

	// 타이머 핸들.
	FTimerHandle GameTimerHandle;

protected:
	// 플레이어 스타트 액터 배열.
	TArray<TObjectPtr<class APlayerStart>> PlayerStartArray;
};
