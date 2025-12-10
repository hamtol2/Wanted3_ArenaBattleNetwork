// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ABGameMode.h"
#include "ABGameMode.h"
#include "Player/ABPlayerController.h"
#include "ArenaBattle.h"
#include "ABGameState.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "ABPlayerState.h"

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

	// 플레이어 스테이트 클래스 지정.
	PlayerStateClass = AABPlayerState::StaticClass();
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
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	// 플레이어 스테이트가 가진 기본 값을 사용해 점수 처리.
	APlayerState* KillerPlayerState = Killer->PlayerState;
	if (KillerPlayerState)
	{
		// 1점 획득 처리.
		KillerPlayerState->SetScore(
			KillerPlayerState->GetScore() + 1
		);

		// 타겟 킬수 확인 (3킬).
		if (KillerPlayerState->GetScore() > 2)
		{
			// 경기 종료처리.
			// 아래 함수 호출 후에 5초 뒤에 ServerTravel.
			FinishMatch();
		}
	}
}

void AABGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 1초마다 시간을 계산할 타이머 설정.
	//GetWorldTimerManager()
	GetWorld()->GetTimerManager().SetTimer(
		GameTimerHandle,
		this,
		&AABGameMode::DefaultGameTimer,
		GetWorldSettings()->GetEffectiveTimeDilation(),	// 1초.
		true
	);
}

void AABGameMode::DefaultGameTimer()
{
	// 남은 경기 시간 계산.
	AABGameState* const ABGameState
		= Cast<AABGameState>(GameState);
	if (ABGameState && ABGameState->RemainingTime > 0)
	{
		// 남은 시간 감소 처리.
		ABGameState->RemainingTime -= 1;

		// 로그 출력.
		AB_LOG(
			LogABNetwork,
			Log,
			TEXT("Remaining Time: %d"),
			ABGameState->RemainingTime
		);

		// 남은 시간이 없으면, 매치 상태 변경.
		if (ABGameState->RemainingTime <= 0)
		{
			// 경기 상태 확인 후 처리.
			// 경기가 진행 중이라면, 경기 종료 처리.
			if (GetMatchState() == MatchState::InProgress)
			{
				FinishMatch();
			}
			// 이미 경기 종료 후 다음 경기를 기다리는 경우라면,
			// 다른 월드로 이동.
			else if (GetMatchState() == MatchState::WaitingPostMatch)
			{
				// 서버 트래블.
				// 클라이언트의 접속을 유지시킨채로 다른 레벨로 이동시킴.
				GetWorld()->ServerTravel(
					TEXT("/Game/ArenaBattle/Maps/Part3Step3?listen")
				);
			}
		}
	}
}

void AABGameMode::FinishMatch()
{
	// 경기 종료 처리.
	AABGameState* const ABGameState
		= Cast<AABGameState>(GameState);
	if (ABGameState && IsMatchInProgress())
	{
		// 경기 종료 함수 호출.
		// 이 함수를 호출하면 WaitingPostMatch 상태로 전환됨.
		EndMatch();

		// 경기 종료 후 레벨을 이동하기 전에 잠시 대기할 수 있게 시간 설정.
		ABGameState->RemainingTime
			= ABGameState->ShowResultWaitingTime;
	}
}
