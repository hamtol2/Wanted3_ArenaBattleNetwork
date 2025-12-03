// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/ABFountain.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "ArenaBattle.h"
#include "Components/PointLightComponent.h"

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

	// 네트워크 전송 빈도 낮추기 (1초에 한 번).
	SetNetUpdateFrequency(1.0f);

	// NetCullDistance 값 낮게 설정.
	SetNetCullDistanceSquared(4000000.0f);
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();

	// 서버 로직.
	if (HasAuthority())
	{
		// 타이머를 설정해서 반복적으로 같은 값 설정.
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda([&]()
				{
					//ServerRotationYaw += 1.0f;

					//// 4000 바이트의 데이터 설정.
					//BigData.Init(BigDataElement, 1000);
					//
					//// 지속적으로 전달하기 위해 값 변경.
					//BigDataElement += 1.0f;

					// 라이트 색상 변경.
					ServerLightColor = FLinearColor(
						FMath::RandRange(0.0f, 1.0f),
						FMath::RandRange(0.0f, 1.0f),
						FMath::RandRange(0.0f, 1.0f),
						1.0f
					);

					// OnRep_ 함수는 서버에서 실행이 안되기 때문에.
					// 명시적으로 호출.
					OnRep_ServerLightColor();
				}),
			1.0f,
			true
		);
	}

}

void AABFountain::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 복제할 속성을 매크로를 통해 지정.
	DOREPLIFETIME(AABFountain, ServerRotationYaw);

	// 데이터 전송 테스트를 위한 변수 등록.
	//DOREPLIFETIME(AABFountain, BigData);

	DOREPLIFETIME(AABFountain, ServerLightColor);
}

void AABFountain::OnActorChannelOpen(
	FInBunch& InBunch, UNetConnection* Connection)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::OnActorChannelOpen(InBunch, Connection);

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

bool AABFountain::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	bool NetRelevantResult
		= Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);

	if (!NetRelevantResult)
	{
		AB_LOG(
			LogABNetwork,
			Log,
			TEXT("Not Relevant: [%s] %s"),
			*RealViewer->GetName(),
			*SrcLocation.ToCompactString()
		);
	}

	return NetRelevantResult;
}

void AABFountain::OnRep_ServerRotationYaw()
{
	AB_LOG(LogABNetwork, Log, TEXT("Yaw: %f"), ServerRotationYaw);

	// 변경된 회전 값을 반영해 새로운 회전 값 생성 후 적용.
	FRotator NewRotator = RootComponent->GetComponentRotation();
	NewRotator.Yaw = ServerRotationYaw;

	RootComponent->SetWorldRotation(NewRotator);

	// 지난 업데이트로부터 걸린 시간 설정.
	ClientTimeBetweenLastUpdate = ClientTimeSinceUpdate;

	// 서버로부터 업데이트를 받은 다음에는 값 초기화.
	ClientTimeSinceUpdate = 0.0f;
}

void AABFountain::OnRep_ServerLightColor()
{
	AB_LOG(
		LogABNetwork,
		Log,
		TEXT("LightColor: %s"),
		*ServerLightColor.ToString()
	);

	// 컴포넌트 검색.
	UPointLightComponent* PointLight = Cast<UPointLightComponent>(
		GetComponentByClass(UPointLightComponent::StaticClass())
	);

	if (PointLight)
	{
		PointLight->SetLightColor(ServerLightColor);
	}

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
	else
	{
		// 변경된 회전 값을 반영해 새로운 회전 값 생성 후 적용.
		//FRotator NewRotator = RootComponent->GetComponentRotation();
		//NewRotator.Yaw = ServerRotationYaw;
		//
		//RootComponent->SetWorldRotation(NewRotator);

		// 서버로부터 데이터를 받은 이후에 지난 시간 업데이트.
		ClientTimeSinceUpdate += DeltaTime;

		// ClientTimeSinceUpdate 값이 작은지 확인.
		// 사실 0인지 비교하는 것과 같음. 
		// 0에 근접한 (보간에 의미가 없는) 시간 차이인지 확인.
		if (ClientTimeSinceUpdate < KINDA_SMALL_NUMBER)
		{
			return;
		}

		// 보간(Interpolation) 처리.
		// 다음 네트워크 패킷 전송 때 수신할 값을 예측.
		const float EstimateRotationYaw
			= ServerRotationYaw + RotationRate * ClientTimeBetweenLastUpdate;

		// 보간할 비율(t,a) 구하기.
		const float LerpRatio
			= ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;

		// 보간.
		const float ClientNewYaw
			= FMath::Lerp(ServerRotationYaw, EstimateRotationYaw, LerpRatio);

		// 회전 값 설정.
		FRotator ClientRotator = RootComponent->GetComponentRotation();
		ClientRotator.Yaw = ClientNewYaw;

		RootComponent->SetWorldRotation(ClientRotator);
	}
}

