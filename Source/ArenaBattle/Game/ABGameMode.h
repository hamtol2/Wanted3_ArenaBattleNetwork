// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interface/ABGameInterface.h"
#include "ABGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABGameMode 
	: public AGameModeBase, 
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
	// 플레이어 스타트 액터 배열.
	TArray<TObjectPtr<class APlayerStart>> PlayerStartArray;
};
