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

	void setHudVariables(unsigned int currentStage, double runTime, double runRecord, double stageTime, double stageRecord, double stageTimeComplete, double stageRecordComplete, double currentSpeed, bool drawStageInfo);

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

	// HUD Font Object
	class UFont* hudText;

	// HUD Box
	class UTexture2D* hudBoxTex;

	// Hud variables
	unsigned int currentStage;
	double runTime;
	double runRecord;
	double stageTime;
	double stageTimeComplete;
	double stageRecordComplete;
	double stageRecord;
	double currentSpeed;
	bool drawStageInformation;

	// Hud drawing routines
	void drawCrosshair(const FVector2D& position);
	void drawHudBox(const FVector2D& position);
	void drawStageAndRunTime(const FVector2D& position, const FVector2D& position2);
	void drawRecord(const FVector2D& position);
	void drawSpeed(const FVector2D& position, const FVector2D& position2);
	void drawStageInfo(const FVector2D& position, const FVector2D& position2, const FVector2D& position3);
};

