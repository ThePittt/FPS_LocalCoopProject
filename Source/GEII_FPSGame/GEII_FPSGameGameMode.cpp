// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEII_FPSGameGameMode.h"
#include "GEII_FPSGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGEII_FPSGameGameMode::AGEII_FPSGameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
