// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CS330_FinalProject.h"
#include "CS330_FinalProjectGameMode.h"
#include "CS330_FinalProjectHUD.h"
#include "CS330_FinalProjectCharacter.h"

ACS330_FinalProjectGameMode::ACS330_FinalProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ACS330_FinalProjectHUD::StaticClass();
}
