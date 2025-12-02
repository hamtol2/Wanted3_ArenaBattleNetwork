// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/ABFountain.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "ArenaBattle.h"

// Sets default values
AABFountain::AABFountain()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Water = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Water"));

	RootComponent = Body;
	Water->SetupAttachment(Body);
	Water->SetRelativeLocation(FVector(0.0f, 0.0f, 132.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01'"));
	if (BodyMeshRef.Object)
	{
		Body->SetStaticMesh(BodyMeshRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WaterMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Fountain_02.SM_Plains_Fountain_02'"));
	if (WaterMeshRef.Object)
	{
		Water->SetStaticMesh(WaterMeshRef.Object);
	}

	// 리플리케이션 활성화.
	bReplicates = true;
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();

	//// 서버 로직.
	//if (HasAuthority())
	//{
	//	// 타이머를 설정해서 반복적으로 같은 값 설정.
	//	FTimerHandle Handle;
	//	GetWorld()->GetTimerManager().SetTimer(
	//		Handle,
	//		FTimerDelegate::CreateLambda([&]()
	//		{
	//			ServerRotationYaw += 1.0f;
	//		}),
	//		1.0f,
	//		true
	//	);
	//}

}

void AABFountain::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 복제할 속성을 매크로를 통해 지정.
	DOREPLIFETIME(AABFountain, ServerRotationYaw);
}

void AABFountain::OnActorChannelOpen(
	FInBunch& InBunch, UNetConnection* Connection)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::OnActorChannelOpen(InBunch, Connection);

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

void AABFountain::OnRep_ServerRotationYaw()
{
	AB_LOG(LogABNetwork, Log, TEXT("Yaw: %f"), ServerRotationYaw);

	// 변경된 회전 값을 반영해 새로운 회전 값 생성 후 적용.
	FRotator NewRotator = RootComponent->GetComponentRotation();
	NewRotator.Yaw = ServerRotationYaw;

	RootComponent->SetWorldRotation(NewRotator);
}

// Called every frame
void AABFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 액터 회전 처리.

	// 게임 로직은 서버에서 처리.
	// 이를 위해 현재 실행되는 곳이 서버인지 확인.
	if (HasAuthority())
	{
		// 회전 적용.
		AddActorLocalRotation(FRotator(0.0f, RotationRate * DeltaTime, 0.0f));

		// 변경된 회전 값을 프로퍼티에 저장.
		ServerRotationYaw = RootComponent->GetComponentRotation().Yaw;
	}
	// 클라이언트.
	//else
	//{
	//	// 변경된 회전 값을 반영해 새로운 회전 값 생성 후 적용.
	//	FRotator NewRotator = RootComponent->GetComponentRotation();
	//	NewRotator.Yaw = ServerRotationYaw;

	//	RootComponent->SetWorldRotation(NewRotator);
	//}
}

