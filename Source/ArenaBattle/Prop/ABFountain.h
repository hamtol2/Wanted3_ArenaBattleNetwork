// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ABFountain.generated.h"

UCLASS()
class ARENABATTLE_API AABFountain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AABFountain();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 네트워크로 복제할 속성을 추가할 때 사용할 함수.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 액터 채널 열릴 떄 호출되는 함수.
	virtual void OnActorChannelOpen(
		class FInBunch& InBunch, 
		class UNetConnection* Connection) override;

	// 연관성 처리 함수 오버라이드.
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	UFUNCTION()
	void OnRep_ServerRotationYaw();

	UFUNCTION()
	void OnRep_ServerLightColor();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<class UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<class UStaticMeshComponent> Water;

	// 리플리케이션 옵션 설정한 프로퍼티.
	UPROPERTY(ReplicatedUsing = OnRep_ServerRotationYaw)
	float ServerRotationYaw;

	// 회전 속도 변수.
	float RotationRate = 30.0f;

	// 클라이언트에서 회전 보간을 처리하기위한 변수.
	// 서버로부터 패킷을 받은 후에 지난 시간.
	float ClientTimeSinceUpdate = 0.0f;

	// 서버로부터 데이터를 받고, 그 다음에 데이터를 받기까지 걸린 시간.
	float ClientTimeBetweenLastUpdate = 0.0f;

	// 의도적으로 네트워크를 포화상태로 만들기 위한 변수.
	//UPROPERTY(Replicated)
	//TArray<float> BigData;
	
	//// 값 설정에 사용할 변수.
	//float BigDataElement = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ServerLightColor)
	FLinearColor ServerLightColor;
};
