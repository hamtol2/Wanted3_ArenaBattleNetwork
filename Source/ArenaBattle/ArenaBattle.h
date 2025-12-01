// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// NetMode.
#define LOG_NETMODEINFO ( (GetNetMode() == ENetMode::NM_Client) ? \
	*FString::Printf(TEXT("Client_%d"), UE::GetPlayInEditorID()) : \
	(GetNetMode() == ENetMode::NM_Standalone) ? \
	TEXT("StandAlone") : TEXT("Server") )


// __FUNCTION__ -> const char* (ANSI).
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)

#define AB_LOG(LogCat, Verbosity, Format, ...) \
UE_LOG(LogCat, Verbosity, TEXT("[%s] %s %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

DECLARE_LOG_CATEGORY_EXTERN(LogABNetwork, Log, All);