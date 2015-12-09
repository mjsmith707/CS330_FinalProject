// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CS330_FinalProject.h"
#include "CS330_FinalProjectHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"

ACS330_FinalProjectHUD::ACS330_FinalProjectHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshiarTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshiarTexObj.Object;

	// Set some pretty text font
	ConstructorHelpers::FObjectFinder<UFont> fontObj(TEXT("/Game/OtherContent/verdanab"));
	hudText = fontObj.Object;

	// Set the box texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> hudBoxTexObj(TEXT("/Game/OtherContent/Hudbox"));
	hudBoxTex = hudBoxTexObj.Object;
}

// The joy's of hardcoded HUDs :)
void ACS330_FinalProjectHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair
	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X - (CrosshairTex->GetSurfaceWidth() * 0.5)),
										   (Center.Y - (CrosshairTex->GetSurfaceHeight() * 0.5f)) );

	// Get the base hud position (bottomish center)
	const FVector2D hudDrawPositionBase( (Center.X - (CrosshairTex->GetSurfaceWidth() * 0.5)),
										   (Canvas->ClipY*0.8f - (CrosshairTex->GetSurfaceHeight() * 0.5)) );

	const FVector2D hudBoxDrawPosition( (hudDrawPositionBase.X - (hudBoxTex->GetSurfaceWidth() * 0.5)),
										   (hudDrawPositionBase.Y - (hudBoxTex->GetSurfaceHeight() * 0.12f)) );
	
	// Current stage / Run Time
	const FVector2D stagePosition(hudDrawPositionBase.X-50.0f, hudDrawPositionBase.Y);
	const FVector2D stagePositionTime(hudDrawPositionBase.X+68.0f, hudDrawPositionBase.Y);

	// Record time
	const FVector2D recordPosition(hudDrawPositionBase.X-50.0f, hudDrawPositionBase.Y+20.0f);

	// Speedmeter
	const FVector2D speedPosition(hudDrawPositionBase.X-50.0f, hudDrawPositionBase.Y+40.0f);
	const FVector2D speedPositionSpeed(hudDrawPositionBase.X+10.0f, hudDrawPositionBase.Y+40.0f);

	// Stage Time and Stage Record
	const FVector2D StageInfoPosition(Canvas->ClipX * 0.46f, Canvas->ClipY * 0.2f);
	const FVector2D StageInfoTimePosition(StageInfoPosition.X-50.0f, StageInfoPosition.Y);
	const FVector2D StageInfoTimePositionTime(StageInfoPosition.X+80.0f, StageInfoPosition.Y);
	const FVector2D StageInfoBestPosition(StageInfoPosition.X-50.0f, StageInfoPosition.Y+20.0f);

	// Draw hud elements
	drawCrosshair(CrosshairDrawPosition);
	drawHudBox(hudBoxDrawPosition);
	drawStageAndRunTime(stagePosition, stagePositionTime);
	drawRecord(recordPosition);
	drawSpeed(speedPosition, speedPositionSpeed);
	if (drawStageInformation) {
		drawStageInfo(StageInfoTimePosition, StageInfoTimePositionTime, StageInfoBestPosition);
	}
}

// Draw function for the crosshair
void ACS330_FinalProjectHUD::drawCrosshair(const FVector2D& position) {
	// draw the crosshair
	FCanvasTileItem TileItem( position, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );
}

// Draws function for the box in the bottom center
void ACS330_FinalProjectHUD::drawHudBox(const FVector2D& position) {
	// draw the hudbox
	FCanvasTileItem TileItem( position, hudBoxTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );
}

// Draw function for stage and time
void ACS330_FinalProjectHUD::drawStageAndRunTime(const FVector2D& position, const FVector2D& position2) {
	FString stageStr(FString::Printf(TEXT("Stage: %d | Time: "), currentStage));
	FText stageText = FText::FromString(stageStr);
	FCanvasTextItem Stage(position, stageText, hudText, FLinearColor::White);
	Canvas->DrawItem(Stage);
	FString stageStr2(FString::Printf(TEXT("%f"), runTime));
	FText stageText2 = FText::FromString(stageStr2);
	if (runRecord == 0.0f) {
		FCanvasTextItem Stage2(position2, stageText2, hudText, FLinearColor::Green);
		Canvas->DrawItem(Stage2);
	}
	else if (runTime <= runRecord) {
		FCanvasTextItem Stage2(position2, stageText2, hudText, FLinearColor::Green);
		Canvas->DrawItem(Stage2);
	}
	else {
		FCanvasTextItem Stage2(position2, stageText2, hudText, FLinearColor::Red);
		Canvas->DrawItem(Stage2);
	}
}

