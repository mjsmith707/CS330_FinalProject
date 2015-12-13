// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once

UENUM(EditAnywhere)
enum class SurfGameState : uint8 {
	MainMenu,
	LevelStart,
	InSpawn,
	Running,
	OutOfBounds,
	FinishedRunning,
	UNKNOWN
};

class APlayerCharacter;
class ASurfTriggerVolume;
class ACS330_FinalProjectHUD;
#include "GhostReplayCharacter.h"
#include "GameFramework/GameMode.h"
#include "CS330_FinalProjectGameMode.generated.h"

UCLASS(minimalapi)
class ACS330_FinalProjectGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ACS330_FinalProjectGameMode();

	virtual void Tick( float DeltaSeconds ) override;
    virtual void BeginPlay() override;

	// To get the current state
	SurfGameState getCurrentState() const;

	// For changing between states
	void transitionState(SurfGameState newstate);

	// Called by SurfTriggerVolume
	void setLastHitSpawnStage(unsigned int stageID);
	void setLastHitFinishedStage(unsigned int stageID);
private:
	// Current game state
	SurfGameState currentState;

	// Player Character
	APlayerCharacter* player;

	// Player's HUD
	ACS330_FinalProjectHUD* hud;
	const float stageInfoDrawMaxTime = 5.0f;
	float stageInfoDrawTime;
	bool drawStageInfo;
	double hudStageTime;
	double hudStageRecord;

	// Spawn Volumes
	TArray<ASurfTriggerVolume*> spawnVolumes;

	// Index of current stage
	unsigned int currentStage;

	// Stage of last hit spawn
	unsigned int lastHitSpawnStage;

	// Stage of last hit finished
	unsigned int lastHitFinishedStage;

	// Number of stages in level
	unsigned int numStages;

	// Total Run Timer
	double runTimer;

	// Array of stage timers
	TArray<double> stageTimer;

	// Best run time
	double bestTime;

	// Array of best stage times
	TArray<double> bestStageTimes;

	// Per tick state update
	void updateState(float deltaTime);

	// Hud update function
	void updateHUD(float deltaTime);

	// Tick functions for each state
	void handleMainMenu(float deltaTime);
	void handleLevelStart(float deltaTime);
	void handleInSpawn(float deltaTime);
	void handleRunning(float deltaTime);
	void handleOutOfBounds(float deltaTime);
	void handleFinishedRunning(float deltaTime);

	// For transitioning between states
	void handleTransitionMainMenu();
	void handleTransitionLevelStart();
	void handleTransitionInSpawn();
	void handleTransitionRunning();
	void handleTransitionOutOfBounds();
	void handleTransitionFinishedRunning();

	
};