
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ABCharacterStat.generated.h"

USTRUCT(BlueprintType)
struct FABCharacterStat : public FTableRowBase
{
	GENERATED_BODY()

public:
	FABCharacterStat() : MaxHp(0.0f), Attack(0.0f), AttackRange(0.0f), AttackSpeed(0.0f) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float MaxHp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float MovementSpeed;

	FABCharacterStat operator+(const FABCharacterStat& Other) const
	{
		const float* const ThisPtr = reinterpret_cast<const float* const>(this);
		const float* const OtherPtr = reinterpret_cast<const float* const>(&Other);

		FABCharacterStat Result;
		float* ResultPtr = reinterpret_cast<float*>(&Result);
		int32 StatNum = sizeof(FABCharacterStat) / sizeof(float);
		for (int32 i = 0; i < StatNum; i++)
		{
			ResultPtr[i] = ThisPtr[i] + OtherPtr[i];
		}

		return Result;
	}

	bool NetSerialize(
		FArchive& Ar, 
		class UPackageMap* Map, 
		bool& bOutSuccess)
	{
		// 데이터 저장.
		/*
		float MaxHp;
		float Attack;
		float AttackRange;
		float AttackSpeed;
		float MovementSpeed;
		*/

		// 저장.
		uint32 uMaxHp = (uint32)MaxHp;
		Ar.SerializeIntPacked(uMaxHp);
		// 불러오기.
		MaxHp = (float)uMaxHp;

		// 저장.
		uint32 uAttack = (uint32)Attack;
		Ar.SerializeIntPacked(uAttack);
		// 불러오기.
		Attack = (float)uAttack;

		// 저장.
		uint32 uAttackRange = (uint32)AttackRange;
		Ar.SerializeIntPacked(uAttackRange);
		// 불러오기.
		AttackRange = (float)uAttackRange;

		// 저장.
		uint32 uAttackSpeed = (uint32)AttackSpeed;
		Ar.SerializeIntPacked(uAttackSpeed);
		// 불러오기.
		AttackSpeed = (float)uAttackSpeed;

		// 저장.
		uint32 uMovementSpeed = (uint32)MovementSpeed;
		Ar.SerializeIntPacked(uMovementSpeed);
		// 불러오기.
		MovementSpeed = (float)uMovementSpeed;

		return true;
	}
};

// NetSerialization 지원을 위한 템플릿 정의.
template<>
struct TStructOpsTypeTraits<FABCharacterStat>
	: public TStructOpsTypeTraitsBase2<FABCharacterStat>
{
	enum
	{
		WithNetSerializer = true,
	};
};
