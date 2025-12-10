// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ABGameState.generated.h"

/**
 *
 */
UCLASS()
class ARENABATTLE_API AABGameState : public AGameState
{
	GENERATED_BODY()

public:
	AABGameState();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

	//virtual void HandleBeginPlay() override;
	//virtual void OnRep_ReplicatedHasBegunPlay() override;

	// 경기 시간 추적용.
	UPROPERTY(Transient, Replicated)
	int32 RemainingTime;

	// 플레이 시간.
	int32 MatchPlayTime = 2000;

	// 게임 종료 후 결과를 보여줄 때까지 잠깐 대기할 시간.
	int32 ShowResultWaitingTime = 5;
};
