// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "CS330_FinalProjectHUD.generated.h"

UCLASS()
class ACS330_FinalProjectHUD : public AHUD
{
	GENERATED_BODY()

public:
	ACS330_FinalProjectHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

