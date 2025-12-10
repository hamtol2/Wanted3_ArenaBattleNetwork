// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ABGameMode.h"
#include "ABGameMode.h"
#include "Player/ABPlayerController.h"
#include "ArenaBattle.h"
#include "ABGameState.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

AABGameMode::AABGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> DefaultPawnClassRef(TEXT("/Script/Engine.Blueprint'/Game/ArenaBattle/Blueprint/BP_ABCharacterPlayer.BP_ABCharacterPlayer_C'"));
	if (DefaultPawnClassRef.Class)
	{
		DefaultPawnClass = DefaultPawnClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(TEXT("/Script/ArenaBattle.ABPlayerController"));
	if (PlayerControllerClassRef.Class)
	{
		PlayerControllerClass = PlayerControllerClassRef.Class;
	}

	GameStateClass = AABGameState::StaticClass();
}

//void AABGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("==========================="));
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//
//	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
//
//	//ErrorMessage = TEXT("Server is full");
//
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
//}
//
//APlayerController* AABGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//
//	APlayerController* NewPlayerController = Super::Login(
//		NewPlayer,
//		InRemoteRole,
//		Portal,
//		Options,
//		UniqueId,
//		ErrorMessage
//	);
//
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
//
//	return NewPlayerController;
//}
//
//void AABGameMode::PostLogin(APlayerController* NewPlayer)
//{
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
//
//	Super::PostLogin(NewPlayer);
//
//	// NetDriver 가져오기.
//	UNetDriver* NetDriver = GetNetDriver();
//	if (NetDriver)
//	{
//		// 클라 접속이 없는 경우.
//		if (NetDriver->ClientConnections.Num() == 0)
//		{
//			AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No client connection"));
//		}
//
//		// 클라 접속이 있는 경우.
//		else
//		{
//			// 접속된 클라이언트의 이름 출력.
//			for (const auto& Connection : NetDriver->ClientConnections)
//			{
//				AB_LOG(
//					LogABNetwork, 
//					Log, 
//					TEXT("Client Connections: %s"), 
//					*Connection->GetName()
//				);
//			}
//		}
//	}
//	else
//	{
//		AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No NetDriver"));
//	}
//
//	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
//}

void AABGameMode::StartPlay()
{
	Super::StartPlay();

	// 월드에 있는 PlayerStart 액터를 검색해서 배열에 추가.
	for (APlayerStart* PlayerStart
		: TActorRange<APlayerStart>(GetWorld()))
	{
		// 배열에 추가.
		PlayerStartArray.Add(PlayerStart);
	}
}

FTransform AABGameMode::GetRandomStartTransform() const
{
	// 예외처리.
	if (PlayerStartArray.Num() == 0)
	{
		return FTransform(FVector(0.0f, 0.0f, 230.0f));
	}

	// 랜덤 인덱스 선택.
	int32 RandIndex = FMath::RandRange(0, PlayerStartArray.Num() - 1);

	// 선택한 인덱스에 해당하는 PlayerStart의 트랜스폼 반환.
	return PlayerStartArray[RandIndex]->GetActorTransform();
}

void AABGameMode::OnPlayerKilled(
	AController* Killer,
	AController* KilledPlayer,
	APawn* KilledPawn)
{

}