// Draw the run record
void ACS330_FinalProjectHUD::drawRecord(const FVector2D& position) {
	FString recordStr(FString::Printf(TEXT("Record: %f"), runRecord));
	FText recordText = FText::FromString(recordStr);
	FCanvasTextItem Record(position, recordText, hudText, FLinearColor::White);
	Canvas->DrawItem(Record);
}

// Draw the current speed
void ACS330_FinalProjectHUD::drawSpeed(const FVector2D& position, const FVector2D& position2) {
	FString speedStr(FString::Printf(TEXT("Speed:")));
	FText speedText = FText::FromString(speedStr);
	FCanvasTextItem Speed(position, speedText, hudText, FLinearColor::White);
	Canvas->DrawItem(Speed);
	FString speedStr2(FString::Printf(TEXT("%f"), currentSpeed));
	FText speedText2 = FText::FromString(speedStr2);
	if (currentSpeed < 2000.0f) {
		FCanvasTextItem Speed2(position2, speedText2, hudText, FLinearColor::White);
		Canvas->DrawItem(Speed2);
	}
	else if (currentSpeed < 5000.0f) {
		FCanvasTextItem Speed2(position2, speedText2, hudText, FLinearColor::Yellow);
		Canvas->DrawItem(Speed2);
	}
	else {
		FCanvasTextItem Speed2(position2, speedText2, hudText, FLinearColor::Red);
		Canvas->DrawItem(Speed2);
	}
}

// Draws the completed stage time and completed stage record
void ACS330_FinalProjectHUD::drawStageInfo(const FVector2D& position, const FVector2D& position2, const FVector2D& position3) {
	FString stageStr(FString::Printf(TEXT("Stage Completed:")));
	FText stageText = FText::FromString(stageStr);
	FCanvasTextItem StageInfo(position, stageText, hudText, FLinearColor::White);
	Canvas->DrawItem(StageInfo);

	FString stageStr2(FString::Printf(TEXT("%f"), stageTimeComplete));
	FText stageText2 = FText::FromString(stageStr2);
	if (stageRecordComplete == 0.0f) {
		FCanvasTextItem StageInfo2(position2, stageText2, hudText, FLinearColor::Green);
		Canvas->DrawItem(StageInfo2);
	}
	else if (stageTimeComplete <= stageRecordComplete) {
		FCanvasTextItem StageInfo2(position2, stageText2, hudText, FLinearColor::Green);
		Canvas->DrawItem(StageInfo2);
	}
	else {
		FCanvasTextItem StageInfo2(position2, stageText2, hudText, FLinearColor::Red);
		Canvas->DrawItem(StageInfo2);
	}

	FString bestStr(FString::Printf(TEXT("Best Time: %f"), stageRecordComplete));
	FText bestText = FText::FromString(bestStr);
	FCanvasTextItem BestInfo(position3, bestText, hudText, FLinearColor::White);
	Canvas->DrawItem(BestInfo);
}

// Public setter for updating the hud
void ACS330_FinalProjectHUD::setHudVariables(unsigned int currentStage1, double runTime1, double runRecord1, double stageTime1, double stageRecord1, double stageTimeComplete1, double stageRecordComplete1, double currentSpeed1, bool drawStageInformation1) {
	this->currentStage = currentStage1;
	this->runTime = runTime1;
	this->stageTime = stageTime1;
	this->runRecord = runRecord1;
	this->stageTimeComplete = stageTimeComplete1;
	this->stageRecordComplete = stageRecordComplete1;
	this->stageRecord = stageRecord1;
	this->currentSpeed = currentSpeed1;
	this->drawStageInformation = drawStageInformation1;
}