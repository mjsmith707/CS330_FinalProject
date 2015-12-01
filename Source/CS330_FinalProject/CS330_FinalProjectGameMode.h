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

	SurfGameState getCurrentState() const;

	// For changing between states
	void transitionState(SurfGameState newstate);

private:
	// Current game state
	SurfGameState currentState;

	// Player Character
	APlayerCharacter* player;

	// Spawn Volumes
	TArray<ASurfTriggerVolume*> spawnVolumes;

	// Run Timer
	double runTimer;

	// Array of times
	TArray<double> bestTimes;

	// Per tick state update
	void updateState(float deltaTime);

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