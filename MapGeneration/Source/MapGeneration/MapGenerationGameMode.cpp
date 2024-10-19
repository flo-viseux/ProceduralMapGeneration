// Copyright Epic Games, Inc. All Rights Reserved.

#include "MapGenerationGameMode.h"
#include "MapGenerationCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMapGenerationGameMode::AMapGenerationGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
